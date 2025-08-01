#ifndef PHANTOM_WRAPPER_HPP
#define PHANTOM_WRAPPER_HPP

#include "config.h"
#include "webpage.h"

#include <QString>
#include <QObject>

class PhantomWrapper : public QObject
{
	Q_OBJECT
	WebPage *mPage;
	CookieJar *mCookieJar;
	Config *mConfig;
	QVariantMap mDefaultSettings;
signals:
	void pageHasBeenLoaded();
private slots:
	void onPageLoadingFinished();
public:
	PhantomWrapper(QObject *parent=nullptr);
	void loadCookiesFromFireFoxProfile(const QString &pathToFile) const;
	void loadCookiesFromFile(const QString &pathToFile) const;
	void loadPage(const QString &url);
	QString getPageHtml() const;
	QString getPagePlainText() const;
	QString getPageTitle() const;
	QUrl getPageURL() const;
	QString getPageURLEncoded() const;
	QStringList extractPageLinks() const;
};

#endif // PHANTOM_WRAPPER_HPP
