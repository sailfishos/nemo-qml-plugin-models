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

#include "basefiltermodel.h"

#include <QtDebug>

BaseFilterModel::BaseFilterModel(QObject *parent)
    : QAbstractListModel(parent)
    , model_(0)
    , populated_(false)
{
}

void BaseFilterModel::setSourceModel(QObject *model)
{
    if (model != static_cast<QObject *>(model_)) {
        const bool wasPopulated(populated_);
        setModel(qobject_cast<QAbstractListModel *>(model));

        if (populated_ != wasPopulated) {
            emit populatedChanged();
        }
        emit sourceModelChanged();
        emit countChanged();
    }
}

QObject *BaseFilterModel::sourceModel() const
{
    return model_;
}

bool BaseFilterModel::populated() const
{
    return populated_;
}

int BaseFilterModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return mapping_.size();
}

QHash<int, QByteArray> BaseFilterModel::roleNames() const
{
    if (model_)
        return model_->roleNames();

    return this->QAbstractListModel::roleNames();
}

QVariant BaseFilterModel::data(const QModelIndex &index, int role) const
{
    if (index.parent().isValid())
        return QVariant();

    return getRole(index.row(), index.column(), role);
}

QVariant BaseFilterModel::getRole(int row, int column, const QString &roleName) const
{
    auto it = std::find_if(roles_.cbegin(), roles_.cend(), [&roleName](const QPair<int, QByteArray> &pair) { return roleName == pair.second; });
    if (it == roles_.end())
        return QVariant();

    return getRole(row, column, it->first);
}

QVariant BaseFilterModel::getRole(int row, int column, int role) const
{
    return model_->data(model_->index(sourceRow(row), column), role);
}

QVariantMap BaseFilterModel::getRoles(int row, int column) const
{
    QVariantMap rv;

    const int source(sourceRow(row));
    for (auto it = roles_.cbegin(), end = roles_.cend(); it != end; ++it) {
        QVariant value = getRole(source, column, it->first);
        if (value.isValid()) {
            rv.insert(QString::fromUtf8(it->second), value);
        }
    }

    return rv;
}

void BaseFilterModel::sourceModelReset()
{
    populateModel();
}

void BaseFilterModel::sourcePopulatedChanged()
{
    const bool populated = modelPopulated_.read(model_).toBool();
    if (populated != populated_) {
        populated_ = populated;

        emit populatedChanged();
    }
}

void BaseFilterModel::sourceRowsInserted(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid())
        return;

    sourceItemsInserted(first, (last - first + 1));

    std::vector<int> insertItems;
    for (int i = first; i <= last; ++i)
        if (includeItem(i))
            insertItems.push_back(i);

    if (insertItems.empty())
        return;

    auto firstIt = std::lower_bound(mapping_.begin(), mapping_.end(), first);
    const int insertIndex(firstIt - mapping_.begin());
    const int insertCount(insertItems.size());

    beginInsertRows(QModelIndex(), insertIndex, insertIndex + insertCount - 1);
    mapping_.insert(firstIt, insertItems.cbegin(), insertItems.cend());
    itemsInserted(insertIndex, insertCount);
    endInsertRows();

    emit countChanged();
}

void BaseFilterModel::sourceRowsMoved(const QModelIndex &parent, int first, int last, const QModelIndex &destination, int row)
{
    if (parent.isValid() || destination.isValid())
        return;

    auto firstIt = std::lower_bound(mapping_.begin(), mapping_.end(), first);
    if (firstIt == mapping_.end())
        return;

    auto destinationIt = std::lower_bound(mapping_.begin(), mapping_.end(), row);
    if (destinationIt == firstIt)
        return;

    auto lastIt = firstIt;
    for (auto end = mapping_.end(); lastIt != end; ++lastIt) {
        if (*lastIt > last)
            break;
    }

    if (lastIt == firstIt)
        return;

    const int moveIndex(firstIt - mapping_.begin());
    const int moveCount(lastIt - firstIt);
    const int destinationIndex(destinationIt - mapping_.begin());

    beginMoveRows(QModelIndex(), moveIndex, moveIndex + moveCount - 1, QModelIndex(), destinationIndex);
    mapping_.erase(firstIt, lastIt);
    auto insertIt = std::lower_bound(mapping_.begin(), mapping_.end(), row);
    const int insertIndex(insertIt - mapping_.begin());
    for (int i = 0; i < moveCount; ++i, ++insertIt) {
        insertIt = mapping_.insert(insertIt, row + i);
    }

    sourceItemsMoved(first, (last - first + 1), row);
    itemsMoved(moveIndex, moveCount, insertIndex);

    endMoveRows();
}

