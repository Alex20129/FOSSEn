#include <fcntl.h>
#include <unistd.h>
#include <QCoreApplication>
#include <QNetworkCookie>
#include <QWebEngineCookieStore>
#include <QFileInfo>
#include <QSettings>
#include <QDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <htmlcxx/html/ParserDom.h>
#include <htmlcxx/html/Uri.h>

#include "web_page_processor.hpp"

void WebPageProcessor::extractPageLinks(bool ok)
{
	if(!ok)
	{
		return;
	}

	QString pageContent=getPageContent();

	using namespace htmlcxx;

	HTML::ParserDom parser;
	tree<htmlcxx::HTML::Node> dom = parser.parseTree(pageContent.toStdString());

	for (tree<htmlcxx::HTML::Node>::iterator it = dom.begin(); it != dom.end(); ++it)
	{
		if (it->isTag())
		{
			if (it->tagName() == "a" || it->tagName() == "A")
			{
				it->parseAttributes();
				std::string href;

				std::pair<bool, std::string> href_pair = it->attribute("href");
				if (href_pair.first)  // true, если атрибут найден
				{
					href = href_pair.second;
				}

				if (!href.empty())
				{
					qDebug()<<href;

					QString finalUrl = getPageURLEncoded()+QString::fromStdString(href);
					QUrl qurl(finalUrl);

					if (qurl.isValid() &&
						(qurl.scheme() == QLatin1String("http") || qurl.scheme() == QLatin1String("https")))
					{
						qurl = qurl.adjusted(QUrl::RemoveFragment | QUrl::StripTrailingSlash);
						if (!mPageLinks.contains(qurl))
						{
							mPageLinks.append(qurl);
						}
					}
				}
			}
		}
	}

	qDebug() << "htmlcxx extracted" << mPageLinks.size() << "links from" << getPageURLEncoded();
	emit pageLoadingFinished();
}

WebPageProcessor::WebPageProcessor(QObject *parent) : QObject(parent)
{
	mWebPage=new QWebEnginePage(this);
	mProfile = new QWebEngineProfile(this);
	mProfile->setHttpCacheType(QWebEngineProfile::NoCache);
	mProfile->setHttpUserAgent("Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:135.0) Gecko/20100101 Firefox/135.0");
	mProfile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);
	connect(mWebPage, &QWebEnginePage::loadFinished, this, &WebPageProcessor::extractPageLinks);
}

void WebPageProcessor::loadCookiesFromFireFoxProfile(const QString &path_to_file) const
{
	if(path_to_file.isEmpty())
	{
		return;
	}
	QSettings settings(path_to_file, QSettings::IniFormat);
	QStringList profiles = settings.childGroups();
	QFileInfo iniFile(path_to_file);
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

void WebPageProcessor::loadCookiesFromFile(const QString &pathToFile) const
{
	QList<QNetworkCookie> cookies;
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "firefox_cookies");
	db.setDatabaseName(pathToFile);
	if (db.open())
	{
		QSqlQuery query(db);
		if (query.exec("SELECT host, path, isSecure, expiry, name, value FROM moz_cookies"))
		{
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
		}
		db.close();
	}
	QSqlDatabase::removeDatabase("firefox_cookies");
	for (const QNetworkCookie &cookie : cookies)
	{
		mProfile->cookieStore()->setCookie(cookie);
	}
}

void WebPageProcessor::loadPage(const QUrl &url)
{
	mWebPage->load(url);
}

QString WebPageProcessor::getPageContent() const
{
	QString content;
	mWebPage->toHtml([&content](const QString &html) { content = html; });
	return content;
}

QString WebPageProcessor::getPageContentAsPlainText() const
{
	QString plainText;
	mWebPage->toPlainText([&plainText](const QString &text) { plainText = text; });
	return plainText;
}

// #include <QTextDocument>
// QString WebPageProcessor::getPageContentAsPlainText() const
// {
// 	QTextDocument tDoc;
// 	tDoc.setHtml(mWebPage->content());
// 	QString plainText = tDoc.toPlainText().simplified();
// 	return plainText;
// }

QString WebPageProcessor::getPageTitle() const
{
	return mWebPage->title();
}

QUrl WebPageProcessor::getPageURL() const
{
	return mWebPage->url();
}

QByteArray WebPageProcessor::getPageURLEncoded() const
{
	return mWebPage->url().toEncoded();
}

QList<QUrl> WebPageProcessor::getPageLinks() const
{
	return mPageLinks;
}
