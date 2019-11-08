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

#include "combinedsearchmodel.h"
#include "basefiltermodel.h"
#include "synchronizelists.h"

#include <QDateTime>
#include <QDebug>
#include <QMetaObject>
#include <QMetaProperty>

static bool operator==(const SourceModelRow &first,
                       const SourceModelRow &second)
{
    return first.sourceModel == second.sourceModel
            && first.sourceRow == second.sourceRow;
}

static bool operator==(const FilterModelRow &first,
                       const FilterModelRow &second)
{
    return first.filterModel == second.filterModel
            && first.filterRow == second.filterRow;
}

static bool operator==(const SourceAndFilterModelRow &first,
                       const SourceAndFilterModelRow &second)
{
    return first.sourceModelRow.sourceModel == second.sourceModelRow.sourceModel
            && first.sourceModelRow.sourceRow == second.sourceModelRow.sourceRow
            && first.filterModelRow.filterModel == second.filterModelRow.filterModel
            && first.filterModelRow.filterRow == second.filterModelRow.filterRow;
}

static uint qHash(const SourceModelRow &sourceModelRow)
{
    return qHash(sourceModelRow.sourceModel) ^ qHash(sourceModelRow.sourceRow);
}

static uint qHash(const FilterModelRow &filterModelRow)
{
    return qHash(filterModelRow.filterModel) ^ qHash(filterModelRow.filterRow);
}

static uint qHash(const SourceAndFilterModelRow &sourceAndFilterModelRow)
{
    return qHash(sourceAndFilterModelRow.sourceModelRow)
         ^ qHash(sourceAndFilterModelRow.filterModelRow);
}

CombinedSearchModel::CombinedSearchModel(QObject *parent) :
    QAbstractListModel(parent),
    m_deferLoad(false),
    m_populated(false),
    m_sourceModelRole(-1)
{
}

CombinedSearchModel::~CombinedSearchModel()
{
}

void CombinedSearchModel::classBegin()
{
    m_deferLoad = true;
}

void CombinedSearchModel::componentComplete()
{
    m_deferLoad = false;
    setModels(m_deferredModels);
    m_deferredModels.clear();
}

QList<QObject *> CombinedSearchModel::models() const
{
    QList<QObject *> rv;
    if (m_deferLoad) {
        rv.reserve(m_deferredModels.count());
        for (auto it = m_deferredModels.cbegin(), end = m_deferredModels.cend(); it != end; ++it) {
            rv.append(*it);
        }
    } else {
        rv.reserve(m_models.count());
        for (auto it = m_models.cbegin(), end = m_models.cend(); it != end; ++it) {
            rv.append(*it);
        }
    }

    return rv;
}

