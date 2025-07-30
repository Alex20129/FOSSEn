#ifndef CRAWLER_HPP
#define CRAWLER_HPP

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QRandomGenerator>
#include "phantom_wrapper.hpp"
#include "indexer.hpp"

#define PAGE_LOADING_INTERVAL_MIN 960
#define PAGE_LOADING_INTERVAL_MAX 3840

class Crawler : public QObject
{
	Q_OBJECT
	QRandomGenerator *mRNG;
	QThread *mCrawlerPersonalThread;
	QTimer *mLoadingIntervalTimer;
	PhantomWrapper *mPhantom;
	Indexer *mIndexer;
	QList<QString> mURLList;
	QMutex mURLQueueMutex;
	static QSet<QString> sVisitedPages;
	static QSet<QString> sHostnameBlacklist;
	static QMutex sUnwantedLinksMutex;
signals:
	void started(Crawler *crawler);
	void finished(Crawler *crawler);
	void needToIndexNewPage(PageMetadata page_metadata);
private slots:
	void onNewThreadStarted();
	void onNewThreadFinished();
	void loadNextPage();
	void onPageHasBeenLoaded();
public:
	Crawler(QObject *parent=nullptr);
	const PhantomWrapper *getPhantom() const;
	const Indexer *getIndexer() const;
	void start();
	void stop();
	void addURLToQueue(const QString &url_string);
	void addHostnameToBlacklist(const QString &hostname);
};

#endif // CRAWLER_HPP
