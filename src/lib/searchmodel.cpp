/*
 * Copyright (C) 2016 Jolla Ltd.
 * Contact: Matt Vogt <matthew.vogt@jollamobile.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Jolla Ltd. nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#include "searchmodel.h"

#include <QSequentialIterable>
#include <MLocale>
#include <MBreakIterator>

#include <QtDebug>

namespace {

const ML10N::MLocale mLocale;

QMap<uint, QString> decompositionMapping()
{
    QMap<uint, QString> rv;

    rv.insert(0x00df, QStringLiteral("ss")); // sharp-s ('sz' ligature)
    rv.insert(0x1e9e, QStringLiteral("SS"));
    rv.insert(0x00e6, QStringLiteral("ae")); // 'ae' ligature
    rv.insert(0x00c6, QStringLiteral("AE"));
    rv.insert(0x00f0, QStringLiteral("d"));  // eth
    rv.insert(0x00d0, QStringLiteral("D"));
    rv.insert(0x00f8, QStringLiteral("o"));  // o with stroke
    rv.insert(0x00d8, QStringLiteral("O"));
    rv.insert(0x00fe, QStringLiteral("th")); // thorn
    rv.insert(0x00de, QStringLiteral("TH"));
    rv.insert(0x0111, QStringLiteral("d"));  // d with stroke
    rv.insert(0x0110, QStringLiteral("D"));
    rv.insert(0x0127, QStringLiteral("h"));  // h with stroke
    rv.insert(0x0126, QStringLiteral("H"));
    rv.insert(0x0138, QStringLiteral("k"));  // kra
    rv.insert(0x0142, QStringLiteral("l"));  // l with stroke
    rv.insert(0x0141, QStringLiteral("L"));
    rv.insert(0x014b, QStringLiteral("n"));  // eng
    rv.insert(0x014a, QStringLiteral("N"));
    rv.insert(0x0153, QStringLiteral("oe")); // 'oe' ligature
    rv.insert(0x0152, QStringLiteral("OE"));
    rv.insert(0x0167, QStringLiteral("t"));  // t with stroke
    rv.insert(0x0166, QStringLiteral("T"));
    rv.insert(0x017f, QStringLiteral("s"));  // long s

    return rv;
}

QStringList tokenize(const QString &word)
{

#if QT_VERSION < 0x051500
    static const QSet<QString> alphabet(mLocale.exemplarCharactersIndex().toSet());
#else
    static const QSet<QString> alphabet(mLocale.exemplarCharactersIndex().begin(), mLocale.exemplarCharactersIndex().end());
#endif

    static const QMap<uint, QString> decompositions(decompositionMapping());

    // Convert the word to canonical form
    QString canonical(word.normalized(QString::NormalizationForm_C));

    QStringList tokens;

    ML10N::MBreakIterator it(mLocale, canonical, ML10N::MBreakIterator::CharacterIterator);
    while (it.hasNext()) {
        const int position = it.next();
        const int nextPosition = it.peekNext();
        if (position < nextPosition) {
            const QString character(canonical.mid(position, (nextPosition - position)));
            QStringList matches;
            if (alphabet.contains(character)) {
                // This character is a member of the alphabet for this locale - do not decompose it
                matches.append(character);
            } else {
                // This character is not a member of the alphabet; decompose it to
                // assist with diacritic-insensitive matching
                QString normalized(character.normalized(QString::NormalizationForm_D));
                matches.append(normalized);

                // For some characters, we want to match alternative spellings that do not correspond
                // to decomposition characters
                const uint codePoint(normalized.at(0).unicode());
                QMap<uint, QString>::const_iterator dit = decompositions.find(codePoint);
                if (dit != decompositions.end()) {
                    matches.append(*dit);
                }
            }

            if (tokens.isEmpty()) {
                tokens.append(QString());
            }

            int previousCount = tokens.count();
            for (int i = 1; i < matches.count(); ++i) {
                // Make an additional copy of the existing tokens, for each new possible match
                for (int j = 0; j < previousCount; ++j) {
                    tokens.append(tokens.at(j) + matches.at(i));
                }
            }
            for (int j = 0; j < previousCount; ++j) {
                tokens[j].append(matches.at(0));
            }
        }
    }

    return tokens;
}

QList<const QString *> makeSearchToken(const QString &word)
{
    static QMap<uint, const QString *> indexedTokens;
    static QHash<QString, QList<const QString *> > indexedWords;

    QHash<QString, QList<const QString *> >::const_iterator wit = indexedWords.find(word);
    if (wit == indexedWords.end()) {
        QList<const QString *> indexed;

        // Index these tokens for later dereferencing
        for (const QString &token : tokenize(word)) {
            uint hashValue(qHash(token));
            QMap<uint, const QString *>::const_iterator tit = indexedTokens.find(hashValue);
            if (tit == indexedTokens.end()) {
                tit = indexedTokens.insert(hashValue, new QString(token));
            }
            indexed.append(*tit);
        }

        wit = indexedWords.insert(word, indexed);
    }

    return *wit;
}

// Splits a string at word boundaries identified by MBreakIterator
QStringList splitWords(const QString &string)
{
    QStringList rv;

    ML10N::MBreakIterator it(mLocale, string, ML10N::MBreakIterator::WordIterator);
    while (it.hasNext()) {
        const int position = it.next();
        const QString word(string.mid(position, (it.peekNext() - position)).trimmed());
        if (!word.isEmpty()) {
            const bool apostrophe(word.length() == 1 && word.at(0) == QChar('\''));
            if (apostrophe && !rv.isEmpty()) {
                // Special case - a trailing apostrophe is not counted as a component of the
                // previous word, although it is included in the word if there is a following character
                rv.last().append(word);
            } else {
                // Ignore single punctuation marks
                if (word.count() > 1 || !word.at(0).isPunct()) {
                    rv.append(word);
                }
            }
        }
    }

    return rv;
}

QList<const QString *> searchTokens(const QString &string)
{
    QList<const QString *> rv;

    for (const QString &word : splitWords(string)) {
        for (const QString *alternative : makeSearchToken(word)) {
            rv.append(alternative);
        }
    }

    return rv;
}

struct LessThanIndirect {
    template<typename T>
    bool operator()(T lhs, T rhs) const { return *lhs < *rhs; }
};

struct EqualIndirect {
    template<typename T>
    bool operator()(T lhs, T rhs) const { return *lhs == *rhs; }
};

struct FirstElementLessThanIndirect {
    template<typename Container>
    bool operator()(const Container *lhs, typename Container::const_iterator rhs) { return *lhs->cbegin() < *rhs; }

    template<typename Container>
    bool operator()(typename Container::const_iterator lhs, const Container *rhs) { return *lhs < *rhs->cbegin(); }
};

QStringList toStringList(const QVariant &value)
{
    QStringList rv;

    if (static_cast<QMetaType::Type>(value.type()) == QMetaType::QStringList) {
        rv = value.value<QStringList>();
    } else if (value.canConvert<QVariantList>()) {
        QSequentialIterable iterable = value.value<QSequentialIterable>();
        for (auto it = iterable.begin(), end = iterable.end(); it != end; ++it) {
            rv.append(toStringList(*it));
        }
    } else {
        if (value.canConvert<QString>()) {
            rv.append(value.value<QString>());
        }
    }

    return rv;
}

void appendTokens(SearchModel::TokenList *tokens, const QVariant &value)
{
    for (const QString &item : toStringList(value)) {
        for (const QString *word : searchTokens(item)) {
            tokens->first.push_back(word);
        }

        // Also index search text in lower case for case insensitive search
        const QString lowered(mLocale.toLower(item));
        for (const QString *word : searchTokens(lowered)) {
            tokens->second.push_back(word);
        }
    }
}

QList<QStringList> patternTokens(const QString &string, Qt::CaseSensitivity caseSensitive)
{
    QList<QStringList> rv;

    // Test case insensitive searches in lower case
    const QString pattern(caseSensitive == Qt::CaseInsensitive ? mLocale.toLower(string) : string);
    for (const QString &word : splitWords(pattern)) {
        rv.append(tokenize(word));
    }

    return rv;
}

static const QChar *cbegin(const QString &s) { return s.cbegin(); }
static const QChar *cend(const QString &s) { return s.cend(); }

static const QChar *cbegin(const QStringRef &r) { return r.data(); }
static const QChar *cend(const QStringRef &r) { return r.data() + r.size(); }

template <typename StringType>
bool partialMatch(const StringType &key, const QChar * const vbegin, const QChar * const vend)
{
    // Note: both key and value must already be in normalization form D
    const QChar *kbegin = cbegin(key), *kend = cend(key);

    const QChar *vit = vbegin, *kit = kbegin;
    while (kit != kend) {
        if (*kit != *vit)
            break;

        // Preceding base chars match - are there any continuing diacritics?
        QString::const_iterator vmatch = vit++, kmatch = kit++;
        while (vit != vend && (*vit).category() == QChar::Mark_NonSpacing)
             ++vit;
        while (kit != kend && (*kit).category() == QChar::Mark_NonSpacing)
             ++kit;

        if ((vit - vmatch) > 1) {
            // The match value contains diacritics - the key needs to match them
            const QString subValue(QString::fromRawData(vmatch, vit - vmatch));
            const QString subKey(QString::fromRawData(kmatch, kit - kmatch));
            if (subValue.compare(subKey) != 0)
                break;
        } else {
            // Ignore any diacritics in our key
        }

        if (vit == vend) {
            // We have matched to the end of the value
            return true;
        }
    }

    return false;
}

bool partialMatch(const std::vector<const QString *> &tokens, const QString &value, SearchModel::MatchType type)
{
    const QChar *vbegin = value.cbegin(), *vend = value.cend();

    if (type == SearchModel::MatchBeginning) {
        // Find which subset of keys the value might match
        typedef std::vector<const QString *>::const_iterator VectorIterator;
        std::pair<VectorIterator, VectorIterator> bounds = std::equal_range(tokens.cbegin(), tokens.cend(), vbegin, FirstElementLessThanIndirect());
        for ( ; bounds.first != bounds.second; ++bounds.first) {
            const QString &key(*(*bounds.first));
            if (partialMatch(key, vbegin, vend))
                return true;
        }
    } else if (type == SearchModel::MatchAnywhere) {
        // Test all tokens that contain the initial character (in normalization form D)
        for (const QString *token : tokens) {
            // Test each possible location in the token
            for (auto begin = token->cbegin(), it = begin, end = token->cend(); it != end; ) {
                it = std::find(it, end, *vbegin);
                if (it != end) {
                    const QStringRef key(token->midRef((it - begin)));
                    if (partialMatch(key, vbegin, vend))
                        return true;

                    ++it;
                }
            }
        }
    }

    return false;
}

bool matchTokens(const std::vector<const QString *> &tokens, const QList<QStringList> &patterns, SearchModel::MatchType type)
{
    for (const QStringList &part : patterns) {
        bool match = false;
        for (const QString &alternative : part) {
            if (partialMatch(tokens, alternative, type)) {
                match = true;
                break;
            }
        }
        if (!match) {
            return false;
        }
    }

    return true;
}

}


SearchModel::SearchModel(QObject *parent)
    : BaseFilterModel(parent)
    , sensitivity_(Qt::CaseSensitive)
    , matchType_(MatchBeginning)
{
}

void SearchModel::setSearchRoles(const QStringList &roles)
{
    if (roles != roleNames_) {
        roleNames_ = roles;
        roles_.clear();
        searchTokensInvalidated();

        if (populated_ && model_) {
            buildMapping();
        }

        emit searchRolesChanged();
    }
}

QStringList SearchModel::searchRoles() const
{
    return roleNames_;
}

void SearchModel::setSearchProperties(const QStringList &properties)
{
    if (properties != propertyNames_) {
        propertyNames_ = properties;
        properties_.clear();
        searchTokensInvalidated();

        if (populated_ && model_) {
            buildMapping();
        }

        emit searchRolesChanged();
    }
}

QStringList SearchModel::searchProperties() const
{
    return propertyNames_;
}

void SearchModel::setPattern(const QString &pattern)
{
    if (pattern != pattern_) {
        const bool refinement(!pattern_.isEmpty() && pattern.startsWith(pattern_));
        const bool unrefinement(pattern_.startsWith(pattern));

        pattern_ = pattern;
        patterns_ = patternTokens(pattern_, sensitivity_);

        if (populated_ && model_) {
            if (refinement) {
                refineMapping();
            } else if (unrefinement) {
                unrefineMapping();
            } else {
                buildMapping();
            }
        }

        emit patternChanged();
    }
}

QString SearchModel::pattern() const
{
    return pattern_;
}

void SearchModel::setCaseSensitivity(Qt::CaseSensitivity sensitivity)
{
    if (sensitivity != sensitivity_) {
        sensitivity_ = sensitivity;
        patterns_ = patternTokens(pattern_, sensitivity_);

        if (populated_ && model_) {
            const bool refinement(!pattern_.isEmpty() && sensitivity_ == Qt::CaseSensitive);
            const bool unrefinement(!pattern_.isEmpty() && sensitivity_ == Qt::CaseInsensitive);
            if (refinement) {
                refineMapping();
            } else if (unrefinement) {
                unrefineMapping();
            } else {
                buildMapping();
            }
        }

        emit caseSensitivityChanged();
    }
}

Qt::CaseSensitivity SearchModel::caseSensitivity() const
{
    return sensitivity_;
}

void SearchModel::setMatchType(MatchType type)
{
    if (type != matchType_) {
        matchType_ = type;

        if (populated_ && model_) {
            buildMapping();
        }

        emit matchTypeChanged();
    }
}

SearchModel::MatchType SearchModel::matchType() const
{
    return matchType_;
}

bool SearchModel::filtered() const
{
    return !pattern_.isEmpty();
}

bool SearchModel::includeItem(int sourceRow) const
{
    if (pattern_.isEmpty())
        return true;

    std::shared_ptr<TokenList> &itemTokens = tokens_.at(sourceRow);
    if (!itemTokens) {
        itemTokens = std::move(searchTokens(sourceRow));
    }

    return matchTokens((sensitivity_ == Qt::CaseInsensitive ? itemTokens->second : itemTokens->first), patterns_, matchType_);
}

std::unique_ptr<SearchModel::TokenList> SearchModel::searchTokens(int sourceRow) const
{
    auto rv = new TokenList;

    if (roles_.empty() && !roleNames_.empty()) {
        for (auto it = roleNames_.cbegin(), end = roleNames_.cend(); it != end; ++it) {
            int role = findRole(*it);
            if (role != -1) {
                roles_.push_back(role);
            }
        }
    }
    if (properties_.empty() && !propertyNames_.empty()) {
        for (auto it = propertyNames_.cbegin(), end = propertyNames_.cend(); it != end; ++it) {
            const QMetaProperty property = findProperty(it->toUtf8());
            if (property.isValid()) {
                properties_.push_back(property);
            }
        }
    }

    TokenList tokens;
    for (auto it = roles_.cbegin(), end = roles_.cend(); it != end; ++it) {
        appendTokens(&tokens, getSourceValue(sourceRow, *it));
    }
    for (auto it = properties_.cbegin(), end = properties_.cend(); it != end; ++it) {
        appendTokens(&tokens, getSourceValue(sourceRow, *it));
    }

    auto copySorted = [](std::vector<const QString *> &src, std::vector<const QString *> &dst) {
        std::sort(src.begin(), src.end(), LessThanIndirect());
        src.erase(std::unique(src.begin(), src.end(), EqualIndirect()), src.end());

        dst.reserve(src.size());
        std::copy(src.cbegin(), src.cend(), std::back_inserter(dst));
    };
    if (!tokens.first.empty()) {
        copySorted(tokens.first, rv->first);
    }
    if (!tokens.second.empty()) {
        copySorted(tokens.second, rv->second);
    }

    return std::unique_ptr<TokenList>(rv);
}

void SearchModel::searchTokensInvalidated()
{
    std::fill(tokens_.begin(), tokens_.end(), std::shared_ptr<TokenList>());
}

void SearchModel::setModel(QAbstractItemModel *model)
{
    roles_.clear();
    properties_.clear();

    BaseFilterModel::setModel(model);
}

void SearchModel::sourceItemsInserted(int insertIndex, int insertCount)
{
    tokens_.reserve(tokens_.size() + insertCount);
    tokens_.insert(tokens_.begin() + insertIndex, insertCount, std::shared_ptr<TokenList>());
}

void SearchModel::sourceItemsMoved(int moveIndex, int moveCount, int insertIndex)
{
    std::vector<std::shared_ptr<TokenList>> movedItems;
    movedItems.reserve(moveCount);

    auto first = tokens_.begin() + moveIndex;
    auto last = first + moveCount;
    std::copy(first, last, std::back_inserter(movedItems));

    tokens_.erase(first, last);
    tokens_.insert(tokens_.begin() + insertIndex, movedItems.begin(), movedItems.end());
}

void SearchModel::sourceItemsRemoved(int removeIndex, int removeCount)
{
    tokens_.erase(tokens_.begin() + removeIndex, tokens_.begin() + (removeIndex + removeCount));
}

void SearchModel::sourceItemsChanged(int changeIndex, int changeCount)
{
    std::fill(tokens_.begin() + changeIndex, tokens_.begin() + (changeIndex + changeCount), std::shared_ptr<TokenList>());
}

void SearchModel::sourceItemsCleared()
{
    tokens_.clear();
}

