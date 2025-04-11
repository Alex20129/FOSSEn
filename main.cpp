#include <iostream>
#include <string>

#include <QApplication>

#include "main.hpp"
#include "phantom_wrapper.hpp"

using namespace std;

int main(int argc, char** argv)
{
	QApplication fossenApp(argc, argv);
	PhantomWrapper phWrapper;

	QObject::connect(&phWrapper, &PhantomWrapper::webPageHasBeenLoaded, &fossenApp, &QApplication::quit);

	QString url("https://www.google.com/search?udm=2&q=fast+sorting+algorithm");
	// https://www.lostfilm.tv/movies/
	// https://www.google.com/search?udm=2&q=fast+sorting+algorithm
	// https://search.yahoo.com/search?p=qt+web+kit

	phWrapper.loadPage(url);

	return(fossenApp.exec());
}
