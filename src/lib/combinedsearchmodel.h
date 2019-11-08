/*
 * Copyright (C) 2019 Open Mobile Platform LLC
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

#ifndef COMBINEDSEARCHMODEL_H
#define COMBINEDSEARCHMODEL_H

#include <QQmlParserStatus>
#include <QAbstractListModel>
#include <QVector>

class BaseFilterModel;
struct SourceModelRow {
    QObject *sourceModel = nullptr;
    int sourceRow = 0;
};
struct FilterModelRow {
    BaseFilterModel *filterModel = nullptr;
    int filterRow = 0;
};
struct SourceAndFilterModelRow {
    SourceModelRow sourceModelRow;
    FilterModelRow filterModelRow;
};
struct FilterModelRowRange {
    // the range of values in m_rows which come from this filter model.
    int startRow = 0;
    int rowCount = 0;
};

class Q_DECL_EXPORT CombinedSearchModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(bool populated READ populated NOTIFY populatedChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QList<QObject *> models READ models WRITE setModels NOTIFY modelsChanged)

public:
    explicit CombinedSearchModel(QObject *parent = 0);
    ~CombinedSearchModel();

    bool populated() const { return m_populated; }
    int count() const { return m_rows.count(); }

    QList<QObject *> models() const;
    void setModels(const QList<QObject *> &models);

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

    void classBegin() Q_DECL_OVERRIDE;
    void componentComplete() Q_DECL_OVERRIDE;

    // for synchronizeList
    int insertRange(int index, int count, const QVector<SourceAndFilterModelRow> &source, int sourceIndex);
    int removeRange(int index, int count);

signals:
    void populatedChanged();
    void countChanged();
    void modelsChanged();

private slots:
    void filterModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
    void filterModelHeaderDataChanged(Qt::Orientation orientation, int first, int last);
    void filterModelLayoutChanged(const QList<QPersistentModelIndex> &parents = QList<QPersistentModelIndex>(), QAbstractItemModel::LayoutChangeHint hint = QAbstractItemModel::NoLayoutChangeHint);
    void filterModelReset();
    void filterModelRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void filterModelRowsInserted(const QModelIndex &parent, int first, int last);
    void filterModelRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row);
    void filterModelRowsRemoved(const QModelIndex &parent, int first, int last);
    void filterModelPopulatedChanged();
    void sourceModelRowsChanged();

private:
    void populate();
    void injectRow(const SourceAndFilterModelRow &row, QVector<SourceAndFilterModelRow> &updatedRows, bool demoteDuplicate);
    void exciseRow(const SourceAndFilterModelRow &row, QVector<SourceAndFilterModelRow> &updatedRows, int beginSearchIndex = 0);

private:
    bool m_deferLoad;
    bool m_populated;
    mutable int m_sourceModelRole;
    QList<QObject *> m_deferredModels;
    QList<BaseFilterModel *> m_models;
    QList<const BaseFilterModel *> m_unpopulated;

    QVector<SourceAndFilterModelRow> m_rows; // the data we return/expose
    QHash<FilterModelRow, int> m_filterRows; // index of fmr in m_rows
    QHash<BaseFilterModel*, FilterModelRowRange> m_filterRowRanges; // the rows in m_rows coming from the filter model
    QVector<SourceAndFilterModelRow> m_duplicateRows; // the rows from filter models which are suppressed due to being duplicates
    QHash<int, SourceAndFilterModelRow> m_changeRows; // filterModel rows which are changing, built when handling rowsAboutToBeRemoved() etc.
};

#endif // COMBINEDSEARCHMODEL_H
