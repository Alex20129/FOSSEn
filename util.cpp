#include "util.hpp"

#include <QRegularExpression>
#include <QSet>

QMap<QString, uint64_t> ExtractWordsAndFrequencies(const QString &text)
{
	static const QRegularExpression wordsRegex("\\W+");
	static const QRegularExpression digitsRegex("^[0-9]+$");
	static const QSet<QString> stopWords =
	{
		"the", "and", "for", "with", "was", "such"
	};
	QMap<QString, uint64_t> wordMap;
	QStringList words = text.toLower().split(wordsRegex, Qt::SkipEmptyParts);
	for (const QString &word : words)
	{
		if (word.length()>2 && word.length()<32)
		{
			if (!stopWords.contains(word) && !digitsRegex.match(word).hasMatch())
			{
				wordMap[word] += 1;
			}
		}
	}
	return wordMap;
}
