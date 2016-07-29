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

#include <QtDebug>
#include <QRegularExpression>

namespace {

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

void appendTokens(std::vector<QString> *tokens, const QVariant &value)
{
    foreach (const QString &item, toStringList(value)) {
        tokens->push_back(item);
    }
}

std::vector<QString> patternTokens(const QString &pattern)
{
    std::vector<QString> rv;

    const QRegularExpression ws("\\s+");
    foreach (const QString &token, pattern.split(ws, QString::SkipEmptyParts)) {
        rv.push_back(token);
    }

    return rv;
}

bool matchTokens(const std::vector<QString> *tokens, const std::vector<QString> &patterns, Qt::CaseSensitivity caseSensitive)
{
    auto tbegin = tokens->cbegin(), tit = tbegin, tend = tokens->cend();
    for (auto pit = patterns.cbegin(), pend = patterns.cend(); pit != pend; ++pit) {
        for (tit = tbegin; tit != tend; ++tit) {
            if (tit->startsWith(*pit, caseSensitive))
                break;
        }
        if (tit == tend)
            return false;
    }

    return true;
}

}


SearchModel::SearchModel(QObject *parent)
    : BaseFilterModel(parent)
    , sensitivity_(Qt::CaseSensitive)
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
        patterns_ = patternTokens(pattern_);

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

bool SearchModel::filtered() const
{
    return !pattern_.isEmpty();
}

bool SearchModel::includeItem(int sourceRow) const
{
    if (pattern_.isEmpty())
        return true;

    std::shared_ptr<std::vector<QString>> &itemTokens = tokens_.at(sourceRow);
    if (!itemTokens) {
        itemTokens = std::move(searchTokens(sourceRow));
    }

    return matchTokens(itemTokens.get(), patterns_, sensitivity_);
}

std::unique_ptr<std::vector<QString>> SearchModel::searchTokens(int sourceRow) const
{
    auto tokens = new std::vector<QString>();

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

    for (auto it = roles_.cbegin(), end = roles_.cend(); it != end; ++it) {
        appendTokens(tokens, getSourceValue(sourceRow, *it));
    }
    for (auto it = properties_.cbegin(), end = properties_.cend(); it != end; ++it) {
        appendTokens(tokens, getSourceValue(sourceRow, *it));
    }

    return std::unique_ptr<std::vector<QString>>(tokens);
}

void SearchModel::searchTokensInvalidated()
{
    std::fill(tokens_.begin(), tokens_.end(), std::shared_ptr<std::vector<QString>>());
}

void SearchModel::setModel(QAbstractListModel *model)
{
    roles_.clear();
    properties_.clear();

    BaseFilterModel::setModel(model);
}

void SearchModel::sourceItemsInserted(int insertIndex, int insertCount)
{
    tokens_.reserve(tokens_.size() + insertCount);
    tokens_.insert(tokens_.begin() + insertIndex, insertCount, std::shared_ptr<std::vector<QString>>());
}

void SearchModel::sourceItemsMoved(int moveIndex, int moveCount, int insertIndex)
{
    std::vector<std::shared_ptr<std::vector<QString>>> movedItems;
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
    std::fill(tokens_.begin() + changeIndex, tokens_.begin() + (changeIndex + changeCount), std::shared_ptr<std::vector<QString>>());
}

void SearchModel::sourceItemsCleared()
{
    tokens_.clear();
}

