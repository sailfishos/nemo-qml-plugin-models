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

#ifndef OBJECTLISTMODEL_H
#define OBJECTLISTMODEL_H

#include <nemomodels.h>
#include <QAbstractListModel>
#include <QVariantMap>

class NEMO_QML_PLUGIN_MODELS_EXPORT ObjectListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool automaticRoles READ automaticRoles WRITE setAutomaticRoles NOTIFY automaticRolesChanged)
    Q_PROPERTY(bool populated READ populated WRITE setPopulated NOTIFY populatedChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    explicit ObjectListModel(QObject *parent = 0, bool automaticRoles = false, bool populated = true);

    void setAutomaticRoles(bool enabled);
    bool automaticRoles() const;

    void setPopulated(bool populated);
    bool populated() const;

    int count() const { return rowCount(); }

    Q_INVOKABLE void insertItem(int index, QObject *item);

    Q_INVOKABLE void appendItem(QObject *item);
    void appendItems(const QList<QObject *> &items);

    Q_INVOKABLE void removeItem(QObject *item);
    void removeItems(const QList<QObject *> &items);
    Q_INVOKABLE void removeItemAt(int index);

    void moveItem(int oldIndex, int newIndex);

    void itemChanged(QObject *item);
    void itemChangedAt(int index);

    Q_INVOKABLE void clear();
    void deleteAll();

    Q_INVOKABLE QObject *get(int index) const;
    Q_INVOKABLE int indexOf(QObject *item) const;

    template<typename DerivedType>
    DerivedType *get(int index) const { return qobject_cast<DerivedType *>(get(index)); }

    QVariant itemRole(const QObject *item, int role) const;
    QVariantMap itemRoles(const QObject *item) const;

    bool updateItem(QObject *item, const QVariantMap &roles);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;

    template<typename T>
    void synchronizeList(const QList<T*> &list);
    void synchronizeList(const QList<QObject*> &list);

    int insertRange(int index, int count, const QList<QObject *> &source, int sourceIndex);
    int removeRange(int index, int count);

private slots:
    void objectDestroyed();

signals:
    void automaticRolesChanged();
    void populatedChanged();
    void countChanged();
    void itemAdded(QObject *item);
    void itemRemoved(QObject *item);

private:
    bool automaticRoles_;
    bool populated_;
    QHash<int, QByteArray> roles_;
    QList<QObject*> items_;
    QList<QObject*> insertions_;
    QList<QObject*> removals_;
};

template<typename T>
void ObjectListModel::synchronizeList(const QList<T*> &list)
{
    synchronizeList(reinterpret_cast<const QList<QObject *> &>(list));
}

#endif // OBJECTLISTMODEL_H
