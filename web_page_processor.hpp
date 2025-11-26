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
	QList<QUrl> mPageLinks;
private slots:
	void extractPageLinks(bool ok);
public:
	WebPageProcessor(QObject *parent=nullptr);
	void loadCookiesFromFireFoxProfile(const QString &path_to_file);
	void loadCookiesFromFile(const QString &pathToFile);
	void loadPage(const QUrl &url);
	QString getPageContent() const;
	QString getPageContentAsPlainText() const;
	QString getPageTitle() const;
	QUrl getPageURL() const;
	QByteArray getPageURLEncoded() const;
	QList<QUrl> getPageLinks() const;
signals:
	void pageLoadingFinished();
};

#endif // WEB_PAGE_PROCESSOR_HPP
