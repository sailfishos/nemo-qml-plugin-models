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

#include "objectlistmodel.h"
#include "synchronizelists.h"

#include <QQmlEngine>
#include <QMetaProperty>
#include <QDebug>

namespace {

enum {
    ObjectPointerRole = Qt::UserRole,
    RoleMapRole,
    FirstAutomaticRole,
};

QHash<int, QByteArray> rolesFromItem(QObject *item)
{
    QHash<int, QByteArray> rv;

    int role(FirstAutomaticRole);

    const QMetaObject *mo(item->metaObject());
    for (int i = 0, n = mo->propertyCount(); i < n; ++i) {
        const QMetaProperty prop(mo->property(i));
        if (prop.isReadable()) {
            rv.insert(role, prop.name());
            ++role;
        }
    }
    for (const auto name : item->dynamicPropertyNames()) {
        rv.insert(role, name);
        ++role;
    }

    return rv;
}

}

ObjectListModel::ObjectListModel(QObject *parent, bool automaticRoles, bool populated)
    : QAbstractListModel(parent)
    , automaticRoles_(automaticRoles)
    , populated_(populated)
{
}

void ObjectListModel::setAutomaticRoles(bool enabled)
{
    if (enabled != automaticRoles_) {
        automaticRoles_ = enabled;
        roles_.clear();

        emit automaticRolesChanged();
    }
}

bool ObjectListModel::automaticRoles() const
{
    return automaticRoles_;
}

void ObjectListModel::setPopulated(bool populated)
{
    if (populated != populated_) {
        populated_ = populated;

        emit populatedChanged();
    }
}

bool ObjectListModel::populated() const
{
    return populated_;
}

void ObjectListModel::insertItem(int index, QObject *item)
{
    if (automaticRoles_ && roles_.isEmpty() && items_.isEmpty()) {
        // Special case: we need to derive the roles from this first item, and reset the model
        roles_ = rolesFromItem(item);
        beginResetModel();
        items_.insert(index, item);
        endResetModel();
    } else {
        beginInsertRows(QModelIndex(), index, index);
        items_.insert(index, item);
        connect(item, &QObject::destroyed, this, &ObjectListModel::objectDestroyed);
        endInsertRows();
    }

    emit itemAdded(item);
    emit countChanged();
}

void ObjectListModel::appendItem(QObject *item)
{
    insertItem(items_.count(), item);
}

void ObjectListModel::appendItems(const QList<QObject *> &items)
{
    if (!items.isEmpty()) {
        if (automaticRoles_ && roles_.isEmpty() && items_.isEmpty()) {
            // Special case: we need to derive the roles from the first item
            insertItem(0, items.at(0));
            appendItems(items.mid(1));
            return;
        }

        const int index(items_.count());
        beginInsertRows(QModelIndex(), index, (index + items.count() - 1));
        for (QObject *item : items) {
            items_.append(item);
            connect(item, &QObject::destroyed, this, &ObjectListModel::objectDestroyed);
        }
        endInsertRows();

        for (QObject *item : items) {
            emit itemAdded(item);
        }
        emit countChanged();
    }
}

void ObjectListModel::removeItem(QObject *item)
{
    removeItemAt(items_.indexOf(item));
}

void ObjectListModel::removeItems(const QList<QObject *> &items)
{
    QList<QPair<int, QObject *> > removals;
    for (QObject *item : items) {
        int index = items_.indexOf(item);
        if (index != -1) {
            removals.append(qMakePair(index, item));
        }
    }

    if (!removals.isEmpty()) {
        std::sort(removals.begin(), removals.end(), [](const QPair<int, QObject *> &lhs, const QPair<int, QObject *> &rhs) { return lhs.first < rhs.first; });

        int count(removals.count());
        while (count > 0) {
            // Find any contiguous runs of removal indexes to be processed together
            int last = count - 1;
            int first = last;
            while (first > 0 && removals.at(first - 1).first == (removals.at(first).first - 1)) {
                --first;
            }

            beginRemoveRows(QModelIndex(), removals.at(first).first, removals.at(last).first);
            while (last >= first) {
                const QPair<int, QObject *> &removal(removals.at(last));
                --last;

                items_.removeAt(removal.first);
                disconnect(removal.second, &QObject::destroyed, this, &ObjectListModel::objectDestroyed);
            }
            endRemoveRows();

            count = first;
        }

        QList<QPair<int, QObject *> >::const_iterator it = removals.constBegin(), end = removals.constEnd();
        for ( ; it != end; ++it) {
            emit itemRemoved(it->second);
        }

        emit countChanged();
    }
}

void ObjectListModel::removeItemAt(int index)
{
    if (index >= 0 && index < items_.size()) {
        QObject *item(items_.at(index));
        beginRemoveRows(QModelIndex(), index, index);
        items_.removeAt(index);
        disconnect(item, &QObject::destroyed, this, &ObjectListModel::objectDestroyed);
        endRemoveRows();

        emit itemRemoved(item);
        emit countChanged();
    }
}


void ObjectListModel::moveItem(int oldIndex, int newIndex)
{
    if (oldIndex >= 0 && oldIndex < items_.size() && newIndex >= 0 && newIndex < items_.size()) {
        beginMoveRows(QModelIndex(), oldIndex, oldIndex, QModelIndex(), (newIndex > oldIndex) ? (newIndex + 1) : newIndex);
        items_.move(oldIndex, newIndex);
        endMoveRows();
    }
}

