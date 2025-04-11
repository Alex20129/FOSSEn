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

	QString startingURL1("https://codeforum.org");
	QString startingURL2("https://stackoverflow.com/search?q=malloc");
	QString startingURL3("https://www.google.com/search?udm=2&q=fast+sorting+algorithm");
	QString startingURL4("https://search.yahoo.com/search?p=qt+web+kit");
	QString startingURL5("https://www.lostfilm.tv/movies/");

	myCrawler->addURLToQueue(startingURL1);
	myCrawler->addURLToQueue(startingURL2);
	myCrawler->addURLToQueue(startingURL3);
	myCrawler->addURLToQueue(startingURL4);
	myCrawler->addURLToQueue(startingURL5);

	QTimer::singleShot(0, myCrawler, &Crawler::start);

	return(fossenApp.exec());
}
