#ifndef CRAWLER_HPP
#define CRAWLER_HPP

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QRandomGenerator>
#include "web_page_processor.hpp"
#include "indexer.hpp"

#define PAGE_LOADING_INTERVAL_MIN 500
#define PAGE_LOADING_INTERVAL_MAX 5000

class Crawler : public QObject
{
	Q_OBJECT
	QRandomGenerator *mRNG;
	QThread *mCrawlerPrivateThread;
	QTimer *mLoadingIntervalTimer;
	WebPageProcessor *mWebPageProcessor;
	Indexer *mIndexer;
	QList<QUrl> *mURLListActive, *mURLListQueued;
	QHash<QString, QString> mCrawlingZones;
	static QSet<QString> sVisitedURLList;
	static QSet<QString> sHostnameBlacklist;
	static QMutex sUnwantedLinksMutex;
	void swapURLLists();
private slots:
	void onThreadStarted();
	void onThreadFinished();
	void loadNextPage();
	void onPageLoadingFinished();
public:
	Crawler(QObject *parent=nullptr);
	~Crawler();
	const WebPageProcessor *getPhantom() const;
	const Indexer *getIndexer() const;
	void start();
	void stop();
	void addURLsToQueue(const QList<QUrl> &urls);
	void addURLToQueue(const QUrl &url);
	void addHostnameToBlacklist(const QString &hostname);
	void addCrawlingZone(const QUrl &zone_url);
public slots:
	void searchTest();
signals:
	void started(Crawler *crawler);
	void finished(Crawler *crawler);
	void needToIndexNewPage(PageMetadata page_metadata);
};

#endif // CRAWLER_HPP
