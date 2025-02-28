#ifndef PHANTOM_WRAPPER_HPP
#define PHANTOM_WRAPPER_HPP

#include "config.h"
#include "webpage.h"

#include <QString>
#include <QObject>
#include <memory>

class PhantomWrapper : public QObject
{
	Q_OBJECT
	WebPage *m_page;
	CookieJar *m_cookie_jar;
	Config *m_config;
	QVariantMap m_default_settings;
private slots:
	void onWebPageLoadingFinished();
public:
	PhantomWrapper(QObject *parent=nullptr);
	~PhantomWrapper();

	void loadPage(const QString &url);

	// Получение отрендеренного HTML
	QString getPageHtml() const;

	// Получение текстового содержимого страницы
	QString getPagePlainText() const;

signals:
	void webPageHasBeenLoaded();
};

#endif // PHANTOM_WRAPPER_HPP
