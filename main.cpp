#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <iostream>
#include <string>

#include "main.hpp"
#include "crawler.hpp"

using namespace std;

int main(int argc, char** argv)
{
	QApplication fossenApp(argc, argv);
	Crawler *myCrawler=new Crawler;

	const PhantomWrapper *phantom = myCrawler->getPhantom();
	if (phantom)
	{
		phantom->loadCookiesFromFireFoxProfile("/home/alex/snap/firefox/common/.mozilla/firefox/profiles.ini");
	}

	QFile crawlerConfigFile("crawler.json");
	if (!crawlerConfigFile.exists())
	{
		qDebug() << "crawler.json doesn't exist";
		return 22;
	}

	if(!crawlerConfigFile.open(QIODevice::ReadOnly))
	{
		qDebug() << "Couldn't open crawler.json";
		return 33;
	}

	QByteArray crawlerConfigData=crawlerConfigFile.readAll();
	crawlerConfigFile.close();

	QJsonParseError err;
	QJsonDocument crawlerConfigJsonDoc = QJsonDocument::fromJson(crawlerConfigData, &err);

	if (err.error != QJsonParseError::NoError)
	{
		qDebug() << "Couldn't parse crawler.json :" << err.errorString();
		return 44;
	}

	if(!crawlerConfigJsonDoc.isObject())
	{
		qDebug() << "Bad JSON in crawler.json file";
		return 55;
	}

	QJsonObject rawlerConfigJsonObject = crawlerConfigJsonDoc.object();

	for (const QJsonValue &startURLEntry : rawlerConfigJsonObject.value("start_urls").toArray())
	{
		if(startURLEntry.isObject())
		{
			const QJsonObject urlObject=startURLEntry.toObject();
			myCrawler->addURLToQueue(urlObject.value("url").toString());
			myCrawler->addCrawlingZone(urlObject.value("zone_prefix").toString());
		}
	}
	for (const QJsonValue &host : rawlerConfigJsonObject.value("black_list").toArray())
	{
		myCrawler->addHostnameToBlacklist(host.toString());
	}

	// ======
	// const Indexer *indexer = myCrawler->getIndexer();
	QObject::connect(myCrawler, &Crawler::finished, &fossenApp, &QApplication::quit);
	// QObject::connect(myCrawler, &Crawler::finished, myCrawler, &Crawler::searchTest);
	// ======

	QTimer::singleShot(0, myCrawler, &Crawler::start);

	return(fossenApp.exec());
}
