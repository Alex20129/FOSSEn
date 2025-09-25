#include <fcntl.h>
#include <unistd.h>
#include <QApplication>
#include <QWebElement>
#include <QFileInfo>
#include <QDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "phantom_wrapper.hpp"

PhantomWrapper::PhantomWrapper(QObject *parent) : QObject(parent)
{
	mDefaultSettings["javascriptEnabled"]=true;
	mDefaultSettings["loadImages"]=false;
	mDefaultSettings["userAgent"]="Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:135.0) Gecko/20100101 Firefox/135.0";
	mConfig=new Config(this);
	mCookieJar=new CookieJar(mConfig->cookiesFile(), this);
	mPage=new WebPage(this);
	mPage->setCookieJar(mCookieJar);
	connect(mPage, &WebPage::loadFinished, this, &PhantomWrapper::pageLoadingFinished);
}

void PhantomWrapper::loadCookiesFromFireFoxProfile(const QString &pathToFile) const
{
	QSettings settings(pathToFile, QSettings::IniFormat);
	QStringList profiles = settings.childGroups();
	QFileInfo iniFile(pathToFile);
	QDir profilesDir = iniFile.absoluteDir();
	QString profilePath;
	for (const QString &group : profiles)
	{
		if (group.startsWith("Profile"))
		{
			settings.beginGroup(group);
			if (settings.contains("Default") && settings.value("Default").toInt() == 1)
			{
				profilePath = settings.value("Path").toString();
				settings.endGroup();
				break;
			}
			if (profilePath.isEmpty())
			{
				profilePath = settings.value("Path").toString();
			}
			settings.endGroup();
		}
	}
	if (profilePath.isEmpty())
	{
		return;
	}
	QString cookiesFilePath = profilesDir.absoluteFilePath(profilePath + "/cookies.sqlite");
	if (!QFile::exists(cookiesFilePath))
	{
		return;
	}
	loadCookiesFromFile(cookiesFilePath);
}

void PhantomWrapper::loadCookiesFromFile(const QString &pathToFile) const
{
	QList<QNetworkCookie> cookies;
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "firefox_cookies");
	db.setDatabaseName(pathToFile);
	if (!db.open())
	{
		return;
	}
	QSqlQuery query(db);
	if (!query.exec("SELECT host, path, isSecure, expiry, name, value FROM moz_cookies"))
	{
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
		if (expiry != 0 && expiry < QDateTime::currentSecsSinceEpoch())
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

void PhantomWrapper::loadPage(const QUrl &url)
{
	mPage->openUrl(url, "get", mDefaultSettings);
}

QString PhantomWrapper::getPageContent() const
{
	return mPage->content();
}

#include <QTextDocument>
QString PhantomWrapper::getPageContentAsPlainText() const
{
	QTextDocument tDoc;
	tDoc.setHtml(mPage->content());
	QString plainText = tDoc.toPlainText().simplified();
	return plainText;
}

QString PhantomWrapper::getPageTitle() const
{
	return mPage->title();
}

QUrl PhantomWrapper::getPageURL() const
{
	return mPage->url();
}

QString PhantomWrapper::getPageURLEncoded() const
{
	return mPage->urlEncoded();
}

QList<QUrl> PhantomWrapper::extractPageLinks() const
{
	QList<QUrl> links;
	QWebFrame *pageMainFrame=mPage->mainFrame();
	if (pageMainFrame)
	{
		QUrl baseUrl=mPage->url();
		QWebElementCollection elements=pageMainFrame->findAllElements("a");
		for (const QWebElement &element : elements)
		{
			QString href=element.attribute("href");
			QUrl processedUrl;
			if (!href.isEmpty())
			{
				if (baseUrl.isValid())
				{
					processedUrl=baseUrl.resolved(QUrl(href));
				}
				else
				{
					processedUrl=QUrl(href);
				}
				if (processedUrl.isValid())
				{
					if (processedUrl.scheme() == QStringLiteral("http") || processedUrl.scheme() == QStringLiteral("https"))
					{
						links.append(processedUrl.adjusted(QUrl::RemoveFragment));
					}
				}
			}
		}
	}
	return links;
}
