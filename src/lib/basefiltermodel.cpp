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
    for (int i = 0; i < moveCount; ++i, ++insertIt) {
        insertIt = mapping_.insert(insertIt, row + i);
    }
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

    const int removeIndex(firstIt - mapping_.begin());
    const int removeCount(lastIt - firstIt);

    beginRemoveRows(QModelIndex(), removeIndex, removeIndex + removeCount - 1);
    mapping_.erase(firstIt, lastIt);
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

    emit dataChanged(index(firstIndex, topLeft.column()), index(lastIndex, bottomRight.column()), roles);
}

void BaseFilterModel::sourceLayoutChanged()
{
    // Not supported - just do a reset
    populateModel();
}

void BaseFilterModel::populateModel()
{
    mapping_.clear();

    beginResetModel();

    if (model_) {
        for (int i = 0, n = model_->rowCount(); i < n; ++i) {
            if (includeItem(i)) {
                mapping_.push_back(i);
            }
        }
    }

    endResetModel();

    emit countChanged();
}

int BaseFilterModel::sourceRow(int row) const
{
    return mapping_.at(row);
}

void BaseFilterModel::setModel(QAbstractListModel *model)
{
    if (model_) {
        disconnect(model_);
    }

    modelPopulated_ = QMetaProperty();
    populated_ = false;
    roles_.clear();
    mapping_.clear();
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

        if (!modelPopulated_.isValid() || modelPopulated_.read(model_).toBool()) {
            populateModel();
            populated_ = true;
        }
    }
}

