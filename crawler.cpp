#include <QFile>
#include "util.hpp"
#include "crawler.hpp"
#include "simple_hash_func.hpp"

QSet<QString> Crawler::sVisitedPages;
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
	mIndexer->initialize("in_test.bin");
	mLoadingIntervalTimer->setSingleShot(1);
	connect(mCrawlerPersonalThread, &QThread::started, this, &Crawler::onNewThreadStarted);
	connect(mCrawlerPersonalThread, &QThread::finished, this, &Crawler::onNewThreadFinished);
	connect(mPhantom, &PhantomWrapper::pageHasBeenLoaded, this, &Crawler::onPageHasBeenLoaded);
	connect(mLoadingIntervalTimer, &QTimer::timeout, this, &Crawler::loadNextPage);
	connect(this, &Crawler::needToIndexNewPage, mIndexer, &Indexer::addPage);
}

const PhantomWrapper *Crawler::getPhantom() const
{
	return mPhantom;
}

const Indexer *Crawler::getIndexer() const
{
	return mIndexer;
}

void Crawler::onNewThreadStarted()
{
	qDebug("Crawler::onNewThreadStarted()");
	mLoadingIntervalTimer->setInterval(mRNG->bounded(PAGE_LOADING_INTERVAL_MIN, PAGE_LOADING_INTERVAL_MAX));
	mLoadingIntervalTimer->start();
	emit started(this);
}

void Crawler::onNewThreadFinished()
{
	qDebug("Crawler::onNewThreadFinished()");
	qDebug()<<"Visited Pages:\n"<<sVisitedPages.values();
	emit finished(this);
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
	qDebug() << mURLList.count() << "URLs pending on the list";
	mURLQueueMutex.unlock();
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
	qDebug("Crawler::onPageHasBeenLoaded()");

	QString pageURL = mPhantom->getPageURLEncoded();
	QString pagePlainText = mPhantom->getPagePlainText();
	QByteArray pageHtml=mPhantom->getPageHtml().toUtf8();
	QStringList pageLinksList = mPhantom->extractPageLinks();
	PageMetadata pageMetadata;

	pageMetadata.contentHash=xorshift_hash_64((uint8_t *)pageHtml.data(), pageHtml.size());
	pageMetadata.timeStamp = QDateTime::currentDateTime();
	pageMetadata.title = mPhantom->getPageTitle();
	pageMetadata.url = mPhantom->getPageURL();
	pageMetadata.words = ExtractWordsAndFrequencies(pagePlainText);

	emit needToIndexNewPage(pageMetadata);

#ifndef NDEBUG
	qDebug() << pageURL;
	qDebug()<<pageMetadata.words.keys();

	QFile pageHTMLFile(QString("page_")+QString::number(pageMetadata.contentHash&0xFFFFFF, 16)+QString(".html"));
	if (pageHTMLFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		pageHTMLFile.write(pageHtml);
		pageHTMLFile.close();
	}
	else
	{
		qWarning() << "Failed to open page.html";
	}

	QFile pageTXTFile(QString("page_")+QString::number(pageMetadata.contentHash&0xFFFFFF, 16)+QString(".txt"));
	if (pageTXTFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		pageTXTFile.write(pagePlainText.toUtf8());
		pageTXTFile.close();
	}
	else
	{
		qWarning() << "Failed to open page.txt";
	}

	QFile pageLinksFile(QString("page_")+QString::number(pageMetadata.contentHash&0xFFFFFF, 16)+QString("_links.txt"));
	if (pageLinksFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		for(QString link : pageLinksList)
		{
			pageLinksFile.write(link.toUtf8()+QByteArray("\n"));
		}
		pageLinksFile.close();
	}
	else
	{
		qWarning() << "Failed to open page_links.txt";
	}
#endif

	sUnwantedLinksMutex.lock();
	sVisitedPages.insert(pageURL);
	sUnwantedLinksMutex.unlock();

	for (const QString &link : pageLinksList)
	{
		addURLToQueue(link);
	}

#ifndef NDEBUG
	if(++visited_n>40)
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
	mURLQueueMutex.lock();
#ifndef NDEBUG
	qDebug() << "unvisited pages:" << mURLList;
#endif
	mURLList.clear();
	mURLQueueMutex.unlock();
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
