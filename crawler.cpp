#include <QFile>
#include "util.hpp"
#include "crawler.hpp"
#include "simple_hash_func.hpp"

QSet<QString> Crawler::sVisitedURLList;
QSet<QString> Crawler::sHostnameBlacklist;

QMutex Crawler::sUnwantedLinksMutex;

Crawler::Crawler(QObject *parent) : QObject(parent)
{
	uint32_t rngSeed=QDateTime::currentSecsSinceEpoch()+reinterpret_cast<uintptr_t>(this);
	mURLListActive=new QList<QString>;
	mURLListQueued=new QList<QString>;
	mRNG=new QRandomGenerator(rngSeed);
	mCrawlerPrivateThread=new QThread(this);
	mLoadingIntervalTimer=new QTimer(this);
	mPhantom=new PhantomWrapper(this);
	mIndexer=new Indexer(this);
	mIndexer->initialize("in_test.bin");
	mLoadingIntervalTimer->setSingleShot(1);
	connect(mCrawlerPrivateThread, &QThread::started, this, &Crawler::onThreadStarted);
	connect(mCrawlerPrivateThread, &QThread::finished, this, &Crawler::onThreadFinished);
	connect(mPhantom, &PhantomWrapper::pageHasBeenLoaded, this, &Crawler::onPageHasBeenLoaded);
	connect(mLoadingIntervalTimer, &QTimer::timeout, this, &Crawler::loadNextPage);
	connect(this, &Crawler::needToIndexNewPage, mIndexer, &Indexer::addPage);
}

Crawler::~Crawler()
{
	stop();
	delete mRNG;
	delete mURLListQueued;
	delete mURLListActive;
}

void Crawler::swapURLLists()
{
	qSwap(mURLListActive, mURLListQueued);
}

const PhantomWrapper *Crawler::getPhantom() const
{
	return mPhantom;
}

const Indexer *Crawler::getIndexer() const
{
	return mIndexer;
}

void Crawler::onThreadStarted()
{
	qDebug("Crawler::onThreadStarted");
	mLoadingIntervalTimer->setInterval(mRNG->bounded(PAGE_LOADING_INTERVAL_MIN, PAGE_LOADING_INTERVAL_MAX));
	mLoadingIntervalTimer->start();
	emit started(this);
}

void Crawler::onThreadFinished()
{
	qDebug("Crawler::onThreadFinished");
	sUnwantedLinksMutex.lock();
	qDebug()<<"Visited Pages:\n"<<sVisitedURLList.values();
	sUnwantedLinksMutex.unlock();
	emit finished(this);
}

void Crawler::loadNextPage()
{
	qDebug("Crawler::loadNextPage");
	QString nextURL;
	if (mURLListActive->isEmpty())
	{
		swapURLLists();
		if (mURLListActive->isEmpty())
		{
			return;
		}
	}
	nextURL = mURLListActive->takeAt(mRNG->bounded(0, mURLListActive->count()));
	sUnwantedLinksMutex.lock();
	sVisitedURLList.insert(nextURL);
	sUnwantedLinksMutex.unlock();
	qDebug() << mURLListActive->count()+mURLListQueued->count() << "URLs pending on the list";
	mPhantom->loadPage(nextURL);
}

#ifndef NDEBUG
static int visited_n=0;
#endif

