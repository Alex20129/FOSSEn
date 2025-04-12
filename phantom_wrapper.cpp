#include "phantom_wrapper.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <QApplication>
#include <QWebElement>

PhantomWrapper::PhantomWrapper(QObject *parent) : QObject(parent)
{
	mDefaultSettings["javascriptEnabled"]=true;
	mDefaultSettings["loadImages"]=false;
	mDefaultSettings["userAgent"]="Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:135.0) Gecko/20100101 Firefox/135.0";
	mConfig=new Config(this);
	mCookieJar=new CookieJar(mConfig->cookiesFile(), this);
	mPage=new WebPage(this);
	mPage->setCookieJar(mCookieJar);
	connect(mPage, &WebPage::loadFinished, this, &PhantomWrapper::onPageLoadingFinished);
}

void PhantomWrapper::loadPage(const QString &url)
{
	mPage->openUrl(url, "get", mDefaultSettings);
}

QString PhantomWrapper::getPageHtml() const
{
	return mPage->content();
}

QString PhantomWrapper::getPagePlainText() const
{
	return mPage->plainText();
}

QString PhantomWrapper::getPageTitle() const
{
	return mPage->title();
}

QString PhantomWrapper::getPageURL() const
{
	return mPage->url();
}

QStringList PhantomWrapper::extractPageLinks() const
{
	QStringList links;
	QWebFrame *mainFrame=mPage->mainFrame();
	if (mainFrame)
	{
		QUrl baseUrl=mPage->url();
		QWebElementCollection elements=mainFrame->findAllElements("a");
		for (const QWebElement &element : elements)
		{
			QString href=element.attribute("href");
			QUrl newUrl;
			if (!href.isEmpty())
			{
				if (baseUrl.isValid())
				{
					newUrl=baseUrl.resolved(QUrl(href));
				}
				else
				{
					newUrl=QUrl(href);
				}
				if (newUrl.isValid())
				{
					if (newUrl.scheme() == QStringLiteral("http") || newUrl.scheme() == QStringLiteral("https"))
					{
						links.append(newUrl.toString());
					}
				}
			}
		}
	}
	return links;
}

void PhantomWrapper::onPageLoadingFinished()
{
	//for debug purpose
	int pageHtmlFile = open("page.html", O_WRONLY | O_CREAT, 0664);
	if(pageHtmlFile>=0)
	{
		write(pageHtmlFile, getPageHtml().toStdString().data(), getPageHtml().toStdString().length());
		close(pageHtmlFile);
	}

	//for debug purpose
	int pageTextFile = open("page.txt", O_WRONLY | O_CREAT, 0664);
	if(pageTextFile>=0)
	{
		write(pageTextFile, getPagePlainText().toStdString().data(), getPagePlainText().toStdString().length());
		close(pageTextFile);
	}

	//for debug purpose
	QStringList PageLinksList = extractPageLinks();
	int pageLinksFile = open("page_links.txt", O_WRONLY | O_CREAT, 0664);
	if(pageLinksFile>=0)
	{
		for(QString link : PageLinksList)
		{
			write(pageLinksFile, link.toStdString().data(), link.toStdString().length());
			write(pageLinksFile, "\n", 1);
		}
		close(pageLinksFile);
	}

	emit(pageHasBeenLoaded());
}