void BaseFilterModel::sourceRowsRemoved(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid())
        return;

    auto firstIt = std::lower_bound(mapping_.begin(), mapping_.end(), first);
    if (firstIt == mapping_.end())
        return;

    auto lastIt = firstIt;
    for (auto end = mapping_.end(); lastIt != end; ++lastIt) {
        if (*lastIt > last)
            break;
    }

    if (lastIt == firstIt)
        return;

    sourceItemsRemoved(first, (last - first + 1));

    const int removeIndex(firstIt - mapping_.begin());
    const int removeCount(lastIt - firstIt);

    beginRemoveRows(QModelIndex(), removeIndex, removeIndex + removeCount - 1);
    mapping_.erase(firstIt, lastIt);
    itemsRemoved(removeIndex, removeCount);
    endRemoveRows();

    emit countChanged();
}

void BaseFilterModel::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    if (topLeft.parent().isValid())
        return;

    const int first(topLeft.row());
    auto firstIt = std::lower_bound(mapping_.begin(), mapping_.end(), first);
    if (firstIt == mapping_.end())
        return;
    
    const int firstIndex(firstIt - mapping_.begin());

    const int last(bottomRight.row());
    auto lastIt = std::lower_bound(mapping_.begin(), mapping_.end(), last);

    int lastIndex(lastIt - mapping_.begin());
    if (lastIt != mapping_.end() && *lastIt == last)
        ++lastIndex;

    sourceItemsChanged(first, (last - first + 1));
    itemsChanged(firstIndex, (lastIndex - firstIndex + 1));

    emit dataChanged(index(firstIndex, topLeft.column()), index(lastIndex, bottomRight.column()), roles);
}

void BaseFilterModel::sourceLayoutChanged()
{
    // Not supported - just do a reset
    populateModel();
}

void BaseFilterModel::populateModel()
{
    const size_t previousCount(mapping_.size());

    beginResetModel();

    sourceItemsCleared();
    if (model_)
        sourceItemsInserted(0, model_->rowCount());

    buildMapping(false);

    endResetModel();

    if (previousCount != mapping_.size())
        emit countChanged();
}

void BaseFilterModel::buildMapping(bool reportChanges)
{
    // TODO: synchronize_lists would be useful here
    const int removeCount(mapping_.size());
    if (removeCount) {
        if (reportChanges) {
            beginRemoveRows(QModelIndex(), 0, removeCount - 1);
        }
        mapping_.clear();
        itemsCleared();
        if (reportChanges) {
            endRemoveRows();
        }
    }

    const int sourceItemCount(model_->rowCount());
    if (sourceItemCount) {
        std::vector<int> insertItems;
        insertItems.reserve(sourceItemCount);
        if (filtered()) {
            for (int i = 0, n = model_->rowCount(); i < n; ++i) {
                if (includeItem(i)) {
                    insertItems.push_back(i);
                }
            }
        } else {
            insertItems.resize(sourceItemCount);
            std::iota(insertItems.begin(), insertItems.end(), 0);
        }

        if (!insertItems.empty()) {
            const int insertCount(insertItems.size());
            if (reportChanges) {
                beginInsertRows(QModelIndex(), 0, insertCount - 1);
            }
            mapping_.assign(insertItems.cbegin(), insertItems.cend());
            itemsInserted(0, insertCount);
            if (reportChanges) {
                endInsertRows();
            }
        }
    }

    if (reportChanges) {
        emit countChanged();
    }
}