void Crawler::onPageHasBeenLoaded()
{
	qDebug("Crawler::onPageHasBeenLoaded");

	QString pageURL = mPhantom->getPageURLEncoded();
	QString pagePlainText = mPhantom->getPagePlainText();
	QByteArray pageHtml=mPhantom->getPageHtml().toUtf8();
	QStringList pageLinksList = mPhantom->extractPageLinks();
	PageMetadata pageMetadata;

	qDebug() << pageURL;

	pageMetadata.contentHash=xorshift_hash_64((uint8_t *)pageHtml.data(), pageHtml.size());
	pageMetadata.timeStamp = QDateTime::currentDateTime();
	pageMetadata.title = mPhantom->getPageTitle();
	pageMetadata.url = mPhantom->getPageURL();
	pageMetadata.words = ExtractWordsAndFrequencies(pagePlainText);

	emit needToIndexNewPage(pageMetadata);

	addURLsToQueue(pageLinksList);

#ifndef NDEBUG
	QFile pageHTMLFile(QString("page_")+QString::number(pageMetadata.contentHash&UINT32_MAX, 16).toUpper() + QString(".html"));
	if (pageHTMLFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		pageHTMLFile.write(pageHtml);
		pageHTMLFile.close();
	}
	else
	{
		qWarning() << "Failed to open page.html";
	}

	QFile pageTXTFile(QString("page_")+QString::number(pageMetadata.contentHash&UINT32_MAX, 16).toUpper() + QString(".txt"));
	if (pageTXTFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		pageTXTFile.write(pagePlainText.toUtf8());
		pageTXTFile.close();
	}
	else
	{
		qWarning() << "Failed to open page.txt";
	}

	QFile pageLinksFile(QString("page_")+QString::number(pageMetadata.contentHash&UINT32_MAX, 16).toUpper() + QString("_links.txt"));
	if (pageLinksFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		for(const QString &link : pageLinksList)
		{
			pageLinksFile.write(link.toUtf8()+QByteArray("\n"));
		}
		pageLinksFile.close();
	}
	else
	{
		qWarning() << "Failed to open page_links.txt";
	}

	QFile pageWordsFile(QString("page_")+QString::number(pageMetadata.contentHash&UINT32_MAX, 16).toUpper() + QString("_words.txt"));
	if (pageWordsFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		for(const QString word : pageMetadata.words.keys())
		{
			pageWordsFile.write(word.toUtf8()+" "+QString::number(pageMetadata.words[word]).toUtf8()+QByteArray("\n"));
		}
		pageWordsFile.close();
	}
	else
	{
		qWarning() << "Failed to open page_words.txt";
	}
#endif

#ifndef NDEBUG
	if(++visited_n>=4)
	{
		stop();
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
	qDebug("Crawler::start");
	if(this->parent())
	{
		this->setParent(nullptr);
	}
	this->moveToThread(mCrawlerPrivateThread);
	mCrawlerPrivateThread->start();
}

void Crawler::stop()
{
	qDebug("Crawler::stop");
	mLoadingIntervalTimer->stop();
	qDebug() << "unvisited pages:" << *mURLListActive << *mURLListQueued;
	mURLListActive->clear();
	mURLListQueued->clear();
	mCrawlerPrivateThread->quit();
}

void Crawler::addURLsToQueue(const QStringList &url_string_list)
{
	qDebug("Crawler::addURLsToQueue");
	for (const QString &link : url_string_list)
	{
		addURLToQueue(link);
	}
}

void Crawler::addURLToQueue(const QString &url_string)
{
	qDebug("Crawler::addURLToQueue");
	QUrl newUrl(url_string);
	bool skipThisURL=0;
	sUnwantedLinksMutex.lock();
	if (sHostnameBlacklist.contains(newUrl.host()))
	{
		skipThisURL=1;
		qDebug() << "Skipping blacklisted host:" << newUrl.host();
	}
	else if (sVisitedURLList.contains(url_string))
	{
		skipThisURL=1;
		qDebug() << "Skipping visited URL:" << url_string;
	}
	sUnwantedLinksMutex.unlock();
	if (!skipThisURL)
	{
		if (mURLListQueued->contains(url_string))
		{
			qDebug() << "Skipping duplicate URL:\n" << url_string;
		}
		else
		{
			qDebug() << "Adding URL to the processing list:\n" << url_string;
			mURLListQueued->append(url_string);
		}
	}
}

void Crawler::addHostnameToBlacklist(const QString &hostname)
{
	qDebug("Crawler::addHostnameToBlacklist");
	sUnwantedLinksMutex.lock();
	if (!sHostnameBlacklist.contains(hostname))
	{
		sHostnameBlacklist.insert(hostname);
		qDebug() << "Host name has been added to the blacklist:" << hostname;
	}
	sUnwantedLinksMutex.unlock();
}

void Crawler::searchTest()
{
	qDebug("Crawler::searchTest");
	QStringList words;
	words.append("algorithm");
	QList<PageMetadata> searchResults=mIndexer->searchWords(words);
	for(const PageMetadata page : searchResults)
	{
		qDebug() << "=================";
		qDebug() << page.contentHash;
		qDebug() << page.title;
		qDebug() << page.timeStamp;
		qDebug() << page.url;
	}
}
