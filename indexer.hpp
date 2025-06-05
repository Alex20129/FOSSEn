#ifndef INDEXER_HPP
#define INDEXER_HPP

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QUrl>
#include <QMap>
#include <QStringList>
#include <QDateTime>

struct DocumentMetadata
{
	QString title;
	QUrl url;
	QDateTime timestamp;
};

class Indexer : public QObject
{
	Q_OBJECT
	QSqlDatabase mDatabase;
public:
	Indexer(QObject *parent = nullptr);
	~Indexer();
	bool initialize(const QString &dbPath);
	void addDocument(const DocumentMetadata &metadata, const QMap<QString, int> &wordFrequencies, const QString &text);
	// TODO: SQLite FTS5
	QList<DocumentMetadata> searchWords(const QStringList &words) const;
};

#endif // INDEXER_HPP
