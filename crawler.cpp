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

QMap<QString, int> Crawler::extractWordsAndFrequency(const QString &text)
{
	qDebug("Crawler::extractWordsAndFrequency()");
	static const QRegularExpression wordsRegex("\\W+");
	static const QRegularExpression digitsRegex("^[0-9]+$");
	static const QSet<QString> stopWords =
	{
		"the", "and", "or", "a",
		"an", "in", "on", "at",
		"to", "for", "of", "with",
		"by", "was", "so", "such",
		"be", "as", "is"
	};
	QMap<QString, int> wordMap;
	QStringList tokens = text.toLower().split(wordsRegex, Qt::SkipEmptyParts);
	for (const QString &token : tokens)
	{
		if (token.length()>1 && token.length()<32)
		{
			if (!stopWords.contains(token) && !digitsRegex.match(token).hasMatch())
			{
				wordMap[token] += 1;
			}
		}
	}
	qDebug()<<wordMap.keys();
	return wordMap;
}

void Crawler::onNewThreadStarted()
{
	qDebug("Crawler::onNewThreadStarted()");
	emit started(this);
	mLoadingIntervalTimer->setInterval(mRNG->bounded(PAGE_LOADING_INTERVAL_MIN, PAGE_LOADING_INTERVAL_MAX));
	mLoadingIntervalTimer->start();
}

void Crawler::onNewThreadFinished()
{
	qDebug("Crawler::onNewThreadFinished()");
	emit finished(this);
	qDebug()<<"Visited Pages:\n"<<sVisitedPages.keys();
}

void Crawler::loadNextPage()
{
	qDebug("Crawler::loadNextPage()");
	QString nextURL;
	mURLQueueMutex.lock();
	if (!mURLList.isEmpty())
	{
		nextURL=mURLList.takeAt(mRNG->bounded(0, mURLList.count()));
	}
	mURLQueueMutex.unlock();
	qDebug() << mURLList.count() << "URLs in list";
	if(nextURL.length())
	{
		mPhantom->loadPage(nextURL);
	}
}

//for debug purpose
static int visited_n=0;

void Crawler::onPageHasBeenLoaded()
{
	qDebug("Crawler::onPageHasBeenLoaded()");
	QString pageURL = mPhantom->getPageURL();
	QString plainText = mPhantom->getPagePlainText();
	PageData data;

	qDebug() << "Page has been loaded:" << pageURL;

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

	//for debug purpose
	if(++visited_n>30)
	{
		stop();
		return;
	}
	else
	{
		mLoadingIntervalTimer->setInterval(mRNG->bounded(PAGE_LOADING_INTERVAL_MIN, PAGE_LOADING_INTERVAL_MAX));
		mLoadingIntervalTimer->start();
	}
}

void Crawler::start()
{
	qDebug("Crawler::start()");
	if(this->parent())
	{
		this->setParent(nullptr);
	}
	this->moveToThread(mCrawlerPersonalThread);
	mCrawlerPersonalThread->start();
}

void Crawler::stop()
{
	qDebug("Crawler::stop()");
	mLoadingIntervalTimer->stop();
	qDebug() << "unvisited pages:" << mURLList;
	mURLList.clear();
	mCrawlerPersonalThread->quit();
}

void Crawler::addURLToQueue(const QString &url_string)
{
	qDebug("Crawler::addURLToQueue()");
	QUrl newUrl(url_string);
	bool skipThisURL=0;
	sUnwantedLinksMutex.lock();
	if (sHostnameBlacklist.contains(newUrl.host()))
	{
		skipThisURL=1;
		qDebug() << "Skipping blacklisted host:" << newUrl.host();
	}
	else if (sVisitedPages.contains(url_string))
	{
		skipThisURL=1;
		qDebug() << "Skipping visited URL:" << url_string;
	}
	sUnwantedLinksMutex.unlock();
	if (!skipThisURL)
	{
		mURLQueueMutex.lock();
		if (mURLList.contains(url_string))
		{
			qDebug() << "Skipping duplicate URL:" << url_string;
		}
		else
		{
			mURLList.append(url_string);
			qDebug() << "URL has been added to the processing queue:" << url_string;
		}
		mURLQueueMutex.unlock();
	}
}

void Crawler::addHostnameToBlacklist(const QString &hostname)
{
	qDebug("Crawler::addHostToBlacklist()");
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
		qDebug() << "Host address has been added to the blacklist:" << hostname;
		if(shortName.length())
		{
			sHostnameBlacklist.insert(shortName);
			qDebug() << "Host address has been added to the blacklist:" << shortName;
		}
	}
	sUnwantedLinksMutex.unlock();
}
