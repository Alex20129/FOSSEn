#include <QRegularExpression>
#include "crawler.hpp"

QHash<QString, PageData> Crawler::sVisitedPages;
QMutex Crawler::sVisitedPagesMutex;
QSet<QString> Crawler::sBlacklist;
QMutex Crawler::sBlacklistMutex;

Crawler::Crawler(QObject *parent) : QObject(parent)
{
	mCrawlerPersonalThread=new QThread(this);
	mPhantom=new PhantomWrapper(this);
	connect(mCrawlerPersonalThread, &QThread::started, this, &Crawler::onNewThreadStarted);
	connect(mCrawlerPersonalThread, &QThread::finished, this, &Crawler::onNewThreadFinished);
	connect(mPhantom, &PhantomWrapper::pageHasBeenLoaded, this, &Crawler::onPageHasBeenLoaded);
}

QMap<QString, int> Crawler::extractWordsAndFrequency(const QString& text)
{
	QMap<QString, int> wordMap;
	QStringList tokens = text.toLower().split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);
	for (const QString &token : tokens)
	{
		if (token.length())
		{
			qDebug()<<token;
			wordMap[token] += 1;
		}
	}
	return wordMap;
}

void Crawler::onNewThreadStarted()
{
	qDebug("Crawler::onNewThreadStarted()");
	if (!mURLQueue.isEmpty())
	{
		mPhantom->loadPage(mURLQueue.dequeue());
	}
}

void Crawler::onNewThreadFinished()
{
	qDebug("Crawler::onNewThreadFinished()");
	qDebug()<<"Visited Pages:\n"<<sVisitedPages.keys();
}

void Crawler::onPageHasBeenLoaded()
{
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
	if(sVisitedPages.size()>255)
	{
		stop();
		return;
	}

	if (!mURLQueue.isEmpty())
	{
		mPhantom->loadPage(mURLQueue.dequeue());
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
	emit started(this);
}

void Crawler::stop()
{
	qDebug("Crawler::stop()");
	mURLQueue.clear();
	mCrawlerPersonalThread->quit();
	mCrawlerPersonalThread->wait();
	emit finished(this);
}

void Crawler::addURLToQueue(const QString &url_string)
{
	qDebug("Crawler::addURLToQueue()");
	bool badURL=0;
	QUrl newUrl(url_string);
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
		if (!sVisitedPages.contains(url_string))
		{
			qDebug() << "Adding new URL into processing queue:" << url_string;
			mURLQueue.enqueue(url_string);
		}
		sVisitedPagesMutex.unlock();
	}
}
