#include <iostream>
#include <string>

#include <QApplication>

#include "main.hpp"
#include "crawler.hpp"

using namespace std;

int main(int argc, char** argv)
{
	QApplication fossenApp(argc, argv);
	Crawler *myCrawler=new Crawler;

	QObject::connect(myCrawler, &Crawler::finished, &fossenApp, &QApplication::quit);

	myCrawler->addURLToQueue(QString("https://stackoverflow.com/search?q=malloc"));
	myCrawler->addURLToQueue(QString("https://www.google.com/search?udm=2&q=fast+sorting+algorithm"));
	myCrawler->addURLToQueue(QString("https://search.yahoo.com/search?p=quantum+computing"));
	myCrawler->addURLToQueue(QString("https://www.lostfilm.tv/movies/"));

	myCrawler->addURLToBlacklist(QString("www.linkedin.com"));
	myCrawler->addURLToBlacklist(QString("linkedin.com"));
	myCrawler->addURLToBlacklist(QString("www.instagram.com"));
	myCrawler->addURLToBlacklist(QString("www.google.com"));
	myCrawler->addURLToBlacklist(QString("www.yandex.ru"));
	myCrawler->addURLToBlacklist(QString("www.ya.ru"));
	myCrawler->addURLToBlacklist(QString("www.yahoo.com"));
	myCrawler->addURLToBlacklist(QString("www.dzen.ru"));
	myCrawler->addURLToBlacklist(QString("dzen.ru"));
	myCrawler->addURLToBlacklist(QString("www.facebook.com"));

	QTimer::singleShot(0, myCrawler, &Crawler::start);

	return(fossenApp.exec());
}
