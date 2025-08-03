#include <QRegularExpression>
#include <QDebug>
#include "indexer.hpp"

Indexer::Indexer(QObject *parent) : QObject(parent)
{
}

Indexer::~Indexer()
{
	qDeleteAll(localIndexStorage);
	localIndexStorage.clear();
}

//TODO: init, save, load
void Indexer::initialize(const QString &db_path)
{
	qDebug("Indexer::initialize");
	qDebug()<<db_path;
}

void Indexer::load(const QString &db_path)
{
	qDebug("Indexer::load");
	qDebug()<<db_path;
}

void Indexer::save(const QString &db_path)
{
	qDebug("Indexer::save");
	qDebug()<<db_path;
}

void Indexer::merge(const Indexer &other)
{
	qDebug("Indexer::merge");
	for (auto it = other.localIndexStorage.constBegin(); it != other.localIndexStorage.constEnd(); it++)
	{
		uint64_t hash = it.key();
		if (!localIndexStorage.contains(hash))
		{
			PageMetadata *pageMetaDataCopy = new PageMetadata(*it.value());
			localIndexStorage.insert(hash, pageMetaDataCopy);
		}
	}
	for (auto it = other.localIndexTableOfContents.constBegin(); it != other.localIndexTableOfContents.constEnd(); it++)
	{
		const QString &word = it.key();
		const QSet<uint64_t> &hashes = it.value();
		localIndexTableOfContents[word].unite(hashes);
	}
	qDebug() << "Merged index: added" << other.localIndexStorage.size() << "pages and"
		<< other.localIndexTableOfContents.size() << "words";
}

QList<PageMetadata> Indexer::searchWords(const QStringList &words) const
{
	QList<PageMetadata> results;
	if (words.isEmpty())
	{
		return results;
	}
	QString smallestSetKey;
	int minSize = INT_MAX;
	for (const QString &word : words)
	{
		QString lowerWord = word.toLower();
		if (!localIndexTableOfContents.contains(lowerWord))
		{
			return results;
		}
		if (localIndexTableOfContents[lowerWord].size() < minSize)
		{
			minSize = localIndexTableOfContents[lowerWord].size();
			smallestSetKey = lowerWord;
		}
	}
	QSet<uint64_t> commonHashes = localIndexTableOfContents[smallestSetKey];
	for (const QString &word : words)
	{
		QString lowerWord = word.toLower();
		if (lowerWord != smallestSetKey)
		{
			commonHashes.intersect(localIndexTableOfContents[lowerWord]);
			if (commonHashes.isEmpty())
			{
				return results;
			}
		}
	}
	for (uint64_t hash : commonHashes)
	{
		if (localIndexStorage.contains(hash))
		{
			results.append(*localIndexStorage[hash]);
		}
	}
	return results;
}

void Indexer::addPage(PageMetadata page_metadata)
{
	if(page_metadata.words.isEmpty())
	{
		return;
	}
	if(page_metadata.url.isEmpty())
	{
		return;
	}
	if(localIndexStorage.contains(page_metadata.contentHash))
	{
		return;
	}
	PageMetadata *pageMetaDataCopy=new PageMetadata(page_metadata);
	localIndexStorage.insert(page_metadata.contentHash, pageMetaDataCopy);
	for (auto it = page_metadata.words.constBegin(); it != page_metadata.words.constEnd(); it++)
	{
		const QString word = it.key();
		localIndexTableOfContents[word].insert(page_metadata.contentHash);
	}
}
