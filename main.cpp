#include "main.hpp"

#include <iostream>
#include <string>
#include <sstream>

#include <htmlcxx/html/ParserDom.h>
#include <htmlcxx/html/Node.h>

#include <curl/curl.h>

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

std::string extractJScriptsFromHtml(const std::string& html)
{
	stringstream scripts;
	HTML::ParserDom parser;
	tree<HTML::Node> dom = parser.parseTree(html);
	bool needToExtractJSText=0;

	for (HTML::Node html_node : dom)
	{
		if (html_node.isTag())
		{
			if (html_node.tagName() == "script")
			{
				html_node.parseAttributes();
				if (html_node.attribute("src").second.length())
				{
					// cout << html_node.attribute("src").second;
					// cout << "\n++++++++++++++++++\n";
					std::string jsContent=fetchContent(html_node.attribute("src").second);
					if (jsContent.length())
					{
						scripts << jsContent << "\n";
						// scripts << "\n++++++++++++++++++\n";
					}
				}
				else
				{
					needToExtractJSText=1;
				}
			}
		}
		else if (!html_node.isComment() && needToExtractJSText)
		{
			needToExtractJSText=0;
			if (html_node.text().length())
			{
				scripts << html_node.text() << "\n";
				// scripts << "\n++++++++++++++++++\n";
			}
		}
	}
	return scripts.str();
}

int main()
{
	std::string pg=fetchContent("https://www.thingiverse.com");
	// std::cout << pg << "\n==================\n";

	std::string js=extractJScriptsFromHtml(pg);
	std::cout << js;

	return(0);
}
