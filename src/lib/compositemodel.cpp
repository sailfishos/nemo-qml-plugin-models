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

#include "compositemodel.h"

#include <QDateTime>
#include <QDebug>
#include <QMetaObject>
#include <QMetaProperty>

namespace {

int modelOffset(const QList<QAbstractListModel *> &models, QAbstractListModel *model)
{
    int offset = 0;
    for (auto it = models.cbegin(), end = models.cend(); it != end; ++it) {
        if (*it == model)
            return offset;

        offset += (*it)->rowCount(QModelIndex());
    }

    return -1;
}

int compositeCount(const QList<QAbstractListModel *> &models)
{
    int total = 0;
    for (auto it = models.cbegin(), end = models.cend(); it != end; ++it) {
        total += (*it)->rowCount(QModelIndex());
    }

    return total;
}

}

CompositeModel::CompositeModel(QObject *parent) :
    QAbstractListModel(parent),
    m_populated(false),
    m_count(0),
    m_sourceModelRole(-1)
{
}

CompositeModel::~CompositeModel()
{
}

QList<QObject *> CompositeModel::models() const
{
    QList<QObject *> rv;
    rv.reserve(m_models.count());

    for (auto it = m_models.cbegin(), end = m_models.cend(); it != end; ++it) {
        rv.append(*it);
    }

    return rv;
}

void CompositeModel::setModels(const QList<QObject *> &models)
{
    beginResetModel();

    while (!m_models.isEmpty()) {
        QAbstractListModel *oldModel = m_models.takeLast();
        disconnect(oldModel);
    }

    m_count = 0;
    m_models.clear();
    m_unpopulated.clear();

    for (auto it = models.cbegin(), end = models.cend(); it != end; ++it) {
        if (QAbstractListModel *newModel = qobject_cast<QAbstractListModel *>(*it)) {
            connect(newModel, &QAbstractListModel::columnsAboutToBeInserted, this, &CompositeModel::columnsAboutToBeInserted);
            connect(newModel, &QAbstractListModel::columnsAboutToBeMoved, this, &CompositeModel::columnsAboutToBeMoved);
            connect(newModel, &QAbstractListModel::columnsAboutToBeRemoved, this, &CompositeModel::columnsAboutToBeRemoved);
            connect(newModel, &QAbstractListModel::columnsInserted, this, &CompositeModel::columnsInserted);
            connect(newModel, &QAbstractListModel::columnsMoved, this, &CompositeModel::columnsMoved);
            connect(newModel, &QAbstractListModel::columnsRemoved, this, &CompositeModel::columnsRemoved);
            connect(newModel, &QAbstractListModel::dataChanged, this, &CompositeModel::sourceDataChanged);
            connect(newModel, &QAbstractListModel::headerDataChanged, this, &CompositeModel::sourceHeaderDataChanged);
            connect(newModel, &QAbstractListModel::layoutAboutToBeChanged, this, &CompositeModel::sourceLayoutAboutToBeChanged);
            connect(newModel, &QAbstractListModel::layoutChanged, this, &CompositeModel::sourceLayoutChanged);
            connect(newModel, &QAbstractListModel::modelAboutToBeReset, this, &CompositeModel::sourceModelAboutToBeReset);
            connect(newModel, &QAbstractListModel::modelReset, this, &CompositeModel::sourceModelReset);
            connect(newModel, &QAbstractListModel::rowsAboutToBeInserted, this, &CompositeModel::sourceRowsAboutToBeInserted);
            connect(newModel, &QAbstractListModel::rowsAboutToBeMoved, this, &CompositeModel::sourceRowsAboutToBeMoved);
            connect(newModel, &QAbstractListModel::rowsAboutToBeRemoved, this, &CompositeModel::sourceRowsAboutToBeRemoved);
            connect(newModel, &QAbstractListModel::rowsInserted, this, &CompositeModel::sourceRowsInserted);
            connect(newModel, &QAbstractListModel::rowsMoved, this, &CompositeModel::sourceRowsMoved);
            connect(newModel, &QAbstractListModel::rowsRemoved, this, &CompositeModel::sourceRowsRemoved);

            m_count += newModel->rowCount();
            m_models.append(newModel);

            int populatedIndex = newModel->metaObject()->indexOfProperty("populated");
            if (populatedIndex != -1) {
                QMetaProperty populatedProperty = newModel->metaObject()->property(populatedIndex);
                if (populatedProperty.isValid() && populatedProperty.hasNotifySignal()) {
                    // We have populated properties to report
                    if (populatedProperty.read(newModel).toBool() == false) {
                        // This model is not yet populated
                        int handlerIndex = this->metaObject()->indexOfSlot("sourcePopulatedChanged()");
                        QMetaMethod handler = this->metaObject()->method(handlerIndex);
                        if (handler.isValid()) {
                            m_unpopulated.append(newModel);
                            connect(newModel, populatedProperty.notifySignal(), this, handler);
                        }
                    }
                }
            }
        }
    }

    m_populated = !m_models.isEmpty() && m_unpopulated.isEmpty();

    endResetModel();

    if (m_populated) {
        emit countChanged();
    }
    emit populatedChanged();
}

int CompositeModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_count;
}

QVariant CompositeModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        int row = index.row();
        for (auto it = m_models.cbegin(), end = m_models.cend(); it != end; ++it) {
            if (row < (*it)->rowCount()) {
                if (role == m_sourceModelRole) {
                    return (*it)->objectName();
                }
                return (*it)->data((*it)->index(row, index.column()), role);
            } else {
                row -= (*it)->rowCount();
            }
        }
    }

    return QVariant();
}

QHash<int, QByteArray> CompositeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    if (!m_models.isEmpty()) {
        roles = m_models.first()->roleNames();

        int maxRole = 0;
        for (auto it = roles.cbegin(), end = roles.cend(); it != end; ++it) {
            maxRole = qMax(maxRole, it.key());
        }
        m_sourceModelRole = maxRole + 1;
        roles.insert(m_sourceModelRole, QByteArray("sourceModel"));
    }
    return roles;
}

void CompositeModel::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    if (!topLeft.parent().isValid() && !bottomRight.parent().isValid()) {
        if (QAbstractListModel *model = qobject_cast<QAbstractListModel *>(sender())) {
            if (int offset = modelOffset(m_models, model) >= 0) {
                const QModelIndex offsetTopLeft(index(topLeft.row() + offset, topLeft.column()));
                const QModelIndex offsetBottomRight(index(bottomRight.row() + offset, bottomRight.column()));
                emit dataChanged(offsetTopLeft, offsetBottomRight, roles);
            }
        }
    }
}

void CompositeModel::sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    if (QAbstractListModel *model = qobject_cast<QAbstractListModel *>(sender())) {
        if (int offset = modelOffset(m_models, model) >= 0) {
            emit headerDataChanged(orientation, first + offset, last + offset);
        }
    }
}

void CompositeModel::sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint)
{
    Q_UNUSED(parents)
    Q_UNUSED(hint)

    // I don't know how to handle this...
    beginResetModel();
}

void CompositeModel::sourceLayoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint)
{
    Q_UNUSED(parents)
    Q_UNUSED(hint)

    m_count = compositeCount(m_models);
    endResetModel();
    emit countChanged();
}

void CompositeModel::sourceModelAboutToBeReset()
{
    beginResetModel();
}

void CompositeModel::sourceModelReset()
{
    m_count = compositeCount(m_models);
    endResetModel();
    emit countChanged();
}

void CompositeModel::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    if (!parent.isValid()) {
        if (QAbstractListModel *model = qobject_cast<QAbstractListModel *>(sender())) {
            if (int offset = modelOffset(m_models, model) >= 0) {
                beginInsertRows(parent, start + offset, end + offset);
            }
        }
    }
}

void CompositeModel::sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
{
    if (!sourceParent.isValid()) {
        if (QAbstractListModel *model = qobject_cast<QAbstractListModel *>(sender())) {
            if (int offset = modelOffset(m_models, model) >= 0) {
                beginMoveRows(sourceParent, sourceStart + offset, sourceEnd + offset, destinationParent, destinationRow + offset);
            }
        }
    }
}

void CompositeModel::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    if (!parent.isValid()) {
        if (QAbstractListModel *model = qobject_cast<QAbstractListModel *>(sender())) {
            if (int offset = modelOffset(m_models, model) >= 0) {
                beginRemoveRows(parent, first + offset, last + offset);
            }
        }
    }
}

void CompositeModel::sourceRowsInserted(const QModelIndex &parent, int first, int last)
{
    if (!parent.isValid()) {
        m_count += (last - first) + 1;
        endInsertRows();
        emit countChanged();
    }
}

void CompositeModel::sourceRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row)
{
    Q_UNUSED(start)
    Q_UNUSED(end)
    Q_UNUSED(destination)
    Q_UNUSED(row)

    if (!parent.isValid()) {
        endMoveRows();
    }
}

void CompositeModel::sourceRowsRemoved(const QModelIndex &parent, int first, int last)
{
    if (!parent.isValid()) {
        m_count -= (last - first) + 1;
        endRemoveRows();
        emit countChanged();
    }
}

void CompositeModel::sourcePopulatedChanged()
{
    if (QAbstractListModel *model = qobject_cast<QAbstractListModel *>(sender())) {
        int index = m_unpopulated.indexOf(model);
        if (index >= 0) {
            m_unpopulated.removeAt(index);
            if (m_unpopulated.isEmpty()) {
                m_populated = true;
                m_count = compositeCount(m_models);
                emit populatedChanged();
            }
        }
    }
}

