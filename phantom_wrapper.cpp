#include "phantom_wrapper.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <QApplication>
#include <QWebElement>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

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

void PhantomWrapper::loadCookiesFromFile(const QString &pathToFile)
{
	QList<QNetworkCookie> cookies;
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "firefox_cookies");
	db.setDatabaseName(pathToFile);
	if (!db.open())
	{
		qCritical() << "Failed to open cookies database:" << db.lastError().text();
		return;
	}
	QSqlQuery query(db);
	if (!query.exec("SELECT host, path, isSecure, expiry, name, value FROM moz_cookies"))
	{
		qCritical() << "Failed to query cookies:" << query.lastError().text();
		db.close();
		return;
	}
	while (query.next())
	{
		QString host = query.value("host").toString();
		QString path = query.value("path").toString();
		bool isSecure = query.value("isSecure").toBool();
		qint64 expiry = query.value("expiry").toLongLong();
		QString name = query.value("name").toString();
		QString value = query.value("value").toString();
		if (expiry != 0 && expiry < QDateTime::currentDateTime().toSecsSinceEpoch())
		{
			continue;
		}
		QNetworkCookie cookie(name.toUtf8(), value.toUtf8());
		cookie.setDomain(host);
		cookie.setPath(path);
		cookie.setSecure(isSecure);
		if (expiry != 0)
		{
			cookie.setExpirationDate(QDateTime::fromSecsSinceEpoch(expiry));
		}
		cookies.append(cookie);
	}
	db.close();
	QSqlDatabase::removeDatabase("firefox_cookies");
	for (const QNetworkCookie &cookie : cookies)
	{
		mCookieJar->insertCookie(cookie);
	}
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
		QByteArray PageHtmlUTF8=getPageHtml().toUtf8();
		write(pageHtmlFile, PageHtmlUTF8.data(), PageHtmlUTF8.length());
		close(pageHtmlFile);
	}

	//for debug purpose
	int pageTextFile = open("page.txt", O_WRONLY | O_CREAT, 0664);
	if(pageTextFile>=0)
	{
		QByteArray PagePlainTextUTF8=getPagePlainText().toUtf8();
		write(pageTextFile, PagePlainTextUTF8.data(), PagePlainTextUTF8.length());
		close(pageTextFile);
	}

	//for debug purpose
	QStringList PageLinksList = extractPageLinks();
	int pageLinksFile = open("page_links.txt", O_WRONLY | O_CREAT, 0664);
	if(pageLinksFile>=0)
	{
		QByteArray linkUTF8;
		for(QString link : PageLinksList)
		{
			linkUTF8=link.toUtf8();
			write(pageLinksFile, linkUTF8.data(), linkUTF8.length());
			write(pageLinksFile, "\n", 1);
		}
		close(pageLinksFile);
	}

	emit(pageHasBeenLoaded());
}
