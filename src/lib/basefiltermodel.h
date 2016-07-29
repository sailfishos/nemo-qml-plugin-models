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

#ifndef BASEFILTERMODEL_H
#define BASEFILTERMODEL_H

#include <QAbstractListModel>
#include <QMetaProperty>

#include <vector>

class Q_DECL_EXPORT BaseFilterModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QObject *sourceModel READ sourceModel WRITE setSourceModel NOTIFY sourceModelChanged)
    Q_PROPERTY(bool populated READ populated NOTIFY populatedChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    explicit BaseFilterModel(QObject *parent = 0);

    void setSourceModel(QObject *model);
    QObject *sourceModel() const;

    bool populated() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE QVariant getRole(int row, int column, const QString &roleName) const;
    Q_INVOKABLE QVariant getRole(int row, int column, int role) const;
    Q_INVOKABLE QVariantMap getRoles(int row, int column) const;

signals:
    void sourceModelChanged();
    void populatedChanged();
    void countChanged();

protected slots:
    void sourceModelReset();
    void sourcePopulatedChanged();
    void sourceRowsInserted(const QModelIndex &parent, int first, int last);
    void sourceRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row);
    void sourceRowsRemoved(const QModelIndex &parent, int first, int last);
    void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void sourceLayoutChanged();

protected:
    void populateModel();

    int sourceRow(int row) const;

    virtual void setModel(QAbstractListModel *model);
    virtual bool includeItem(int sourceRow) const = 0;

    QVariant getSourceValue(int sourceRow, int role) const;
    QVariant getSourceValue(int sourceRow, const QMetaProperty &property) const;

    int findRole(const QString &roleName) const;
    QMetaProperty findProperty(const QByteArray &propertyName) const;

    QAbstractListModel *model_;
    QMetaProperty modelPopulated_;
    QMetaMethod objectGet_;
    bool populated_;
    std::vector<int> mapping_;
    std::vector<QPair<int, QByteArray>> roles_;
};

#endif // BASEFILTERMODEL_H
