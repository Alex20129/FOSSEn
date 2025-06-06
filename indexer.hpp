#ifndef INDEXER_HPP
#define INDEXER_HPP

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QUrl>
#include <QMap>
#include <QStringList>
#include <QDateTime>

struct PageMetadata
{
	QDateTime timestamp;
	QString title;
	QUrl url;
	QMap<QString, int> wordsAndFrequencies;
};

class Indexer : public QObject
{
	Q_OBJECT
	QSqlDatabase mDatabase;
public:
	Indexer(QObject *parent = nullptr);
	~Indexer();
	bool initialize(const QString &dbPath);
	void addPage(const PageMetadata &metadata, const QMap<QString, int> &wordFrequencies, const QString &text);
	// TODO: SQLite FTS5
	QList<PageMetadata> searchWords(const QStringList &words) const;
};

#endif // INDEXER_HPP
