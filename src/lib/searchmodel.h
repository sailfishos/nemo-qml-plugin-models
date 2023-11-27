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

#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <nemomodels.h>
#include "basefiltermodel.h"

#include <QList>
#include <QMetaMethod>

#include <memory>
#include <vector>

class NEMO_QML_PLUGIN_MODELS_EXPORT SearchModel : public BaseFilterModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList searchRoles READ searchRoles WRITE setSearchRoles NOTIFY searchRolesChanged)
    Q_PROPERTY(QStringList searchProperties READ searchProperties WRITE setSearchProperties NOTIFY searchPropertiesChanged)
    Q_PROPERTY(QString pattern READ pattern WRITE setPattern NOTIFY patternChanged)
    Q_PROPERTY(Qt::CaseSensitivity caseSensitivity READ caseSensitivity WRITE setCaseSensitivity NOTIFY caseSensitivityChanged)
    Q_PROPERTY(MatchType matchType READ matchType WRITE setMatchType NOTIFY matchTypeChanged)
    Q_ENUMS(MatchType)

public:
    enum MatchType {
        MatchBeginning,
        MatchAnywhere
    };

    typedef std::pair<std::vector<const QString *>, std::vector<const QString *>> TokenList;

    explicit SearchModel(QObject *parent = 0);

    void setSearchRoles(const QStringList &roles);
    QStringList searchRoles() const;

    void setSearchProperties(const QStringList &properties);
    QStringList searchProperties() const;

    void setPattern(const QString &pattern);
    QString pattern() const;

    void setCaseSensitivity(Qt::CaseSensitivity sensitivity);
    Qt::CaseSensitivity caseSensitivity() const;

    void setMatchType(MatchType type);
    MatchType matchType() const;

signals:
    void searchRolesChanged();
    void searchPropertiesChanged();
    void patternChanged();
    void caseSensitivityChanged();
    void matchTypeChanged();

protected:
    bool filtered() const override;
    bool includeItem(int sourceRow) const override;

    std::unique_ptr<TokenList> searchTokens(int sourceRow) const;
    void searchTokensInvalidated();

    void setModel(QAbstractItemModel *model) override;

    void sourceItemsInserted(int insertIndex, int insertCount) override;
    void sourceItemsMoved(int moveIndex, int moveCount, int insertIndex) override;
    void sourceItemsRemoved(int removeIndex, int removeCount) override;
    void sourceItemsChanged(int changeIndex, int changeCount) override;
    void sourceItemsCleared() override;

    QStringList roleNames_;
    QStringList propertyNames_;
    QString pattern_;
    Qt::CaseSensitivity sensitivity_;
    MatchType matchType_;

    mutable std::vector<int> roles_;
    mutable std::vector<QMetaProperty> properties_;
    mutable QList<QStringList> patterns_;

    mutable std::vector<std::shared_ptr<TokenList>> tokens_;
};

#endif // SEARCHMODEL_H