void CombinedSearchModel::setModels(const QList<QObject *> &models)
{
    if (m_deferLoad) {
        m_deferredModels.clear();
        for (auto it = models.cbegin(), end = models.cend(); it != end; ++it) {
            if (qobject_cast<BaseFilterModel *>(*it) != Q_NULLPTR) {
                m_deferredModels.append(*it);
            }
        }
        return;
    }

    beginResetModel();

    while (!m_models.isEmpty()) {
        BaseFilterModel *oldModel = m_models.takeLast();
        disconnect(oldModel);
    }

    m_models.clear();
    m_unpopulated.clear();

    for (auto it = models.cbegin(), end = models.cend(); it != end; ++it) {
        if (BaseFilterModel *newModel = qobject_cast<BaseFilterModel *>(*it)) {
            connect(newModel, &QAbstractListModel::columnsAboutToBeInserted, this, &CombinedSearchModel::columnsAboutToBeInserted);
            connect(newModel, &QAbstractListModel::columnsAboutToBeMoved, this, &CombinedSearchModel::columnsAboutToBeMoved);
            connect(newModel, &QAbstractListModel::columnsAboutToBeRemoved, this, &CombinedSearchModel::columnsAboutToBeRemoved);
            connect(newModel, &QAbstractListModel::columnsInserted, this, &CombinedSearchModel::columnsInserted);
            connect(newModel, &QAbstractListModel::columnsMoved, this, &CombinedSearchModel::columnsMoved);
            connect(newModel, &QAbstractListModel::columnsRemoved, this, &CombinedSearchModel::columnsRemoved);
            connect(newModel, &QAbstractListModel::dataChanged, this, &CombinedSearchModel::filterModelDataChanged);
            connect(newModel, &QAbstractListModel::headerDataChanged, this, &CombinedSearchModel::filterModelHeaderDataChanged);
            connect(newModel, &QAbstractListModel::layoutChanged, this, &CombinedSearchModel::filterModelLayoutChanged);
            connect(newModel, &QAbstractListModel::modelReset, this, &CombinedSearchModel::filterModelReset);
            connect(newModel, &QAbstractListModel::rowsAboutToBeRemoved, this, &CombinedSearchModel::filterModelRowsAboutToBeRemoved);
            connect(newModel, &QAbstractListModel::rowsRemoved, this, &CombinedSearchModel::filterModelRowsRemoved);
            connect(newModel, &QAbstractListModel::rowsInserted, this, &CombinedSearchModel::filterModelRowsInserted);
            connect(newModel, &QAbstractListModel::rowsMoved, this, &CombinedSearchModel::filterModelRowsMoved);
            connect(newModel, &BaseFilterModel::sourceModelRowsChanged, this, &CombinedSearchModel::sourceModelRowsChanged);

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

    if (!m_models.isEmpty() && m_unpopulated.isEmpty()) {
        populate();
        m_populated = true;
    }

    endResetModel();

    if (m_populated) {
        emit countChanged();
        emit populatedChanged();
    }
}

int CombinedSearchModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_rows.count();
}

QVariant CombinedSearchModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        const int row = index.row();
        return row >= m_rows.size()
             ? QVariant()
             : role == m_sourceModelRole
                    ? m_rows[row].filterModelRow.filterModel->sourceModel()->objectName()
                    : m_rows[row].filterModelRow.filterModel->getRole(
                            m_rows[row].filterModelRow.filterRow, index.column(), role);
    }

    return QVariant();
}

QHash<int, QByteArray> CombinedSearchModel::roleNames() const
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

void CombinedSearchModel::sourceModelRowsChanged()
{
    // Some underlying change in a source model.
    // We need to update our sourceModelRow values stored in
    // all of our internal data, i.e. repopulate from scratch.
    m_populated = false;
    beginResetModel();
    populate();
    endResetModel();
    m_populated = true;
}

void CombinedSearchModel::filterModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    BaseFilterModel *model = qobject_cast<BaseFilterModel *>(sender());
    if (model && !topLeft.parent().isValid() && !bottomRight.parent().isValid()) {
        for (int filterRow = topLeft.row(); filterRow <= bottomRight.row(); ++filterRow) {
            for (int i = 0; i < m_rows.size(); ++i) {
                if (m_rows[i].filterModelRow.filterModel == model
                        && m_rows[i].filterModelRow.filterRow == filterRow) {
                    const QModelIndex offsetTopLeft(index(i, topLeft.column()));
                    const QModelIndex offsetBottomRight(index(i, bottomRight.column()));
                    // TODO: contiguous ranges
                    emit dataChanged(offsetTopLeft, offsetBottomRight, roles);
                    break;
                }
            }
        }
    }
}

void CombinedSearchModel::filterModelHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    Q_UNUSED(orientation);
    Q_UNUSED(first);
    Q_UNUSED(last);
    // TODO - implement?
}

void CombinedSearchModel::filterModelLayoutChanged(const QList<QPersistentModelIndex> &, QAbstractItemModel::LayoutChangeHint)
{
    filterModelReset();
}

void CombinedSearchModel::filterModelReset()
{
    if (!m_populated) {
        return;
    }

    populate();
}

