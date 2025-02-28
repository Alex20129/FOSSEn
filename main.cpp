#include <iostream>
#include <string>

#include <QApplication>

#include <htmlcxx/html/ParserDom.h>
#include <htmlcxx/html/Node.h>

#include <curl/curl.h>

#include "main.hpp"
#include "phantom_wrapper.hpp"

using namespace std;
using namespace htmlcxx;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp)
{
	size_t totalSize = size * nmemb;
	userp->append((char*)contents, totalSize);
	return totalSize;
}

std::string fetchContent(const std::string &url)
{
	std::string readBuffer;
	CURL* curl;
	CURLcode res;
	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		if (res != CURLE_OK)
		{
			// std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
			// error handling here
		}
	}
	return readBuffer;
}

int main(int argc, char** argv)
{
	// std::string pg=fetchContent("https://stackoverflow.com");
	// std::cout << pg << "\n==================\n";

	QApplication fossenApp(argc, argv);
	PhantomWrapper phWrapper;

	QObject::connect(&phWrapper, &PhantomWrapper::webPageHasBeenLoaded, &fossenApp, &QApplication::quit);

	QString url("https://stackoverflow.com");

	phWrapper.loadPage(url);

	return(fossenApp.exec());
}
