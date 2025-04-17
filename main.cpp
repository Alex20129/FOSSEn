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

	myCrawler->addURLToQueue(QString("https://stackoverflow.com/search?q=malloc"));
	myCrawler->addURLToQueue(QString("https://www.google.com/search?q=fast+sorting+algorithm"));
	myCrawler->addURLToQueue(QString("https://search.yahoo.com/search?p=fast+sorting+algorithm"));
	myCrawler->addURLToQueue(QString("https://www.lostfilm.tv/movies/"));
	myCrawler->addURLToQueue(QString("https://habr.com/en/articles/"));

	myCrawler->addURLToBlacklist(QString("www.linkedin.com"));
	myCrawler->addURLToBlacklist(QString("linkedin.com"));
	myCrawler->addURLToBlacklist(QString("www.instagram.com"));
	myCrawler->addURLToBlacklist(QString("instagram.com"));
	myCrawler->addURLToBlacklist(QString("www.google.com"));
	myCrawler->addURLToBlacklist(QString("www.yandex.ru"));
	myCrawler->addURLToBlacklist(QString("www.ya.ru"));
	myCrawler->addURLToBlacklist(QString("www.yahoo.com"));
	myCrawler->addURLToBlacklist(QString("www.dzen.ru"));
	myCrawler->addURLToBlacklist(QString("dzen.ru"));
	myCrawler->addURLToBlacklist(QString("www.facebook.com"));
	myCrawler->addURLToBlacklist(QString("facebook.com"));
	myCrawler->addURLToBlacklist(QString("vk.com"));
	myCrawler->addURLToBlacklist(QString("vk.ru"));

	QTimer::singleShot(0, myCrawler, &Crawler::start);

	return(fossenApp.exec());
}
