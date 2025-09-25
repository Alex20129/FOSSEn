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
private slots:
public:
	PhantomWrapper(QObject *parent=nullptr);
	void loadCookiesFromFireFoxProfile(const QString &pathToFile) const;
	void loadCookiesFromFile(const QString &pathToFile) const;
	void loadPage(const QUrl &url);
	QString getPageContent() const;
	QString getPageContentAsPlainText() const;
	QString getPageTitle() const;
	QUrl getPageURL() const;
	QString getPageURLEncoded() const;
	QList<QUrl> extractPageLinks() const;
signals:
	void pageLoadingFinished();
};

#endif // PHANTOM_WRAPPER_HPP
