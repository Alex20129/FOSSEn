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

	myCrawler->addHostnameToBlacklist(QString("www.linkedin.com"));
	myCrawler->addHostnameToBlacklist(QString("linkedin.com"));
	myCrawler->addHostnameToBlacklist(QString("www.instagram.com"));
	myCrawler->addHostnameToBlacklist(QString("instagram.com"));
	myCrawler->addHostnameToBlacklist(QString("www.google.com"));
	myCrawler->addHostnameToBlacklist(QString("www.yandex.ru"));
	myCrawler->addHostnameToBlacklist(QString("www.ya.ru"));
	myCrawler->addHostnameToBlacklist(QString("www.yahoo.com"));
	myCrawler->addHostnameToBlacklist(QString("www.dzen.ru"));
	myCrawler->addHostnameToBlacklist(QString("dzen.ru"));
	myCrawler->addHostnameToBlacklist(QString("www.facebook.com"));
	myCrawler->addHostnameToBlacklist(QString("facebook.com"));
	myCrawler->addHostnameToBlacklist(QString("vk.com"));
	myCrawler->addHostnameToBlacklist(QString("vk.ru"));

	return 555;

	QTimer::singleShot(0, myCrawler, &Crawler::start);

	return(fossenApp.exec());
}
