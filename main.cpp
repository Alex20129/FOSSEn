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

	QObject::connect(myCrawler, &Crawler::finished, &fossenApp, &QApplication::quit);

	QFile startConfigFile("start.json");
	if (!startConfigFile.exists())
	{
		qDebug() << "start.json doesn't exist";
		return 22;
	}

	if(!startConfigFile.open(QIODevice::ReadOnly))
	{
		qDebug() << "Couldn't open start.json";
		return 33;
	}

	QByteArray startConfigFileData=startConfigFile.readAll();
	startConfigFile.close();

	QJsonParseError err;
	QJsonDocument startConfigFileJsonDoc = QJsonDocument::fromJson(startConfigFileData, &err);

	if (err.error != QJsonParseError::NoError)
	{
		qDebug() << "Couldn't start.json :" << err.errorString();
		return 44;
	}

	if(!startConfigFileJsonDoc.isObject())
	{
		qDebug() << "Bad JSON in start.json file";
		return 55;
	}

	QJsonObject startConfigFileJsonObject = startConfigFileJsonDoc.object();

	for (const QJsonValue &url : startConfigFileJsonObject.value("start_urls").toArray())
	{
		myCrawler->addURLToQueue(url.toString());
	}
	for (const QJsonValue &host : startConfigFileJsonObject.value("black_list").toArray())
	{
		myCrawler->addHostnameToBlacklist(host.toString());
	}

	QTimer::singleShot(0, myCrawler, &Crawler::start);

	return(fossenApp.exec());
}
