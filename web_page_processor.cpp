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

void WebPageProcessor::extractPageContent(bool ok)
{
	if(!ok)
	{
		return;
	}
	mWebPage->toHtml([this](const QString &html)
			{
				this->mPageContent = html;
				emit pageLoadingFinished();
			});
}

void WebPageProcessor::extractPageLinks()
{
	using namespace htmlcxx;
	HTML::ParserDom parser;
	parser.parseTree(mPageContent.toStdString());
	const tree<HTML::Node> &domTree=parser.getTree();
	QUrl baseUrl=getPageURL();
	mPageLinks.clear();
	for (HTML::Node &domNode : domTree)
	{
		if (domNode.isTag())
		{
			if ((domNode.tagName() != "a") && (domNode.tagName() != "A"))
			{
				continue;
			}
			else
			{
				domNode.parseAttributes();
				std::string hrefString;
				std::pair<bool, std::string> href_pair = domNode.attribute("href");
				if (href_pair.first)
				{
					hrefString = href_pair.second;
					QUrl processedUrl;
					if(!hrefString.empty())
					{
						if (baseUrl.isValid())
						{
							processedUrl=baseUrl.resolved(QUrl(QString::fromStdString(hrefString)));
						}
						else
						{
							processedUrl=QUrl(QString::fromStdString(hrefString));
						}
						processedUrl=processedUrl.adjusted(QUrl::RemoveFragment);
						if (processedUrl.isValid())
						{
							if (processedUrl.scheme() == QLatin1StringView("http") || processedUrl.scheme() == QLatin1StringView("https"))
							{
								mPageLinks.append(processedUrl);
							}
						}
					}
				}
			}
		}
	}
	emit pageProcessingFinished();
}

WebPageProcessor::WebPageProcessor(QObject *parent) : QObject(parent)
{
	mWebPage=new QWebEnginePage(this);
	mProfile = new QWebEngineProfile(this);
	mProfile->setHttpCacheType(QWebEngineProfile::NoCache);
	mProfile->setHttpUserAgent("Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:135.0) Gecko/20100101 Firefox/135.0");
	mProfile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);
	connect(mWebPage, &QWebEnginePage::loadFinished, this, &WebPageProcessor::extractPageContent);
	connect(this, &WebPageProcessor::pageLoadingFinished, this, &WebPageProcessor::extractPageLinks);
}

void WebPageProcessor::loadCookiesFromFireFoxProfile(const QString &path_to_file)
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

void WebPageProcessor::loadCookiesFromFile(const QString &pathToFile)
{
	QList<QNetworkCookie> cookies;
	{
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
	return mPageContent;
}

// QString WebPageProcessor::getPageContentAsPlainText() const
// {
// 	QString plainText;
// 	mWebPage->toPlainText([&plainText](const QString &text) { plainText = text; });
// 	return plainText;
// }

#include <QTextDocument>
QString WebPageProcessor::getPageContentAsPlainText() const
{
	QTextDocument tDoc;
	tDoc.setHtml(mPageContent);
	QString plainText = tDoc.toPlainText().simplified();
	return plainText;
}

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
