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

#ifndef FILTERMODEL_H
#define FILTERMODEL_H

#include <nemomodels.h>
#include "basefiltermodel.h"

#include <QList>
#include <QMetaMethod>

class NEMO_QML_PLUGIN_MODELS_EXPORT FilterModel : public BaseFilterModel
{
    Q_OBJECT
    Q_PROPERTY(QVariantList filters READ filters WRITE setFilters NOTIFY filtersChanged)
    Q_PROPERTY(FilterRequirement filterRequirement READ filterRequirement WRITE setFilterRequirement NOTIFY filterRequirementChanged)
    Q_ENUMS(FilterRequirement)

public:
    enum FilterRequirement {
        PassAllFilters,
        PassAnyFilter,
    };
    enum Comparator {
        None,
        Equal,
        LessThan,
        LessThanEqual,
        HasMatch,
        ElementEqual,
        ElementHasMatch,
    };

    explicit FilterModel(QObject *parent = 0);

    void setFilters(const QVariantList &filters);
    QVariantList filters() const;

    void setFilterRequirement(FilterRequirement requirement);
    FilterRequirement filterRequirement() const;

signals:
    void filtersChanged();
    void filterRequirementChanged();

protected:
    struct FilterData {
        mutable int role_;
        mutable QMetaProperty property_;
        mutable bool initialized_;
        bool negate_;
        FilterModel::Comparator comparator_;
        QVariant value_;
        QByteArray roleName_;
        QByteArray propertyName_;

        FilterData(const QString &role, const QString &property, const QVariant &value, FilterModel::Comparator comparator, bool negate);

        bool operator==(const FilterData &other) const;
        bool operator!=(const FilterData &other) const { return !operator==(other); }
    };

    bool filtered() const override;
    bool includeItem(int sourceRow) const override;

    bool passesFilter(int sourceRow, const FilterData &filter) const;
    QVariant itemValue(int sourceRow, const FilterData &filter) const;

    QList<FilterData> filters_;
    FilterRequirement requirement_;
};

#endif // FILTERMODEL_H
