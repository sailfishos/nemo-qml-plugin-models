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


#include "filtermodel.h"

#include <QRegularExpression>
#include <QSequentialIterable>
#include <QtDebug>

namespace {

const std::vector<QPair<QString, QPair<FilterModel::Comparator, bool>>> &comparatorDetails()
{
    static std::vector<QPair<QString, QPair<FilterModel::Comparator, bool>>> details;
    if (details.empty()) {
        details.push_back(qMakePair(QStringLiteral("=="), qMakePair(FilterModel::Equal, false)));
        details.push_back(qMakePair(QStringLiteral("!="), qMakePair(FilterModel::Equal, true)));
        details.push_back(qMakePair(QStringLiteral("<"), qMakePair(FilterModel::LessThan, false)));
        details.push_back(qMakePair(QStringLiteral(">="), qMakePair(FilterModel::LessThan, true)));
        details.push_back(qMakePair(QStringLiteral(">"), qMakePair(FilterModel::LessThanEqual, true)));
        details.push_back(qMakePair(QStringLiteral("<="), qMakePair(FilterModel::LessThanEqual, false)));
        details.push_back(qMakePair(QStringLiteral("match"), qMakePair(FilterModel::HasMatch, false)));
        details.push_back(qMakePair(QStringLiteral("!match"), qMakePair(FilterModel::HasMatch, true)));
        details.push_back(qMakePair(QStringLiteral("contains"), qMakePair(FilterModel::ElementEqual, false)));
        details.push_back(qMakePair(QStringLiteral("!contains"), qMakePair(FilterModel::ElementEqual, true)));
        details.push_back(qMakePair(QStringLiteral("elementMatch"), qMakePair(FilterModel::ElementHasMatch, false)));
        details.push_back(qMakePair(QStringLiteral("!elementMatch"), qMakePair(FilterModel::ElementHasMatch, true)));
    }
    return details;
}

QPair<FilterModel::Comparator, bool> comparatorType(const QString &comparatorText)
{
    const std::vector<QPair<QString, QPair<FilterModel::Comparator, bool>>> &details(comparatorDetails());
    auto it = std::find_if(details.cbegin(), details.cend(), [&comparatorText](const QPair<QString, QPair<FilterModel::Comparator, bool>> &pair) { return comparatorText == pair.first; });
    if (it != details.end()) {
        return it->second;
    }

    return qMakePair(FilterModel::None, false);
}

QString comparatorName(const QPair<FilterModel::Comparator, bool> &comparatorType)
{
    const std::vector<QPair<QString, QPair<FilterModel::Comparator, bool>>> &details(comparatorDetails());
    auto it = std::find_if(details.cbegin(), details.cend(), [&comparatorType](const QPair<QString, QPair<FilterModel::Comparator, bool>> &pair) { return comparatorType == pair.second; });
    if (it != details.end()) {
        return it->first;
    }

    return QString();
}

}

FilterModel::FilterData::FilterData(const QString &role, const QString &property, const QVariant &value, FilterModel::Comparator comparator, bool negate)
    : role_(-1)
    , initialized_(false)
    , negate_(negate)
    , comparator_(comparator)
    , value_(value)
    , roleName_(role.toUtf8())
    , propertyName_(property.toUtf8())
{
}

bool FilterModel::FilterData::operator==(const FilterData &other) const
{
    return (roleName_ == other.roleName_ && propertyName_ == other.propertyName_ && value_ == other.value_ && comparator_ == other.comparator_ && negate_ == other.negate_);
}


FilterModel::FilterModel(QObject *parent)
    : BaseFilterModel(parent)
    , requirement_(PassAllFilters)
{
}

void FilterModel::setFilters(const QVariantList &filters)
{
    QList<FilterData> newFilters;

    foreach (const QVariant &var, filters) {
        const QVariantMap &filter = qvariant_cast<QVariantMap>(var);

        if (!filter.contains("comparator") || !filter.contains("value") || (!filter.contains("property") && !filter.contains("role"))) {
            qWarning() << "Invalid filter specified:" << filter;
        } else if (filter.contains("property") && filter.contains("role")) {
            qWarning() << "Invalid filter - cannot use both property and role:" << filter;
        } else {
            const QString role(filter["role"].value<QString>());
            const QString property(filter["property"].value<QString>());
            const QString comparator(filter["comparator"].value<QString>());
            const QVariant &value(filter["value"]);
            const QPair<FilterModel::Comparator, bool> type(comparatorType(comparator));
            newFilters.append(FilterData(role, property, value, type.first, type.second));
        }
    }

    bool changed(newFilters.count() != filters_.count());
    if (!changed) {
        QList<FilterData>::const_iterator it = newFilters.constBegin(), end = newFilters.constEnd(), oldIt = filters_.constBegin();
        for ( ; it != end; ++it, ++oldIt) {
            if (*it != *oldIt) {
                changed = true;
                break;
            }
        }
    }
    if (changed) {
        filters_ = newFilters;

        if (populated_ && model_) {
            populateModel();
        }

        emit filtersChanged();
    }
}

