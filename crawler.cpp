#include <QRegularExpression>
#include "crawler.hpp"

QHash<QString, PageData> Crawler::sVisitedPages;
QSet<QString> Crawler::sHostnameBlacklist;
QMutex Crawler::sUnwantedLinksMutex;

Crawler::Crawler(QObject *parent) : QObject(parent)
{
	uint32_t rngSeed=QDateTime::currentSecsSinceEpoch()+reinterpret_cast<uintptr_t>(this);
	mRNG=new QRandomGenerator(rngSeed);
	mCrawlerPersonalThread=new QThread(this);
	mLoadingIntervalTimer=new QTimer(this);
	mPhantom=new PhantomWrapper(this);
	mIndexer=new Indexer(this);
	mIndexer->initialize("in_test.sqlite");
	mLoadingIntervalTimer->setSingleShot(1);
	connect(mCrawlerPersonalThread, &QThread::started, this, &Crawler::onNewThreadStarted);
	connect(mCrawlerPersonalThread, &QThread::finished, this, &Crawler::onNewThreadFinished);
	connect(mPhantom, &PhantomWrapper::pageHasBeenLoaded, this, &Crawler::onPageHasBeenLoaded);
	connect(mLoadingIntervalTimer, &QTimer::timeout, this, &Crawler::loadNextPage);
}

const PhantomWrapper *Crawler::getPhantom() const
{
	return mPhantom;
}

const Indexer *Crawler::getIndexer() const
{
	return mIndexer;
}

QMap<QString, int> Crawler::extractWordsAndFrequency(const QString &text)
{
#ifndef NDEBUG
	qDebug("Crawler::extractWordsAndFrequency()");
#endif
	static const QRegularExpression wordsRegex("\\W+");
	static const QRegularExpression digitsRegex("^[0-9]+$");
	static const QSet<QString> stopWords =
	{
		"the", "and", "for", "with", "was", "such"
	};
	QMap<QString, int> wordMap;
	QStringList words = text.toLower().split(wordsRegex, Qt::SkipEmptyParts);
	for (const QString &word : words)
	{
		if (word.length()>2 && word.length()<32)
		{
			if (!stopWords.contains(word) && !digitsRegex.match(word).hasMatch())
			{
				wordMap[word] += 1;
			}
		}
	}
#ifndef NDEBUG
	qDebug()<<wordMap.keys();
#endif
	return wordMap;
}

void Crawler::onNewThreadStarted()
{
#ifndef NDEBUG
	qDebug("Crawler::onNewThreadStarted()");
#endif
	emit started(this);
	mLoadingIntervalTimer->setInterval(mRNG->bounded(PAGE_LOADING_INTERVAL_MIN, PAGE_LOADING_INTERVAL_MAX));
	mLoadingIntervalTimer->start();
}

void Crawler::onNewThreadFinished()
{
#ifndef NDEBUG
	qDebug("Crawler::onNewThreadFinished()");
	qDebug()<<"Visited Pages:\n"<<sVisitedPages.keys();
#endif
	emit finished(this);
}

void Crawler::loadNextPage()
{
#ifndef NDEBUG
	qDebug("Crawler::loadNextPage()");
#endif
	QString nextURL;
	mURLQueueMutex.lock();
	if (!mURLList.isEmpty())
	{
		nextURL=mURLList.takeAt(mRNG->bounded(0, mURLList.count()));
	}
	mURLQueueMutex.unlock();
#ifndef NDEBUG
	qDebug() << mURLList.count() << "Pending URLs in list";
#endif
	if(nextURL.length())
	{
		mPhantom->loadPage(nextURL);
	}
}

#ifndef NDEBUG
static int visited_n=0;
#endif

void Crawler::onPageHasBeenLoaded()
{
#ifndef NDEBUG
	qDebug("Crawler::onPageHasBeenLoaded()");
#endif

	QString pageURL = mPhantom->getPageURL();
	QString plainText = mPhantom->getPagePlainText();
	PageData data;

#ifndef NDEBUG
	qDebug() << "Page has been loaded:" << pageURL;
#endif

	data.timestamp = QDateTime::currentDateTime();
	data.title = mPhantom->getPageTitle();
	data.wordsAndFrequency = extractWordsAndFrequency(plainText);

	sUnwantedLinksMutex.lock();
	sVisitedPages.insert(pageURL, data);
	sUnwantedLinksMutex.unlock();

	QStringList links = mPhantom->extractPageLinks();
	for (const QString &link : links)
	{
		addURLToQueue(link);
	}

#ifndef NDEBUG
	if(++visited_n>30)
	{
		stop();
		return;
	}
	else
#endif
	{
		mLoadingIntervalTimer->setInterval(mRNG->bounded(PAGE_LOADING_INTERVAL_MIN, PAGE_LOADING_INTERVAL_MAX));
		mLoadingIntervalTimer->start();
	}
}

void Crawler::start()
{
#ifndef NDEBUG
	qDebug("Crawler::start()");
#endif
	if(this->parent())
	{
		this->setParent(nullptr);
	}
	this->moveToThread(mCrawlerPersonalThread);
	mCrawlerPersonalThread->start();
}

void Crawler::stop()
{
#ifndef NDEBUG
	qDebug("Crawler::stop()");
#endif
	mLoadingIntervalTimer->stop();
#ifndef NDEBUG
	qDebug() << "unvisited pages:" << mURLList;
#endif
	mURLList.clear();
	mCrawlerPersonalThread->quit();
}

void Crawler::addURLToQueue(const QString &url_string)
{
#ifndef NDEBUG
	qDebug("Crawler::addURLToQueue()");
#endif
	QUrl newUrl(url_string);
	bool skipThisURL=0;
	sUnwantedLinksMutex.lock();
	if (sHostnameBlacklist.contains(newUrl.host()))
	{
		skipThisURL=1;
#ifndef NDEBUG
		qDebug() << "Skipping blacklisted host:" << newUrl.host();
#endif
	}
	else if (sVisitedPages.contains(url_string))
	{
		skipThisURL=1;
#ifndef NDEBUG
		qDebug() << "Skipping visited URL:" << url_string;
#endif
	}
	sUnwantedLinksMutex.unlock();
	if (!skipThisURL)
	{
		mURLQueueMutex.lock();
		if (mURLList.contains(url_string))
		{
#ifndef NDEBUG
			qDebug() << "Skipping duplicate URL:" << url_string;
#endif
		}
		else
		{
			mURLList.append(url_string);
#ifndef NDEBUG
			qDebug() << "URL has been added to the processing queue:" << url_string;
#endif
		}
		mURLQueueMutex.unlock();
	}
}

void Crawler::addHostnameToBlacklist(const QString &hostname)
{
#ifndef NDEBUG
	qDebug("Crawler::addHostToBlacklist()");
#endif
	QString shortName;
	if(hostname.startsWith("www."))
	{
		shortName=hostname;
		shortName.remove(0, 4);
	}
	sUnwantedLinksMutex.lock();
	if (!sHostnameBlacklist.contains(hostname))
	{
		sHostnameBlacklist.insert(hostname);
#ifndef NDEBUG
		qDebug() << "Host address has been added to the blacklist:" << hostname;
#endif
		if(shortName.length())
		{
			sHostnameBlacklist.insert(shortName);
#ifndef NDEBUG
			qDebug() << "Host address has been added to the blacklist:" << shortName;
#endif
		}
	}
	sUnwantedLinksMutex.unlock();
}
