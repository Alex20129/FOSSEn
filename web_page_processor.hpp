#ifndef WEB_PAGE_PROCESSOR_HPP
#define WEB_PAGE_PROCESSOR_HPP

#include <QVariantMap>
#include <QString>
#include <QObject>

class WebPageProcessor : public QObject
{
	Q_OBJECT
	QVariantMap mDefaultSettings;
private slots:
public:
	WebPageProcessor(QObject *parent=nullptr);
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

#endif // WEB_PAGE_PROCESSOR_HPP
