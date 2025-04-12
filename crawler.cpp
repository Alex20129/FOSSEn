#include <QRegularExpression>
#include "crawler.hpp"

QHash<QString, PageData> Crawler::sVisitedPages;
QMutex Crawler::sVisitedPagesMutex;
QSet<QString> Crawler::sBlacklist;
QMutex Crawler::sBlacklistMutex;

Crawler::Crawler(QObject *parent) : QObject(parent)
{
	mCrawlerPersonalThread=new QThread(this);
	mLoadingIntervalTimer=new QTimer(this);
	mPhantom=new PhantomWrapper(this);
	connect(mCrawlerPersonalThread, &QThread::started, this, &Crawler::onNewThreadStarted);
	connect(mCrawlerPersonalThread, &QThread::finished, this, &Crawler::onNewThreadFinished);
	connect(mPhantom, &PhantomWrapper::pageHasBeenLoaded, this, &Crawler::onPageHasBeenLoaded);
	connect(mLoadingIntervalTimer, &QTimer::timeout, this, &Crawler::loadNextPage);
	mLoadingIntervalTimer->setInterval(1000);
	mLoadingIntervalTimer->setSingleShot(1);
}

QMap<QString, int> Crawler::extractWordsAndFrequency(const QString &text)
{
	qDebug("Crawler::extractWordsAndFrequency()");
	QMap<QString, int> wordMap;
	QStringList tokens = text.toLower().split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);
	for (const QString &token : tokens)
	{
		if (token.length()>0 && token.length()<32)
		{
			wordMap[token] += 1;
			qDebug()<<token;
		}
	}
	return wordMap;
}

void Crawler::onNewThreadStarted()
{
	qDebug("Crawler::onNewThreadStarted()");
	emit started(this);
	loadNextPage();
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
	if (!mURLQueue.isEmpty())
	{
		qDebug() << mURLQueue.size() << "URLs in queue";
		mPhantom->loadPage(mURLQueue.dequeue());
	}
}

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

	sVisitedPagesMutex.lock();
	sVisitedPages.insert(pageURL, data);
	sVisitedPagesMutex.unlock();

	QStringList links = mPhantom->extractPageLinks();
	for (const QString &link : links)
	{
		addURLToQueue(link);
	}

	//for debug purpose
	if(++visited_n>12)
	{
		stop();
		return;
	}
	else
	{
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
	mURLQueue.clear();
	mCrawlerPersonalThread->quit();
	mCrawlerPersonalThread->wait();
}

void Crawler::addURLToQueue(const QString &url_string)
{
	qDebug("Crawler::addURLToQueue()");
	QUrl newUrl(url_string);
	bool badURL=0;
	qDebug() << "host" << newUrl.host();
	sBlacklistMutex.lock();
	if (sBlacklist.contains(newUrl.host()))
	{
		qDebug() << "Skipping blacklisted host:" << newUrl.host();
		badURL=1;
	}
	sBlacklistMutex.unlock();
	if (!badURL)
	{
		sVisitedPagesMutex.lock();
		if (sVisitedPages.contains(url_string))
		{
			qDebug() << "Skipping visited URL:" << url_string;
		}
		else if (mURLQueue.contains(url_string))
		{
			qDebug() << "Skipping enqueued URL:" << url_string;
		}
		else
		{
			mURLQueue.enqueue(url_string);
			qDebug() << "New URL has been added into processing queue:" << url_string;
		}
		sVisitedPagesMutex.unlock();
	}
}

void Crawler::addURLToBlacklist(const QString &url_string)
{
	qDebug("Crawler::addURLToQueue()");
	sBlacklistMutex.lock();
	sBlacklist.insert(url_string);
	sBlacklistMutex.unlock();
}