void BaseFilterModel::refineMapping()
{
    std::vector<int> removeIndices;

    // Test if any of the current items should now be excluded
    for (auto begin = mapping_.begin(), it = begin, end = mapping_.end(); it != end; ++it) {
        if (!includeItem(*it)) {
            removeIndices.push_back(it - begin);
        }
    }

    if (!removeIndices.empty()) {
        std::reverse(removeIndices.begin(), removeIndices.end());
        for (auto it = removeIndices.cbegin(), end = removeIndices.cend(); it != end; ) {
            auto rangeLast = it, rangeFirst = rangeLast;
            for (auto next = it + 1; next != end; ++next) {
                if (*next == (*rangeFirst - 1)) {
                    rangeFirst = next;
                } else {
                    break;
                }
            }

            // Remove this range
            int removeIndex = *rangeFirst;
            int removeCount = (*rangeLast - *rangeFirst + 1);
            beginRemoveRows(QModelIndex(), removeIndex, removeIndex + removeCount - 1);
            mapping_.erase(mapping_.begin() + removeIndex, mapping_.begin() + removeIndex + removeCount);
            itemsRemoved(removeIndex, removeCount);
            endRemoveRows();

            it += removeCount;
        }

        emit countChanged();
    }
}

void BaseFilterModel::unrefineMapping()
{
    std::vector<std::pair<int, std::vector<int>>> insertIndices;

    // Test if any of the currently excluded items should now be included
    std::vector<int> include;
    int lastIndex = -1;
    for (auto begin = mapping_.begin(), it = begin, end = mapping_.end(); it != end; ++it) {
        const int index(*it);
        if (index != lastIndex + 1) {
            for (int sourceRow = lastIndex + 1; sourceRow < index; ++sourceRow) {
                if (includeItem(sourceRow)) {
                    include.push_back(sourceRow);
                }
            }
            if (!include.empty()) {
                insertIndices.push_back(std::make_pair((it - begin), include));
                include.clear();
            }
        }
        lastIndex = index;
    }

    const int lastSourceRow = model_->rowCount() - 1;
    if (lastIndex < lastSourceRow) {
        for (int sourceRow = lastIndex + 1; sourceRow <= lastSourceRow; ++sourceRow) {
            if (includeItem(sourceRow)) {
                include.push_back(sourceRow);
            }
        }
        if (!include.empty()) {
            insertIndices.push_back(std::make_pair(mapping_.size(), include));
        }
    }

    if (!insertIndices.empty()) {
        std::reverse(insertIndices.begin(), insertIndices.end());
        for (auto it = insertIndices.cbegin(), end = insertIndices.cend(); it != end; ++it) {
            const int insertIndex = it->first;
            const std::vector<int> &insertItems = it->second;
            const int insertCount = insertItems.size();

            beginInsertRows(QModelIndex(), insertIndex, insertIndex + insertCount - 1);
            mapping_.insert(mapping_.begin() + insertIndex, insertItems.cbegin(), insertItems.cend());
            itemsInserted(insertIndex, insertCount);
            endInsertRows();
        }

        emit countChanged();
    }
}

int BaseFilterModel::sourceRow(int row) const
{
    return mapping_.at(row);
}

int BaseFilterModel::indexForSourceRow(int sourceRow) const
{
    auto it = std::lower_bound(mapping_.cbegin(), mapping_.cend(), sourceRow);
    return (it == mapping_.end() || *it != sourceRow) ? -1 : (it - mapping_.cbegin());
}

