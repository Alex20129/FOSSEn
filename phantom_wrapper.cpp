#include "phantom_wrapper.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <QApplication>
#include <QWebElement>

PhantomWrapper::PhantomWrapper(QObject *parent) : QObject(parent)
{
	m_default_settings["javascriptEnabled"]=true;
	m_default_settings["loadImages"]=false;
	m_default_settings["userAgent"]="Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:135.0) Gecko/20100101 Firefox/135.0";
	m_config=new Config(this);
	m_cookie_jar=new CookieJar(m_config->cookiesFile(), this);
	m_page=new WebPage(this);
	m_page->setCookieJar(m_cookie_jar);
	QObject::connect(m_page, &WebPage::loadFinished, this, &PhantomWrapper::onWebPageLoadingFinished);
}

void PhantomWrapper::loadPage(const QString &url)
{
	m_page->openUrl(url, "get", m_default_settings);
}

QString PhantomWrapper::getPageHtml() const
{
	return m_page->content();
}

QString PhantomWrapper::getPagePlainText() const
{
	return m_page->plainText();
}

QStringList PhantomWrapper::getPageLinks() const
{
	QStringList links;
	QWebFrame *mainFrame=m_page->mainFrame();
	if (mainFrame)
	{
		QUrl baseUrl=m_page->url();
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
					if (newUrl.scheme()==QStringLiteral("http") || newUrl.scheme()==QStringLiteral("https"))
					{
						links.append(newUrl.toString());
					}
				}
			}
		}
	}
	return links;
}

void PhantomWrapper::onWebPageLoadingFinished()
{
	int page_html=open("page.html", O_WRONLY | O_CREAT, 0664);
	write(page_html, getPageHtml().toStdString().data(), getPageHtml().toStdString().length());
	close(page_html);

	int page_txt=open("page.txt", O_WRONLY | O_CREAT, 0664);
	write(page_txt, getPagePlainText().toStdString().data(), getPagePlainText().toStdString().length());
	close(page_txt);

	qDebug()<<"PageLinks:\n"<<getPageLinks();

	emit(webPageHasBeenLoaded());
}
