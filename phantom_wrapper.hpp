#ifndef PHANTOM_WRAPPER_HPP
#define PHANTOM_WRAPPER_HPP

#include "config.h"
#include "webpage.h"

#include <QString>
#include <QObject>

class PhantomWrapper : public QObject
{
	Q_OBJECT
	WebPage *m_page;
	CookieJar *m_cookie_jar;
	Config *m_config;
	QVariantMap m_default_settings;
signals:
	void webPageHasBeenLoaded();
private slots:
	void onWebPageLoadingFinished();
public:
	PhantomWrapper(QObject *parent=nullptr);
	void loadPage(const QString &url);
	QString getPageHtml() const;
	QString getPagePlainText() const;
	QStringList getPageLinks() const;
};

#endif // PHANTOM_WRAPPER_HPP