void CombinedSearchModel::filterModelRowsInserted(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid())
        return;

    BaseFilterModel *filterModel = qobject_cast<BaseFilterModel *>(sender());
    if (!filterModel)
        return;

    bool needSynchronise = false;
    QVector<SourceAndFilterModelRow> updatedRows = m_rows;
    for (int i = first; i <= last; ++i) {
        SourceModelRow smr;
        smr.sourceModel = filterModel->sourceModel();
        smr.sourceRow = filterModel->sourceRow(i);

        FilterModelRow fmr;
        fmr.filterModel = filterModel;
        fmr.filterRow = i;

        SourceAndFilterModelRow row;
        row.sourceModelRow = smr;
        row.filterModelRow = fmr;

        BaseFilterModel *foundModel = Q_NULLPTR;
        for (const SourceAndFilterModelRow &existingSmfr : updatedRows) {
            if (existingSmfr.sourceModelRow == row.sourceModelRow) {
                foundModel = existingSmfr.filterModelRow.filterModel;
                break;
            }
        }

        if (!foundModel || (m_models.indexOf(foundModel) > m_models.indexOf(filterModel))) {
            // not found, or this model is higher priority.
            injectRow(row, updatedRows, foundModel != Q_NULLPTR);
            needSynchronise = true;
        } else {
            m_duplicateRows.append(row);
        }
    }

    if (needSynchronise) {
        ::synchronizeList(this, m_rows, updatedRows);
        if (m_rows.size() != updatedRows.size()) {
            emit countChanged();
        }
    }
}

void CombinedSearchModel::filterModelRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)
{
    filterModelReset();
}

void CombinedSearchModel::filterModelRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid())
        return;

    BaseFilterModel *filterModel = qobject_cast<BaseFilterModel *>(sender());
    if (!filterModel)
        return;

    // determine which m_rows will be removed as a result of these
    // removals from the filter model.
    m_changeRows.clear();
    for (int i = first; i <= last; ++i) {
        SourceModelRow smr;
        smr.sourceModel = filterModel->sourceModel();
        smr.sourceRow = filterModel->sourceRow(i);

        FilterModelRow fmr;
        fmr.filterModel = filterModel;
        fmr.filterRow = i;

        SourceAndFilterModelRow sfmr;
        sfmr.sourceModelRow = smr;
        sfmr.filterModelRow = fmr;

        m_changeRows.insert(i, sfmr);
    }
}

void CombinedSearchModel::filterModelRowsRemoved(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid())
        return;

    BaseFilterModel *filterModel = qobject_cast<BaseFilterModel *>(sender());
    if (!filterModel)
        return;

    bool needSynchronise = false;
    QVector<SourceAndFilterModelRow> updatedRows = m_rows;
    for (int i = last; i >= first; --i) {
        const SourceAndFilterModelRow row = m_changeRows.value(i);
        exciseRow(row, updatedRows); // note: the specified row may NOT be in m_rows (it may exist only in m_duplicateRows).
        needSynchronise = true;
    }

    if (needSynchronise) {
        ::synchronizeList(this, m_rows, updatedRows);
        if (m_rows.size() != updatedRows.size()) {
            emit countChanged();
        }
    }
}

void CombinedSearchModel::filterModelPopulatedChanged()
{
    if (BaseFilterModel *model = qobject_cast<BaseFilterModel *>(sender())) {
        int index = m_unpopulated.indexOf(model);
        if (index >= 0) {
            m_unpopulated.removeAt(index);
            if (m_unpopulated.isEmpty()) {
                beginResetModel();
                populate();
                m_populated = true;
                endResetModel();
                emit populatedChanged();
                emit countChanged();
            }
        }
    }
}

