#ifndef CRAWLER_HPP
#define CRAWLER_HPP

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QRandomGenerator>
#include "phantom_wrapper.hpp"

#define PAGE_LOADING_INTERVAL_MIN 960
#define PAGE_LOADING_INTERVAL_MAX 3840

struct PageData
{
	QDateTime timestamp;
	QString title;
	QMap<QString, int> wordsAndFrequency;
};

class Crawler : public QObject
{
	Q_OBJECT
	QRandomGenerator *mRNG;
	QThread *mCrawlerPersonalThread;
	QTimer *mLoadingIntervalTimer;
	PhantomWrapper *mPhantom;
	QList<QString> mURLList;
	QMutex mURLQueueMutex;
	static QHash<QString, PageData> sVisitedPages;
	static QSet<QString> sBlacklist;
	static QMutex sUnwantedLinksMutex;
	QMap<QString, int> extractWordsAndFrequency(const QString &text);
signals:
	void started(Crawler *crawler);
	void finished(Crawler *crawler);
private slots:
	void onNewThreadStarted();
	void onNewThreadFinished();
	void loadNextPage();
	void onPageHasBeenLoaded();
public:
	Crawler(QObject *parent=nullptr);
	const PhantomWrapper *getPhantom() const;
	void start();
	void stop();
	void addURLToQueue(const QString &url_string);
	void addURLToBlacklist(const QString &url_string);
};

#endif // CRAWLER_HPP
