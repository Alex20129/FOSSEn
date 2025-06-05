#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QRegularExpression>
#include <QDebug>
#include "indexer.hpp"

Indexer::Indexer(QObject *parent) : QObject(parent)
{
	mDatabase = QSqlDatabase::addDatabase("QSQLITE", "index_db");
}

Indexer::~Indexer()
{
	if(mDatabase.isOpen())
	{
		mDatabase.close();
	}
	QSqlDatabase::removeDatabase("index_db");
}

bool Indexer::initialize(const QString &dbPath)
{
	mDatabase.setDatabaseName(dbPath);
	if (!mDatabase.open())
	{
		qWarning() << "Failed to open database:" << mDatabase.lastError().text();
		return false;
	}
	QSqlQuery query(mDatabase);
	query.exec("CREATE TABLE IF NOT EXISTS documents ("
			   "doc_id INTEGER PRIMARY KEY AUTOINCREMENT,"
			   "url TEXT UNIQUE NOT NULL,"
			   "title TEXT,"
			   "timestamp DATETIME)");
	query.exec("CREATE TABLE IF NOT EXISTS words ("
			   "word_id INTEGER PRIMARY KEY AUTOINCREMENT,"
			   "word TEXT UNIQUE NOT NULL)");
	query.exec("CREATE TABLE IF NOT EXISTS postings ("
			   "word_id INTEGER,"
			   "doc_id INTEGER,"
			   "frequency INTEGER NOT NULL,"
			   "positions TEXT,"
			   "PRIMARY KEY (word_id, doc_id),"
			   "FOREIGN KEY (word_id) REFERENCES words(word_id),"
			   "FOREIGN KEY (doc_id) REFERENCES documents(doc_id))");
	query.exec("CREATE INDEX IF NOT EXISTS idx_postings_word_id ON postings(word_id)");
	query.exec("CREATE INDEX IF NOT EXISTS idx_documents_url ON documents(url)");
	if (query.lastError().isValid())
	{
		qWarning() << "Failed to create tables:" << query.lastError().text();
		return false;
	}
	return true;
}

QString joinIntList(const QList<int> &list, const QString &separator)
{
	QStringList strList;
	for (int i : list)
	{
		strList.append(QString::number(i));
	}
	return strList.join(separator);
}

void Indexer::addDocument(const DocumentMetadata &metadata, const QMap<QString, int> &wordFrequencies, const QString &text)
{
	QSqlQuery query(mDatabase);
	mDatabase.transaction();

	query.prepare("INSERT OR IGNORE INTO documents (url, title, timestamp) VALUES (?, ?, ?)");
	query.addBindValue(metadata.url);
	query.addBindValue(metadata.title);
	query.addBindValue(metadata.timestamp);
	query.exec();

	if (query.lastError().isValid())
	{
		qWarning() << "Failed to insert document:" << query.lastError().text();
		mDatabase.rollback();
		return;
	}

	query.prepare("SELECT doc_id FROM documents WHERE url = ?");
	query.addBindValue(metadata.url);
	query.exec();
	if (!query.next())
	{
		qWarning() << "Failed to get doc_id for" << metadata.url;
		mDatabase.rollback();
		return;
	}
	int docId = query.value(0).toInt();

	QStringList tokens = text.toLower().split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);
	QMap<QString, QList<int>> wordPositions;
	for (int i = 0; i < tokens.size(); ++i)
	{
		if (wordFrequencies.contains(tokens[i]))
		{
			wordPositions[tokens[i]].append(i);
		}
	}

	for (auto it = wordFrequencies.constBegin(); it != wordFrequencies.constEnd(); it++)
	{
		const QString &word = it.key();
		int frequency = it.value();

		query.prepare("INSERT OR IGNORE INTO words (word) VALUES (?)");
		query.addBindValue(word);
		query.exec();

		query.prepare("SELECT word_id FROM words WHERE word = ?");
		query.addBindValue(word);
		query.exec();
		if (!query.next())
		{
			qWarning() << "Failed to get word_id for" << word;
			mDatabase.rollback();
			return;
		}
		int wordId = query.value(0).toInt();

		QString positions = joinIntList(wordPositions[word], QStringLiteral(","));
		query.prepare("INSERT OR REPLACE INTO postings (word_id, doc_id, frequency, positions) VALUES (?, ?, ?, ?)");
		query.addBindValue(wordId);
		query.addBindValue(docId);
		query.addBindValue(frequency);
		query.addBindValue(positions);
		query.exec();

		if (query.lastError().isValid())
		{
			qWarning() << "Failed to insert posting for" << word << ":" << query.lastError().text();
			mDatabase.rollback();
			return;
		}
	}

	mDatabase.commit();
}

// TODO: SQLite FTS5
QList<DocumentMetadata> Indexer::searchWords(const QStringList &words) const
{
	QList<DocumentMetadata> results;
	QSqlQuery query(mDatabase);

	QStringList conditions;
	for (int i = 0; i < words.size(); ++i)
	{
		conditions << QString("w.word = :word%1").arg(i);
	}
	QString sql = QString(
		"SELECT d.url, d.title, d.timestamp "
		"FROM documents d "
		"JOIN postings p ON d.doc_id = p.doc_id "
		"JOIN words w ON p.word_id = w.word_id "
		"WHERE %1 "
		"GROUP BY d.doc_id ").arg(conditions.join(" OR "));

	query.prepare(sql);
	for (int i = 0; i < words.size(); ++i)
	{
		query.bindValue(QString(":word%1").arg(i), words[i].toLower());
	}
	query.exec();

	while (query.next())
	{
		DocumentMetadata result;
		result.url = query.value("url").toString();
		result.title = query.value("title").toString();
		result.timestamp = query.value("timestamp").toDateTime();
		results.append(result);
	}

	if (query.lastError().isValid())
	{
		qWarning() << "Search failed:" << query.lastError().text();
	}
	return results;
}