void BaseFilterModel::setModel(QAbstractListModel *model)
{
    if (model_) {
        disconnect(model_);
    }

    sourceItemsCleared();

    modelPopulated_ = QMetaProperty();
    objectGet_ = QMetaMethod();
    populated_ = false;
    roles_.clear();
    mapping_.clear();
    itemsCleared();

    model_ = model;
    if (model_) {
        connect(model_, &QAbstractListModel::modelReset, this, &BaseFilterModel::sourceModelReset);
        connect(model_, &QAbstractListModel::rowsInserted, this, &BaseFilterModel::sourceRowsInserted);
        connect(model_, &QAbstractListModel::rowsMoved, this, &BaseFilterModel::sourceRowsMoved);
        connect(model_, &QAbstractListModel::rowsRemoved, this, &BaseFilterModel::sourceRowsRemoved);
        connect(model_, &QAbstractListModel::dataChanged, this, &BaseFilterModel::sourceDataChanged);
        connect(model_, &QAbstractListModel::layoutChanged, this, &BaseFilterModel::sourceLayoutChanged);

        const QHash<int, QByteArray> &roles(model_->roleNames());
        roles_.reserve(roles.size());
        for (auto it = roles.cbegin(), end = roles.cend(); it != end; ++it) {
            roles_.push_back(qMakePair(it.key(), it.value()));
        }

        // Find the 'populated' property in this model, if present
        const QMetaObject *mo(model_->metaObject());
        modelPopulated_ = mo->property(mo->indexOfProperty("populated"));
        if (modelPopulated_.isValid()) {
            QMetaMethod handler(metaObject()->method(metaObject()->indexOfMethod("sourcePopulatedChanged()")));
            connect(model_, modelPopulated_.notifySignal(), this, handler);
        }

        objectGet_ = mo->method(mo->indexOfMethod("get(int)"));

        if (!modelPopulated_.isValid() || modelPopulated_.read(model_).toBool()) {
            populateModel();
            populated_ = true;
        }
    }
}

QVariant BaseFilterModel::getSourceValue(int sourceRow, int role) const
{
    return model_->data(model_->index(sourceRow, 0), role);
}

QVariant BaseFilterModel::getSourceValue(int sourceRow, const QMetaProperty &property) const
{
    QObject *obj = 0;
    if (objectGet_.isValid() && objectGet_.invoke(model_, Q_RETURN_ARG(QObject*, obj), Q_ARG(int, sourceRow))) {
        if (obj)
            return property.read(obj);
    }
    return QVariant();
}

int BaseFilterModel::findRole(const QString &roleName) const
{
    if (model_) {
        const QHash<int, QByteArray> &roles = model_->roleNames();
        for (auto it = roles.cbegin(), end = roles.cend(); it != end; ++it) {
            if (it.value() == roleName) {
                return it.key();
            }
        }
    }

    qWarning() << "No matching role in model:" << roleName;
    return -1;
}

QMetaProperty BaseFilterModel::findProperty(const QByteArray &propertyName) const
{
    QMetaProperty property;

    if (model_ && model_->rowCount() > 0) {
        if (objectGet_.isValid()) {
            QObject *obj;
            if (objectGet_.invoke(model_, Q_RETURN_ARG(QObject*, obj), Q_ARG(int, 0))) {
                if (obj) {
                    const QMetaObject *mo(obj->metaObject());
                    property = mo->property(mo->indexOfProperty(propertyName));
                    if (!property.isValid()) {
                        qWarning() << "No matching property in object:" << obj << propertyName;
                    }
                } else {
                    qWarning() << "Could not retrieve valid object:" << model_;
                }
            } else {
                qWarning() << "Could not invoke get:" << model_;
            }
        } else {
            qWarning() << "No object get function in model:" << model_;
        }
    }

    return property;
}

void BaseFilterModel::sourceItemsInserted(int, int) {}
void BaseFilterModel::itemsInserted(int, int) {}
void BaseFilterModel::sourceItemsMoved(int, int, int) {}
void BaseFilterModel::itemsMoved(int, int, int) {}
void BaseFilterModel::sourceItemsRemoved(int, int) {}
void BaseFilterModel::itemsRemoved(int, int) {}
void BaseFilterModel::sourceItemsChanged(int, int) {}
void BaseFilterModel::itemsChanged(int, int) {}
void BaseFilterModel::sourceItemsCleared() {}
void BaseFilterModel::itemsCleared() {}

