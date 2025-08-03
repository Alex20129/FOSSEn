#ifndef CRAWLER_HPP
#define CRAWLER_HPP

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QRandomGenerator>
#include "phantom_wrapper.hpp"
#include "indexer.hpp"

#define PAGE_LOADING_INTERVAL_MIN 500
#define PAGE_LOADING_INTERVAL_MAX 5000

class Crawler : public QObject
{
	Q_OBJECT
	QRandomGenerator *mRNG;
	QThread *mCrawlerPrivateThread;
	QTimer *mLoadingIntervalTimer;
	PhantomWrapper *mPhantom;
	Indexer *mIndexer;
	QList<QString> *mURLListActive, *mURLListQueued;
	static QSet<QString> sVisitedURLList;
	static QSet<QString> sHostnameBlacklist;
	static QMutex sUnwantedLinksMutex;
	void swapURLLists();
private slots:
	void onThreadStarted();
	void onThreadFinished();
	void loadNextPage();
	void onPageHasBeenLoaded();
public:
	Crawler(QObject *parent=nullptr);
	~Crawler();
	const PhantomWrapper *getPhantom() const;
	const Indexer *getIndexer() const;
	void start();
	void stop();
	void addURLsToQueue(const QStringList &url_string_list);
	void addURLToQueue(const QString &url_string);
	void addHostnameToBlacklist(const QString &hostname);
public slots:
	void searchTest();
signals:
	void started(Crawler *crawler);
	void finished(Crawler *crawler);
	void needToIndexNewPage(PageMetadata page_metadata);
};

#endif // CRAWLER_HPP
