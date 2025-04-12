#ifndef CRAWLER_HPP
#define CRAWLER_HPP

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QQueue>
#include "phantom_wrapper.hpp"

struct PageData
{
	QDateTime timestamp;
	QString title;
	QMap<QString, int> wordsAndFrequency;
};

class Crawler : public QObject
{
	Q_OBJECT
	QThread *mCrawlerPersonalThread;
	QTimer *mLoadingIntervalTimer;
	PhantomWrapper *mPhantom;
	QQueue<QString> mURLQueue;
	QMutex mURLQueueMutex;
	static QHash<QString, PageData> sVisitedPages;
	static QMutex sVisitedPagesMutex;
	static QSet<QString> sBlacklist;
	static QMutex sBlacklistMutex;
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
	void start();
	void stop();
	void addURLToQueue(const QString &url_string);
	void addURLToBlacklist(const QString &url_string);
};

#endif // CRAWLER_HPP