void ObjectListModel::itemChanged(QObject *item)
{
    itemChangedAt(items_.indexOf(item));
}

void ObjectListModel::itemChangedAt(int index)
{
    if (index >= 0 && index < items_.size()) {
        const QModelIndex changedIndex(this->index(index, 0));
        emit dataChanged(changedIndex, changedIndex);
    }
}

void ObjectListModel::clear()
{
    if (items_.isEmpty())
        return;

    beginRemoveRows(QModelIndex(), 0, items_.size());
    for (QObject *item : items_) {
        emit itemRemoved(item);
    }
    items_.clear();
    endRemoveRows();

    emit countChanged();
}

void ObjectListModel::deleteAll()
{
    qDeleteAll(items_);
    populated_ = false;
    
    emit countChanged();
    emit populatedChanged();
}

QObject* ObjectListModel::get(int index) const
{
    if (index >= 0 && index < items_.size()) {
        QObject *item(items_.at(index));
        QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        return item;
    }

    return 0;
}

int ObjectListModel::indexOf(QObject *item) const
{
    return items_.indexOf(item);
}

QVariant ObjectListModel::itemRole(const QObject *item, int role) const
{
    QHash<int, QByteArray>::const_iterator it = roles_.find(role);
    return it != roles_.end() ? item->property(*it) : QVariant();
}

QVariantMap ObjectListModel::itemRoles(const QObject *item) const
{
    QVariantMap rv;

    if (automaticRoles_) {
        for (QHash<int, QByteArray>::const_iterator it = roles_.cbegin(), end = roles_.cend(); it != end; ++it) {
            rv.insert(QString::fromUtf8(it.value()), item->property(it.value()));
        }
    }

    return rv;
}

bool ObjectListModel::updateItem(QObject *item, const QVariantMap &roles)
{
    bool rv = false;

    const QMetaObject *mo(item->metaObject());
    const QList<QByteArray> dynamicPropertyNames(item->dynamicPropertyNames());

    for (QVariantMap::const_iterator it = roles.cbegin(), end = roles.cend(); it != end; ++it) {
        const QByteArray roleName(it.key().toUtf8());
        const QVariant &value(it.value());

        if (dynamicPropertyNames.contains(roleName)) {
            if (item->property(roleName) != value) {
                item->setProperty(roleName, value);
                rv = true;
            }
        } else {
            int propertyIndex(mo->indexOfProperty(roleName));
            if (propertyIndex != -1) {
                QMetaProperty property(mo->property(propertyIndex));
                if (property.isValid() && property.isWritable()) {
                    if (property.read(item) != value) {
                        property.write(item, value);

                        QMetaMethod notify(property.notifySignal());
                        if (notify.isValid()) {
                            notify.invoke(item);
                        }

                        rv = true;
                    }
                    continue;
                }
            }

            qWarning() << "Unable to update object property:" << roleName;
        }
    }

    return rv;
}

int ObjectListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return items_.size();
}

QHash<int, QByteArray> ObjectListModel::roleNames() const
{
    QHash<int, QByteArray> rv;
    if (automaticRoles_) {
        rv = roles_;
        rv.insert(RoleMapRole, "roles");
    }
    rv.insert(ObjectPointerRole, "object");
    return rv;
}

QVariant ObjectListModel::data(const QModelIndex &index, int role) const
{
    if (index.parent().isValid())
        return QVariant();

    if (role >= ObjectPointerRole) {
        const int row(index.row());
        if (row >= 0 && row < items_.size()) {
            QObject *item(items_.at(row));
            if (role == ObjectPointerRole) {
                return QVariant::fromValue(item);
            } else if (automaticRoles_) {
                return role == RoleMapRole ? QVariant::fromValue(itemRoles(item)) : itemRole(item, role);
            }
        }
    }

    return QVariant();
}

void ObjectListModel::synchronizeList(const QList<QObject *> &list)
{
    ::synchronizeList(this, items_, list);

    // Report addition/removals after synch completes, because a move may cause an
    // item to be both removed and added transiently
    for (QObject *item : insertions_) {
        emit itemAdded(item);
    }
    for (QObject *item : removals_) {
        emit itemRemoved(item);
    }

    if (!insertions_.isEmpty() || !removals_.isEmpty()) {
        emit countChanged();
    }

    insertions_.clear();
    removals_.clear();
}

int ObjectListModel::insertRange(int index, int count, const QList<QObject *> &source, int sourceIndex)
{
    const int end = index + count - 1;
    beginInsertRows(QModelIndex(), index, end);

    for (int i = 0; i < count; ++i) {
        QObject *item(source.at(sourceIndex + i));
        items_.insert(index + i, item);
        int removedIndex = removals_.indexOf(item);
        if (removedIndex != -1) {
            removals_.removeAt(removedIndex);
        } else {
            insertions_.append(item);
        }
    }

    endInsertRows();
    return end - index + 1;
}

int ObjectListModel::removeRange(int index, int count)
{
    const int end = index + count - 1;
    beginRemoveRows(QModelIndex(), index, end);

    for (int i = 0; i < count; ++i) {
        QObject *item(items_.at(index));
        int insertedIndex = insertions_.indexOf(item);
        if (insertedIndex != -1) {
            insertions_.removeAt(insertedIndex);
        } else {
            removals_.append(item);
        }
        items_.removeAt(index);
    }

    endRemoveRows();
    return 0;
}

void ObjectListModel::objectDestroyed()
{
    removeItem(QObject::sender());
}

