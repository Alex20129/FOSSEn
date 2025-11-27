#ifndef WEB_PAGE_PROCESSOR_HPP
#define WEB_PAGE_PROCESSOR_HPP

#include <QWebEngineSettings>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QString>
#include <QObject>

class WebPageProcessor : public QObject
{
	Q_OBJECT
	QWebEnginePage *mWebPage;
	QWebEngineProfile *mProfile;
	QString mPageContent;
	QList<QUrl> mPageLinks;
private slots:
	void extractPageContent(bool ok);
	void extractPageLinks();
public:
	WebPageProcessor(QObject *parent=nullptr);
	void loadCookiesFromFireFoxProfile(const QString &path_to_file);
	void loadCookiesFromFireFoxDB(const QString &path_to_file);
	void loadPage(const QUrl &url);
	QString getPageContent() const;
	QString getPageContentAsPlainText() const;
	QString getPageTitle() const;
	QUrl getPageURL() const;
	QByteArray getPageURLEncoded() const;
	QList<QUrl> getPageLinks() const;
signals:
	void pageLoadingFinished();
	void pageProcessingFinished();
};

#endif // WEB_PAGE_PROCESSOR_HPP
