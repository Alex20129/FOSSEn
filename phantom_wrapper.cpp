#include "phantom_wrapper.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <QApplication>
#include <QTimer>

PhantomWrapper::PhantomWrapper(QObject *parent) : QObject(parent)
{
	m_default_settings["javascriptEnabled"] = true;
	m_default_settings["loadImages"] = false;
	m_default_settings["userAgent"] = "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:135.0) Gecko/20100101 Firefox/135.0";

	m_config=new Config(nullptr);
	m_cookie_jar = new CookieJar(m_config->cookiesFile());
	m_page=new WebPage(nullptr);
	m_page->setCookieJar(m_cookie_jar);
	QObject::connect(m_page, &WebPage::loadFinished, this, &PhantomWrapper::onWebPageLoadingFinished);
}

PhantomWrapper::~PhantomWrapper()
{
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

void PhantomWrapper::onWebPageLoadingFinished()
{
	int page_html=open("page.html", O_WRONLY | O_CREAT, 0664);
	write(page_html, getPageHtml().toStdString().data(), getPageHtml().toStdString().length());
	close(page_html);

	int page_txt=open("page.txt", O_WRONLY | O_CREAT, 0664);
	write(page_txt, getPagePlainText().toStdString().data(), getPagePlainText().toStdString().length());
	close(page_txt);

	emit(webPageHasBeenLoaded());
}
