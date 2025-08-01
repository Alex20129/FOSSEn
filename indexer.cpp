#include <QRegularExpression>
#include <QDebug>
#include "indexer.hpp"

Indexer::Indexer(QObject *parent) : QObject(parent)
{
}

//TODO: open, save, load and merge
void Indexer::initialize(const QString &db_path)
{
}

void Indexer::load(const QString &db_path)
{
}

void Indexer::save(const QString &db_path)
{
}

void Indexer::merge(const Indexer &other)
{
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

void Indexer::addPage(const PageMetadata &page_metadata)
{
	if(page_metadata.words.isEmpty())
	{
		return;
	}
	if(page_metadata.url.isEmpty())
	{
		return;
	}
	PageMetadata *pageMetaDataCopy=new PageMetadata(page_metadata);
	localIndexStorage.insert(page_metadata.contentHash, pageMetaDataCopy);
	for (auto it = page_metadata.words.constBegin(); it != page_metadata.words.constEnd(); it++)
	{
		const QString &word = it.key().toLower();
		localIndexTableOfContents[word].insert(page_metadata.contentHash);
	}
}