void CombinedSearchModel::populate()
{
    m_filterRows.clear();
    m_filterRowRanges.clear();
    m_duplicateRows.clear();
    QVector<SourceAndFilterModelRow> changedRows;

    for (QAbstractListModel *model : m_models) {
        if (BaseFilterModel *filterModel = qobject_cast<BaseFilterModel*>(model)) {
            // first, insert an empty row range for this filter model,
            // whose start row is the current size of the de-duplicated rows.
            FilterModelRowRange emptyRowRange;
            emptyRowRange.startRow = changedRows.count();
            m_filterRowRanges.insert(filterModel, emptyRowRange);

            // second, for every datum in the filter model, determine
            // whether it's a duplicate or not, and if not, add it
            // to our data structures.
            for (int i = 0; i < filterModel->rowCount(); ++i) {
                SourceModelRow smr;
                smr.sourceModel = filterModel->sourceModel();
                smr.sourceRow = filterModel->sourceRow(i);

                FilterModelRow fmr;
                fmr.filterModel = filterModel;
                fmr.filterRow = i;

                SourceAndFilterModelRow newRow;
                newRow.sourceModelRow = smr;
                newRow.filterModelRow = fmr;

                bool found = false;
                for (const SourceAndFilterModelRow &existingRow : changedRows) {
                    if (existingRow.sourceModelRow == smr) {
                        found = true;
                        break;
                    }
                }

                if (found) {
                    m_duplicateRows.append(newRow);
                } else {
                    m_filterRows.insert(fmr, changedRows.count());
                    m_filterRowRanges[filterModel].rowCount++;
                    changedRows.append(newRow);
                }
            }
        }
    }

    if (m_populated) {
        ::synchronizeList(this, m_rows, changedRows);
    } else {
        m_rows = changedRows;
    }
    emit countChanged();
}

int CombinedSearchModel::insertRange(int index, int count, const QVector<SourceAndFilterModelRow> &source, int sourceIndex)
{
    const int end = index + count - 1;
    beginInsertRows(QModelIndex(), index, end);

    for (int i = 0; i < count; ++i) {
        m_rows.insert(index + i, source.at(sourceIndex + i));
    }

    endInsertRows();
    return end - index + 1;
}

int CombinedSearchModel::removeRange(int index, int count)
{
    const int end = index + count - 1;
    beginRemoveRows(QModelIndex(), index, end);

    for (int i = 0; i < count; ++i) {
        m_rows.removeAt(index);
    }

    endRemoveRows();
    return 0;
}

void CombinedSearchModel::injectRow(const SourceAndFilterModelRow &row, QVector<SourceAndFilterModelRow> &updatedRows, bool demoteDuplicate)
{
    BaseFilterModel * const filterModel = row.filterModelRow.filterModel;
    const int filterRow = row.filterModelRow.filterRow;

    // find the index we need to inject this row at...
    int indexForNewRow = m_filterRowRanges[filterModel].startRow;
    if (m_filterRowRanges[filterModel].rowCount > 0) {
        // search through all rows provided by this filter model
        // for the appropriate place for this row.
        while (indexForNewRow < updatedRows.count()
                && updatedRows[indexForNewRow].filterModelRow.filterModel == filterModel
                && updatedRows[indexForNewRow].filterModelRow.filterRow < filterRow) {
            indexForNewRow++;
        }
    }

    // shuffle all data structure values down to make space for the row injection.
    for (QHash<FilterModelRow, int>::iterator it = m_filterRows.begin(); it != m_filterRows.end(); ++it) {
        if (it.value() >= indexForNewRow) {
            it.value()++;
        }
    }
    for (QHash<BaseFilterModel*, FilterModelRowRange>::iterator rangeIt = m_filterRowRanges.begin();
            rangeIt != m_filterRowRanges.end(); ++rangeIt) {
        // update the start row of any filter models whose start row is after ours,
        // or any filter model whose start row is the same as ours (i.e. due to currently zero-count)
        // but which should sort after us.
        if (rangeIt.key() != filterModel
                && (rangeIt.value().startRow > indexForNewRow
                        || (rangeIt.value().startRow == indexForNewRow
                            && m_models.indexOf(rangeIt.key()) > m_models.indexOf(filterModel)))) {
            rangeIt.value().startRow++;
        }
    }

    // inject the row at the appropriate place
    m_filterRows.insert(row.filterModelRow, indexForNewRow);
    m_filterRowRanges[filterModel].rowCount++;
    if (indexForNewRow < updatedRows.size()) {
        updatedRows.insert(indexForNewRow, row);
        if (demoteDuplicate) {
            // search subsequent rows for duplicate, and demote.
            for (int i = indexForNewRow+1; i < updatedRows.size(); ++i) {
                if (updatedRows[i].sourceModelRow.sourceModel == row.sourceModelRow.sourceModel
                        && updatedRows[i].sourceModelRow.sourceRow == row.sourceModelRow.sourceRow) {
                    const SourceAndFilterModelRow demoteRow = updatedRows[i];
                    exciseRow(demoteRow, updatedRows, i);
                    m_duplicateRows.append(demoteRow);
                    break;
                }
            }
        }
    } else if (indexForNewRow == updatedRows.size()) {
        updatedRows.append(row);
        if (demoteDuplicate) {
            qWarning() << "Error: no duplicate to demote!";
        }
    } else {
        qWarning() << "Error: invalid insertion index!";
        updatedRows.append(row);
    }
}

