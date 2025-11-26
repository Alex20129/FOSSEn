#ifndef CRAWLER_HPP
#define CRAWLER_HPP

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QRandomGenerator>
#include "web_page_processor.hpp"
#include "indexer.hpp"

#define PAGE_LOADING_INTERVAL_MIN 1024
#define PAGE_LOADING_INTERVAL_MAX 4096

class Crawler : public QObject
{
	Q_OBJECT
	QString mPathToFireFoxProfile;
	QRandomGenerator *mRNG;
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
	void loadNextPage();
	void onPageProcessingFinished();
public:
	Crawler(QObject *parent=nullptr);
	~Crawler();
	void setPathToFireFoxProfile(const QString &path_to_ff_profile);
	const Indexer *getIndexer() const;
	void addURLsToQueue(const QList<QUrl> &urls);
	void addURLToQueue(const QUrl &url);
	void addHostnameToBlacklist(const QString &hostname);
	void addCrawlingZone(const QUrl &zone_url);
public slots:
	void start();
	void stop();
	void searchTest();
signals:
	void started(Crawler *crawler);
	void finished(Crawler *crawler);
	void needToIndexNewPage(PageMetadata page_metadata);
};

#endif // CRAWLER_HPP
