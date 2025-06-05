#include <fcntl.h>
#include <unistd.h>
#include <QApplication>
#include <QWebElement>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "phantom_wrapper.hpp"
#include "simple_hash_func.hpp"

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
						links.append(newUrl.adjusted(QUrl::RemoveFragment).toString());
					}
				}
			}
		}
	}
	return links;
}

void PhantomWrapper::onPageLoadingFinished()
{
#ifndef NDEBUG
	QByteArray pageHtml=getPageHtml().toUtf8();
	uint64_t pageHash=mms_hash_64((uint8_t *)pageHtml.data(), pageHtml.size());

	QFile pageHTMLFile(QString("page_")+QString::number(pageHash, 16)+QString(".html"));
	if (pageHTMLFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		pageHTMLFile.write(pageHtml);
		pageHTMLFile.close();
	}
	else
	{
		qWarning() << "Failed to open page.html";
	}

	QFile pageTXTFile(QString("page_")+QString::number(pageHash, 16)+QString(".txt"));
	if (pageTXTFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		pageTXTFile.write(getPagePlainText().toUtf8());
		pageTXTFile.close();
	}
	else
	{
		qWarning() << "Failed to open page.txt";
	}

	QStringList PageLinksList = extractPageLinks();
	QFile pageLinksFile(QString("page_")+QString::number(pageHash, 16)+QString("_links.txt"));
	if (pageLinksFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		for(QString link : PageLinksList)
		{
			pageLinksFile.write(link.toUtf8());
			pageLinksFile.write("\n");
		}
		pageLinksFile.close();
	}
	else
	{
		qWarning() << "Failed to open page_links.txt";
	}
#endif

	emit pageHasBeenLoaded();
}