void CombinedSearchModel::exciseRow(const SourceAndFilterModelRow &row, QVector<SourceAndFilterModelRow> &updatedRows, int beginSearchIndex)
{
    BaseFilterModel * const filterModel = row.filterModelRow.filterModel;
    const int filterRow = row.filterModelRow.filterRow;
    const bool promoteDuplicate = beginSearchIndex == 0;
    const bool exciseDuplicate = !promoteDuplicate;

    for (int j = beginSearchIndex; j < updatedRows.count(); ++j) {
        const SourceAndFilterModelRow existingRow = updatedRows[j];
        if (existingRow.sourceModelRow.sourceModel == row.sourceModelRow.sourceModel
                && existingRow.sourceModelRow.sourceRow == row.sourceModelRow.sourceRow) {
            if (existingRow.filterModelRow.filterModel == row.filterModelRow.filterModel
                    && existingRow.filterModelRow.filterRow == row.filterModelRow.filterRow) {
                // the source row is provided by this model.
                // remove the row from m_rows.
                updatedRows.removeAt(j);

                // update subsequent m_rows and m_duplicateRows to fixup filterModel indexes.
                // only do this if we're not excising a newly-duplicate
                // (as in that case, the underlying row isn't being removed
                // from the filter model at all, we're just demoting it ourselves).
                if (!exciseDuplicate) {
                    for (int k = j; k < updatedRows.count(); ++k) {
                        SourceAndFilterModelRow &nextRow(updatedRows[k]);
                        if ((nextRow.filterModelRow.filterModel == filterModel)
                                && (nextRow.filterModelRow.filterRow > filterRow)) {
                            nextRow.filterModelRow.filterRow--;
                        }
                    }
                    for (int k = 0; k < m_duplicateRows.count(); ++k) {
                        SourceAndFilterModelRow &dupRow(m_duplicateRows[k]);
                        if ((dupRow.filterModelRow.filterModel == filterModel)
                                && (dupRow.filterModelRow.filterRow > filterRow)) {
                            dupRow.filterModelRow.filterRow--;
                        }
                    }
                }

                // update all m_filterRows to fixup indexes.
                QHash<FilterModelRow, int> updatedFilterRows;
                updatedFilterRows.reserve(m_filterRows.size());
                for (QHash<FilterModelRow, int>::const_iterator filterRowIt = m_filterRows.begin();
                        filterRowIt != m_filterRows.end(); ++filterRowIt) {
                    FilterModelRow currFilterRow = filterRowIt.key();
                    int currRowIndex = filterRowIt.value();
                    if (filterRowIt.key().filterModel == filterModel) {
                        if (filterRowIt.key().filterRow == filterRow) {
                            Q_ASSERT(filterRowIt.value() == j); /* j is the row in m_rows */
                            // this row has been deleted.
                            // don't include it in updated filter rows.
                            continue;
                        } else if (filterRowIt.key().filterRow > filterRow) {
                            currFilterRow.filterRow--;
                        }
                    }

                    if (filterRowIt.value() > j) {
                        currRowIndex--;
                    }

                    updatedFilterRows.insert(currFilterRow, currRowIndex);
                }
                m_filterRows = updatedFilterRows;

                // update m_filterRowRanges to fixup count of rows provided to m_rows by this filterModel
                // and also to modify the startRow for all other models if required.
                m_filterRowRanges[filterModel].rowCount--;
                for (QHash<BaseFilterModel*, FilterModelRowRange>::iterator rangeIt = m_filterRowRanges.begin();
                        rangeIt != m_filterRowRanges.end(); ++rangeIt) {
                    if (rangeIt.key() != filterModel && rangeIt.value().startRow > j) {
                        rangeIt.value().startRow--;
                    }
                }

                if (promoteDuplicate) {
                    // if other models provide this row, we need to reinstate/promote the duplicate row
                    // from the highest-priority model at the appropriate place.
                    FilterModelRow duplicateIndex;
                    for (int k = 0; k < m_duplicateRows.size(); ++k) {
                        const SourceAndFilterModelRow &dupRow(m_duplicateRows[k]);
                        if (dupRow.sourceModelRow.sourceModel == row.sourceModelRow.sourceModel
                                && dupRow.sourceModelRow.sourceRow == row.sourceModelRow.sourceRow
                                && (duplicateIndex.filterModel == Q_NULLPTR
                                    || m_models.indexOf(dupRow.filterModelRow.filterModel) < m_models.indexOf(duplicateIndex.filterModel))) {
                            duplicateIndex.filterModel = dupRow.filterModelRow.filterModel;
                            duplicateIndex.filterRow = k; // not a filter model row, but index into m_duplicateRows.
                        }
                    }

                    if (duplicateIndex.filterModel) {
                        // found the highest priority duplicate.  reinstate it.
                        const SourceAndFilterModelRow promoteRow = m_duplicateRows.takeAt(duplicateIndex.filterRow);
                        injectRow(promoteRow, updatedRows, false);
                    }
                }
            } else {
                // the row was provided first by another higher priority model.
                // so the row from this model must be a duplicate.
                // find and remove that duplicate.
                // Use naive for-loop since indexOf returns invalid index...
                int dupIdx = m_duplicateRows.size() - 1;
                for ( ; dupIdx >= 0; --dupIdx) {
                    const SourceAndFilterModelRow &dupRow(m_duplicateRows[dupIdx]);
                    if (dupRow.sourceModelRow.sourceModel == row.sourceModelRow.sourceModel
                            && dupRow.sourceModelRow.sourceRow == row.sourceModelRow.sourceRow
                            && dupRow.filterModelRow.filterModel == row.filterModelRow.filterModel
                            && dupRow.filterModelRow.filterRow == row.filterModelRow.filterRow) {
                        // found the duplicate row we want to remove.
                        break;
                    }
                }

                if (dupIdx >= 0) {
                    m_duplicateRows.remove(dupIdx);
                } else {
                    qWarning() << "Error: unable to find duplicate!";
                }

                // and update model indexes as this row has been removed from the underlying model.
                for (int k = 0; k < updatedRows.count(); ++k) {
                    SourceAndFilterModelRow &nextRow(updatedRows[k]);
                    if ((nextRow.filterModelRow.filterModel == filterModel)
                            && (nextRow.filterModelRow.filterRow > filterRow)) {
                        nextRow.filterModelRow.filterRow--;
                    }
                }
                for (int k = 0; k < m_duplicateRows.count(); ++k) {
                    SourceAndFilterModelRow &dupRow(m_duplicateRows[k]);
                    if ((dupRow.filterModelRow.filterModel == filterModel)
                            && (dupRow.filterModelRow.filterRow > filterRow)) {
                        dupRow.filterModelRow.filterRow--;
                    }
                }
            }
            break;
        }
    }
}

