#include "util.hpp"

#include <QRegularExpression>
#include <QSet>

QMap<QString, uint64_t> ExtractWordsAndFrequencies(const QString &text)
{
	const QString lowerText=text.toLower();
	static const QRegularExpression wordsRegex("[^a-zа-яё]+");
	static const QRegularExpression digitsRegex("^[0-9]+$");
	QMap<QString, uint64_t> wordMap;
	const QStringList words = lowerText.split(wordsRegex, Qt::SkipEmptyParts);
	for (const QString &word : words)
	{
		if (word.length()>2 && word.length()<33)
		{
			if (!digitsRegex.match(word).hasMatch())
			{
				wordMap[word] += 1;
			}
		}
	}
	return wordMap;
}