QVariantList FilterModel::filters() const
{
    QVariantList rv;

    QList<FilterData>::const_iterator it = filters_.constBegin(), end = filters_.constEnd();
    for ( ; it != end; ++it) {
        QVariantMap filter;
        if (!it->roleName_.isEmpty())
            filter.insert("role", QString::fromUtf8(it->roleName_));
        if (!it->propertyName_.isEmpty())
            filter.insert("property", QString::fromUtf8(it->propertyName_));
        filter.insert("comparator", comparatorName(qMakePair(it->comparator_, it->negate_)));
        filter.insert("value", it->value_);
        rv.append(filter);
    }

    return rv;
}

void FilterModel::setFilterRequirement(FilterRequirement requirement)
{
    if (requirement != requirement_) {
        requirement_ = requirement;

        if (populated_ && model_) {
            populateModel();
        }

        emit filterRequirementChanged();
    }
}

FilterModel::FilterRequirement FilterModel::filterRequirement() const
{
    return requirement_;
}

bool FilterModel::filtered() const
{
    return !filters_.isEmpty();
}

bool FilterModel::includeItem(int sourceRow) const
{
    if (!filters_.isEmpty()) {
        const bool passAll(requirement_ == PassAllFilters);

        foreach (const FilterData &filter, filters_) {
            const bool passed = passesFilter(sourceRow, filter);
            if (passAll && !passed) {
                return false;
            } else if (!passAll && passed) {
                return true;
            }
        }

        return passAll ? true : false;
    }

    return true;
}

bool FilterModel::passesFilter(int sourceRow, const FilterData &filter) const
{
    if (filter.comparator_ == FilterModel::None)
        return true;

    const QVariant value(itemValue(sourceRow, filter));

    QRegularExpression re;
    if (filter.comparator_ == FilterModel::HasMatch || filter.comparator_ == FilterModel::ElementHasMatch) {
        re.setPattern(filter.value_.toString());
    }

    if (filter.comparator_ == FilterModel::Equal) {
        if ((value == filter.value_) == filter.negate_)
            return false;
    } else if (filter.comparator_ == FilterModel::LessThan) {
        if ((value < filter.value_) == filter.negate_)
            return false;
    } else if (filter.comparator_ == FilterModel::LessThanEqual) {
        if ((value <= filter.value_) == filter.negate_)
            return false;
    } else if (filter.comparator_ == FilterModel::HasMatch) {
        if (re.match(value.toString()).hasMatch() == filter.negate_)
            return false;
    } else {
        auto testElement = [&filter, &re](const QVariant &element) -> bool {
            if (filter.comparator_ == FilterModel::ElementEqual) {
                if (element == filter.value_)
                    return true;
            } else if (filter.comparator_ == FilterModel::ElementHasMatch) {
                if (re.match(element.toString()).hasMatch())
                    return true;
            }
            return false;
        };

        // List comparisons - fail only if no element meets the criterion
        if (static_cast<QMetaType::Type>(value.type()) == QMetaType::QStringList) {
            const QStringList elements(value.value<QStringList>());
            auto it = elements.cbegin(), end = elements.cend();
            for ( ; it != end; ++it) {
                if (testElement(*it))
                    break;
            }
            if ((it == end) != filter.negate_) {
                return false;
            }
        } else if (value.canConvert<QVariantList>()) {
            QSequentialIterable iterable = value.value<QSequentialIterable>();
            auto it = iterable.begin(), end = iterable.end();
            for ( ; it != end; ++it) {
                if (testElement(*it))
                    break;
            }
            if ((it == end) != filter.negate_) {
                return false;
            }
        } else if (value.canConvert<QObject *>()) {
            /*
            QObject *obj = value.value<QObject *>();
            int count = obj->property("count").toInt();
            if (count > 0) {
                if (QAbstractListModel *model = qobject_cast<QAbstractListModel *>(obj)) {
                    for (int i = 0; i < count; ++i) {
                        // TODO: access the data of these elements
                    }
                }
            }
            */
        }
    }

    return true;
}

QVariant FilterModel::itemValue(int sourceRow, const FilterData &filter) const
{
    if (filter.role_ != -1) {
        return getSourceValue(sourceRow, filter.role_);
    } else if (filter.property_.isValid()) {
        return getSourceValue(sourceRow, filter.property_);
    }

    if (!filter.initialized_) {
        filter.initialized_ = true;
        if (!filter.roleName_.isEmpty()) {
            filter.role_ = findRole(filter.roleName_);
        } else if (!filter.propertyName_.isEmpty()) {
            filter.property_ = findProperty(filter.propertyName_);
        }

        // Try again after initialization
        return itemValue(sourceRow, filter);
    }

    return QVariant();
}

