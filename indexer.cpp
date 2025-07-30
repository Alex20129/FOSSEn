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
	return results;
}

void Indexer::addPage(const PageMetadata &page_metadata)
{
	PageMetadata *pageMetaDataCopy=new PageMetadata(page_metadata);

	localIndexStorage.insert(page_metadata.contentHash, pageMetaDataCopy);

	for (auto it = page_metadata.words.constBegin(); it != page_metadata.words.constEnd(); it++)
	{
		const QString &word = it.key();
		localIndexTableOfContents[word].insert(page_metadata.contentHash);
	}
}
