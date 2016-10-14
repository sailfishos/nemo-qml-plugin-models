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

#include <QtTest/QtTest>
#include <QObject>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>

class tst_ObjectListModel : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void testPopulation();
    void testInsertion();
    void testRemoval();
    void testBatchRemoval();
    void testMove();
    void testChanged();
    void testSynchronization();
    void testAutomaticRoles();
    void testSimpleExport();
    void testRoleMap();
    void testUpdate();
};

void tst_ObjectListModel::init()
{
}

void tst_ObjectListModel::cleanup()
{
}

QObject *makeObject(const QString &name)
{
    QObject *rv(new QObject);
    rv->setProperty("name", name);
    return rv;
}

QString objectName(const QObject *object)
{
    return object->property("name").value<QString>();
}

void tst_ObjectListModel::testPopulation()
{
    ObjectListModel emptyModel;
    QCOMPARE(emptyModel.populated(), true);
    QCOMPARE(emptyModel.count(), 0);

    ObjectListModel model(0, false, false);
    QCOMPARE(model.populated(), false);
    QCOMPARE(model.count(), 0);

    model.appendItem(makeObject("a"));
    model.appendItem(makeObject("b"));
    model.appendItem(makeObject("c"));
    model.appendItem(makeObject("d"));
    model.appendItem(makeObject("e"));

    QCOMPARE(model.populated(), false);
    QCOMPARE(model.count(), 5);

    model.setPopulated(true);
    QCOMPARE(model.populated(), true);
    QCOMPARE(model.count(), 5);

    model.deleteAll();
    QCOMPARE(model.populated(), false);
    QCOMPARE(model.count(), 0);
}

void tst_ObjectListModel::testInsertion()
{
    QList<QObject *> objects;
    objects.append(makeObject("a"));
    objects.append(makeObject("b"));
    objects.append(makeObject("c"));
    objects.append(makeObject("d"));
    objects.append(makeObject("e"));

    ObjectListModel model;

    QCOMPARE(model.count(), 0);
    QCOMPARE(model.populated(), true);

    QSignalSpy addedSpy(&model, SIGNAL(itemAdded(QObject*)));
    QSignalSpy removedSpy(&model, SIGNAL(itemRemoved(QObject*)));
    QSignalSpy countSpy(&model, SIGNAL(countChanged()));

    model.appendItem(objects.at(0));
    model.appendItems(QList<QObject *>() << objects.at(1) << objects.at(2));

    QCOMPARE(model.count(), 3);
    QCOMPARE(::objectName(model.get(0)), QString("a"));
    QCOMPARE(::objectName(model.get(1)), QString("b"));
    QCOMPARE(::objectName(model.get(2)), QString("c"));

    QCOMPARE(addedSpy.count(), 3);
    QCOMPARE(addedSpy.at(0), QVariantList() << QVariant::fromValue(objects.at(0)));
    QCOMPARE(addedSpy.at(1), QVariantList() << QVariant::fromValue(objects.at(1)));
    QCOMPARE(addedSpy.at(2), QVariantList() << QVariant::fromValue(objects.at(2)));
    QCOMPARE(removedSpy.count(), 0);
    QCOMPARE(countSpy.count(), 2);

    addedSpy.clear();
    countSpy.clear();

    model.insertItem(1, objects.at(3));
    model.insertItem(0, objects.at(4));

    QCOMPARE(addedSpy.count(), 2);
    QCOMPARE(addedSpy.at(0), QVariantList() << QVariant::fromValue(objects.at(3)));
    QCOMPARE(addedSpy.at(1), QVariantList() << QVariant::fromValue(objects.at(4)));
    QCOMPARE(removedSpy.count(), 0);
    QCOMPARE(countSpy.count(), 2);

    QCOMPARE(::objectName(model.get(0)), QString("e"));
    QCOMPARE(::objectName(model.get(1)), QString("a"));
    QCOMPARE(::objectName(model.get(2)), QString("d"));
    QCOMPARE(::objectName(model.get(3)), QString("b"));
    QCOMPARE(::objectName(model.get(4)), QString("c"));

    model.clear();

    QCOMPARE(model.count(), 0);
    QCOMPARE(model.populated(), true);

    QCOMPARE(removedSpy.count(), 5);
    QCOMPARE(countSpy.count(), 3);

    qDeleteAll(objects);
}

void tst_ObjectListModel::testRemoval()
{
    QList<QObject *> objects;
    objects.append(makeObject("a"));
    objects.append(makeObject("b"));
    objects.append(makeObject("c"));
    objects.append(makeObject("d"));
    objects.append(makeObject("e"));

    ObjectListModel model;
    model.appendItems(objects);

    QCOMPARE(model.count(), 5);
    QCOMPARE(::objectName(model.get(0)), QString("a"));
    QCOMPARE(::objectName(model.get(1)), QString("b"));
    QCOMPARE(::objectName(model.get(2)), QString("c"));
    QCOMPARE(::objectName(model.get(3)), QString("d"));
    QCOMPARE(::objectName(model.get(4)), QString("e"));

    QSignalSpy addedSpy(&model, SIGNAL(itemAdded(QObject*)));
    QSignalSpy removedSpy(&model, SIGNAL(itemRemoved(QObject*)));
    QSignalSpy countSpy(&model, SIGNAL(countChanged()));
    QSignalSpy rowsRemovedSpy(&model, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    model.removeItem(objects.at(0));
    model.removeItem(objects.at(2));
    model.removeItem(objects.at(4));

    QCOMPARE(model.count(), 2);
    QCOMPARE(::objectName(model.get(0)), QString("b"));
    QCOMPARE(::objectName(model.get(1)), QString("d"));

    QCOMPARE(addedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 3);
    QCOMPARE(removedSpy.at(0), QVariantList() << QVariant::fromValue(objects.at(0)));
    QCOMPARE(removedSpy.at(1), QVariantList() << QVariant::fromValue(objects.at(2)));
    QCOMPARE(removedSpy.at(2), QVariantList() << QVariant::fromValue(objects.at(4)));
    QCOMPARE(countSpy.count(), 3);

    QCOMPARE(rowsRemovedSpy.count(), 3);
    QCOMPARE(qvariant_cast<QModelIndex>(rowsRemovedSpy.at(0).at(0)), QModelIndex());
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(0).at(1)), 0);
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(0).at(2)), 0);
    QCOMPARE(qvariant_cast<QModelIndex>(rowsRemovedSpy.at(1).at(0)), QModelIndex());
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(1).at(1)), 1);
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(1).at(2)), 1);
    QCOMPARE(qvariant_cast<QModelIndex>(rowsRemovedSpy.at(2).at(0)), QModelIndex());
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(2).at(1)), 2);
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(2).at(2)), 2);

    removedSpy.clear();
    countSpy.clear();
    rowsRemovedSpy.clear();

    model.removeItemAt(1);
    model.removeItemAt(0);
    
    QCOMPARE(model.count(), 0);

    QCOMPARE(addedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 2);
    QCOMPARE(removedSpy.at(0), QVariantList() << QVariant::fromValue(objects.at(3)));
    QCOMPARE(removedSpy.at(1), QVariantList() << QVariant::fromValue(objects.at(1)));
    QCOMPARE(countSpy.count(), 2);

    QCOMPARE(rowsRemovedSpy.count(), 2);
    QCOMPARE(qvariant_cast<QModelIndex>(rowsRemovedSpy.at(0).at(0)), QModelIndex());
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(0).at(1)), 1);
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(0).at(2)), 1);
    QCOMPARE(qvariant_cast<QModelIndex>(rowsRemovedSpy.at(1).at(0)), QModelIndex());
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(1).at(1)), 0);
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(1).at(2)), 0);

    qDeleteAll(objects);
}

void tst_ObjectListModel::testBatchRemoval()
{
    QList<QObject *> objects;
    objects.append(makeObject("a"));
    objects.append(makeObject("b"));
    objects.append(makeObject("c"));
    objects.append(makeObject("d"));
    objects.append(makeObject("e"));
    objects.append(makeObject("f"));
    objects.append(makeObject("g"));
    objects.append(makeObject("h"));
    objects.append(makeObject("i"));

    ObjectListModel model;
    model.appendItems(objects);

    QCOMPARE(model.count(), 9);
    QCOMPARE(::objectName(model.get(0)), QString("a"));
    QCOMPARE(::objectName(model.get(1)), QString("b"));
    QCOMPARE(::objectName(model.get(2)), QString("c"));
    QCOMPARE(::objectName(model.get(3)), QString("d"));
    QCOMPARE(::objectName(model.get(4)), QString("e"));
    QCOMPARE(::objectName(model.get(5)), QString("f"));
    QCOMPARE(::objectName(model.get(6)), QString("g"));
    QCOMPARE(::objectName(model.get(7)), QString("h"));
    QCOMPARE(::objectName(model.get(8)), QString("i"));

    QSignalSpy addedSpy(&model, SIGNAL(itemAdded(QObject*)));
    QSignalSpy removedSpy(&model, SIGNAL(itemRemoved(QObject*)));
    QSignalSpy countSpy(&model, SIGNAL(countChanged()));
    QSignalSpy rowsRemovedSpy(&model, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    QList<QObject *> removals;
    removals.append(objects.at(0));
    removals.append(objects.at(8));
    removals.append(objects.at(7));
    removals.append(objects.at(3));
    removals.append(objects.at(2));
    removals.append(objects.at(5));
    model.removeItems(removals);

    QCOMPARE(model.count(), 3);
    QCOMPARE(::objectName(model.get(0)), QString("b"));
    QCOMPARE(::objectName(model.get(1)), QString("e"));
    QCOMPARE(::objectName(model.get(2)), QString("g"));

    QCOMPARE(addedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 6);
    QCOMPARE(removedSpy.at(0), QVariantList() << QVariant::fromValue(objects.at(0)));
    QCOMPARE(removedSpy.at(1), QVariantList() << QVariant::fromValue(objects.at(2)));
    QCOMPARE(removedSpy.at(2), QVariantList() << QVariant::fromValue(objects.at(3)));
    QCOMPARE(removedSpy.at(3), QVariantList() << QVariant::fromValue(objects.at(5)));
    QCOMPARE(removedSpy.at(4), QVariantList() << QVariant::fromValue(objects.at(7)));
    QCOMPARE(removedSpy.at(5), QVariantList() << QVariant::fromValue(objects.at(8)));
    QCOMPARE(countSpy.count(), 1);

    QCOMPARE(rowsRemovedSpy.count(), 4);
    QCOMPARE(qvariant_cast<QModelIndex>(rowsRemovedSpy.at(0).at(0)), QModelIndex());
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(0).at(1)), 7);
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(0).at(2)), 8);
    QCOMPARE(qvariant_cast<QModelIndex>(rowsRemovedSpy.at(1).at(0)), QModelIndex());
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(1).at(1)), 5);
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(1).at(2)), 5);
    QCOMPARE(qvariant_cast<QModelIndex>(rowsRemovedSpy.at(2).at(0)), QModelIndex());
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(2).at(1)), 2);
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(2).at(2)), 3);
    QCOMPARE(qvariant_cast<QModelIndex>(rowsRemovedSpy.at(3).at(0)), QModelIndex());
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(3).at(1)), 0);
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(3).at(2)), 0);

    removedSpy.clear();
    countSpy.clear();
    rowsRemovedSpy.clear();

    removals.clear();
    removals.append(objects.at(1));
    removals.append(objects.at(6));
    removals.append(objects.at(4));
    model.removeItems(removals);

    QCOMPARE(model.count(), 0);

    QCOMPARE(addedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 3);
    QCOMPARE(removedSpy.at(0), QVariantList() << QVariant::fromValue(objects.at(1)));
    QCOMPARE(removedSpy.at(1), QVariantList() << QVariant::fromValue(objects.at(4)));
    QCOMPARE(removedSpy.at(2), QVariantList() << QVariant::fromValue(objects.at(6)));
    QCOMPARE(countSpy.count(), 1);

    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(qvariant_cast<QModelIndex>(rowsRemovedSpy.at(0).at(0)), QModelIndex());
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(0).at(1)), 0);
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(0).at(2)), 2);

    qDeleteAll(objects);
}

void tst_ObjectListModel::testMove()
{
    QList<QObject *> objects;
    objects.append(makeObject("a"));
    objects.append(makeObject("b"));
    objects.append(makeObject("c"));
    objects.append(makeObject("d"));
    objects.append(makeObject("e"));

    ObjectListModel model;
    model.appendItems(objects);

    QCOMPARE(model.count(), 5);
    QCOMPARE(::objectName(model.get(0)), QString("a"));
    QCOMPARE(::objectName(model.get(1)), QString("b"));
    QCOMPARE(::objectName(model.get(2)), QString("c"));
    QCOMPARE(::objectName(model.get(3)), QString("d"));
    QCOMPARE(::objectName(model.get(4)), QString("e"));

    QSignalSpy addedSpy(&model, SIGNAL(itemAdded(QObject*)));
    QSignalSpy removedSpy(&model, SIGNAL(itemRemoved(QObject*)));
    QSignalSpy countSpy(&model, SIGNAL(countChanged()));
    QSignalSpy movedSpy(&model, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)));

    model.moveItem(0, 2);
    model.moveItem(4, 3);

    QCOMPARE(model.count(), 5);
    QCOMPARE(::objectName(model.get(0)), QString("b"));
    QCOMPARE(::objectName(model.get(1)), QString("c"));
    QCOMPARE(::objectName(model.get(2)), QString("a"));
    QCOMPARE(::objectName(model.get(3)), QString("e"));
    QCOMPARE(::objectName(model.get(4)), QString("d"));

    QCOMPARE(addedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 0);
    QCOMPARE(countSpy.count(), 0);
    QCOMPARE(movedSpy.count(), 2);
    QCOMPARE(movedSpy.at(0), QVariantList() << QModelIndex() << 0 << 0 << QModelIndex() << 3);
    QCOMPARE(movedSpy.at(1), QVariantList() << QModelIndex() << 4 << 4 << QModelIndex() << 3);

    qDeleteAll(objects);
}

void tst_ObjectListModel::testChanged()
{
    QList<QObject *> objects;
    objects.append(makeObject("a"));
    objects.append(makeObject("b"));
    objects.append(makeObject("c"));
    objects.append(makeObject("d"));
    objects.append(makeObject("e"));

    ObjectListModel model;
    model.appendItems(objects);

    QCOMPARE(model.count(), 5);
    QCOMPARE(::objectName(model.get(0)), QString("a"));
    QCOMPARE(::objectName(model.get(1)), QString("b"));
    QCOMPARE(::objectName(model.get(2)), QString("c"));
    QCOMPARE(::objectName(model.get(3)), QString("d"));
    QCOMPARE(::objectName(model.get(4)), QString("e"));

    QSignalSpy addedSpy(&model, SIGNAL(itemAdded(QObject*)));
    QSignalSpy removedSpy(&model, SIGNAL(itemRemoved(QObject*)));
    QSignalSpy countSpy(&model, SIGNAL(countChanged()));
    QSignalSpy changedSpy(&model, SIGNAL(dataChanged(QModelIndex,QModelIndex)));

    model.itemChangedAt(1);

    QCOMPARE(addedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 0);
    QCOMPARE(countSpy.count(), 0);
    QCOMPARE(changedSpy.count(), 1);
    QVariantList args = changedSpy.takeFirst();
    QCOMPARE(args.count(), 2);
    QModelIndex topLeft(qvariant_cast<QModelIndex>(args.at(0)));
    QCOMPARE(topLeft.parent(), QModelIndex());
    QCOMPARE(topLeft.row(), 1);
    QCOMPARE(topLeft.column(), 0);
    QModelIndex bottomRight(qvariant_cast<QModelIndex>(args.at(1)));
    QCOMPARE(bottomRight.parent(), QModelIndex());
    QCOMPARE(bottomRight.row(), 1);
    QCOMPARE(bottomRight.column(), 0);

    qDeleteAll(objects);
}

void tst_ObjectListModel::testSynchronization()
{
    QList<QObject *> objects;
    objects.append(makeObject("a"));
    objects.append(makeObject("b"));
    objects.append(makeObject("c"));
    objects.append(makeObject("d"));
    objects.append(makeObject("e"));

    ObjectListModel model;
    model.appendItem(objects.at(0));
    model.appendItem(objects.at(2));
    model.appendItem(objects.at(4));

    QCOMPARE(model.count(), 3);
    QCOMPARE(::objectName(model.get(0)), QString("a"));
    QCOMPARE(::objectName(model.get(1)), QString("c"));
    QCOMPARE(::objectName(model.get(2)), QString("e"));

    QSignalSpy addedSpy(&model, SIGNAL(itemAdded(QObject*)));
    QSignalSpy removedSpy(&model, SIGNAL(itemRemoved(QObject*)));
    QSignalSpy countSpy(&model, SIGNAL(countChanged()));
    QSignalSpy movedSpy(&model, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)));
    QSignalSpy rowsInsertedSpy(&model, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QSignalSpy rowsRemovedSpy(&model, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    model.synchronizeList(objects);

    QCOMPARE(model.count(), 5);
    QCOMPARE(::objectName(model.get(0)), QString("a"));
    QCOMPARE(::objectName(model.get(1)), QString("b"));
    QCOMPARE(::objectName(model.get(2)), QString("c"));
    QCOMPARE(::objectName(model.get(3)), QString("d"));
    QCOMPARE(::objectName(model.get(4)), QString("e"));

    QCOMPARE(addedSpy.count(), 2);
    QCOMPARE(addedSpy.at(0), QVariantList() << QVariant::fromValue(objects.at(1)));
    QCOMPARE(addedSpy.at(1), QVariantList() << QVariant::fromValue(objects.at(3)));
    QCOMPARE(removedSpy.count(), 0);
    QCOMPARE(countSpy.count(), 1);
    QCOMPARE(movedSpy.count(), 0);
    QCOMPARE(rowsInsertedSpy.count(), 2);
    QCOMPARE(qvariant_cast<QModelIndex>(rowsInsertedSpy.at(0).at(0)), QModelIndex());
    QCOMPARE(qvariant_cast<int>(rowsInsertedSpy.at(0).at(1)), 1);
    QCOMPARE(qvariant_cast<int>(rowsInsertedSpy.at(0).at(2)), 1);
    QCOMPARE(qvariant_cast<QModelIndex>(rowsInsertedSpy.at(1).at(0)), QModelIndex());
    QCOMPARE(qvariant_cast<int>(rowsInsertedSpy.at(1).at(1)), 3);
    QCOMPARE(qvariant_cast<int>(rowsInsertedSpy.at(1).at(2)), 3);
    QCOMPARE(rowsRemovedSpy.count(), 0);

    addedSpy.clear();
    countSpy.clear();
    rowsInsertedSpy.clear();

    model.synchronizeList(QList<QObject *>() << objects.at(0) << objects.at(2));
    QCOMPARE(model.count(), 2);
    QCOMPARE(::objectName(model.get(0)), QString("a"));
    QCOMPARE(::objectName(model.get(1)), QString("c"));

    QCOMPARE(addedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 3);
    QCOMPARE(removedSpy.at(0), QVariantList() << QVariant::fromValue(objects.at(1)));
    QCOMPARE(removedSpy.at(1), QVariantList() << QVariant::fromValue(objects.at(3)));
    QCOMPARE(removedSpy.at(2), QVariantList() << QVariant::fromValue(objects.at(4)));
    QCOMPARE(countSpy.count(), 1);
    QCOMPARE(movedSpy.count(), 0);
    QCOMPARE(rowsInsertedSpy.count(), 0);
    QCOMPARE(rowsRemovedSpy.count(), 2);
    QCOMPARE(qvariant_cast<QModelIndex>(rowsRemovedSpy.at(0).at(0)), QModelIndex());
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(0).at(1)), 1);
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(0).at(2)), 1);
    QCOMPARE(qvariant_cast<QModelIndex>(rowsRemovedSpy.at(1).at(0)), QModelIndex());
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(1).at(1)), 2);
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(1).at(2)), 3);

    removedSpy.clear();
    countSpy.clear();
    rowsRemovedSpy.clear();

    model.synchronizeList(QList<QObject *>() << objects.at(2) << objects.at(0));
    QCOMPARE(model.count(), 2);
    QCOMPARE(::objectName(model.get(0)), QString("c"));
    QCOMPARE(::objectName(model.get(1)), QString("a"));

    QCOMPARE(addedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 0);
    QCOMPARE(countSpy.count(), 0);
    QCOMPARE(movedSpy.count(), 0);
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QCOMPARE(qvariant_cast<QModelIndex>(rowsInsertedSpy.at(0).at(0)), QModelIndex());
    QCOMPARE(qvariant_cast<int>(rowsInsertedSpy.at(0).at(1)), 0);
    QCOMPARE(qvariant_cast<int>(rowsInsertedSpy.at(0).at(2)), 0);
    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(qvariant_cast<QModelIndex>(rowsRemovedSpy.at(0).at(0)), QModelIndex());
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(0).at(1)), 2);
    QCOMPARE(qvariant_cast<int>(rowsRemovedSpy.at(0).at(2)), 2);

    qDeleteAll(objects);
}

void tst_ObjectListModel::testAutomaticRoles()
{
    QObject objA;
    objA.setObjectName(QString("objA"));
    objA.setProperty("family", QVariant::fromValue(QString("Istiophoridae")));
    objA.setProperty("genus", QVariant::fromValue(QString("Istiophorus Lacépède")));
    objA.setProperty("species", QVariant::fromValue(QString("Istiophorus albicans")));

    ObjectListModel model(0, true, true);

    QCOMPARE(model.automaticRoles(), true);
    QCOMPARE(model.populated(), true);
    QCOMPARE(model.count(), 0);
    QCOMPARE(model.roleNames().count(), 2);
    QCOMPARE(model.roleNames().values().contains(QByteArray("object")), true);
    QCOMPARE(model.roleNames().values().contains(QByteArray("roles")), true);

    model.appendItem(&objA);
    QCOMPARE(model.populated(), true);
    QCOMPARE(model.count(), 1);

    const QHash<int, QByteArray> roles(model.roleNames());
    QCOMPARE(roles.count(), 6);
    QCOMPARE(roles.values().contains(QByteArray("object")), true);
    QCOMPARE(roles.values().contains(QByteArray("roles")), true);
    QCOMPARE(roles.values().contains(QByteArray("objectName")), true);
    QCOMPARE(roles.values().contains(QByteArray("family")), true);
    QCOMPARE(roles.values().contains(QByteArray("genus")), true);
    QCOMPARE(roles.values().contains(QByteArray("species")), true);

    const QModelIndex firstIndex(model.index(0, 0));
    QCOMPARE(model.data(firstIndex, roles.key(QByteArray("objectName"))), QVariant::fromValue(QString("objA")));
    QCOMPARE(model.data(firstIndex, roles.key(QByteArray("family"))), QVariant::fromValue(QString("Istiophoridae")));
    QCOMPARE(model.data(firstIndex, roles.key(QByteArray("genus"))), QVariant::fromValue(QString("Istiophorus Lacépède")));
    QCOMPARE(model.data(firstIndex, roles.key(QByteArray("species"))), QVariant::fromValue(QString("Istiophorus albicans")));

    QObject objB;
    objB.setObjectName(QString("objB"));
    objB.setProperty("family", QVariant::fromValue(QString("Chlamyphoridae")));
    objB.setProperty("genus", QVariant::fromValue(QString("Chlamyphorus Harlan")));
    objB.setProperty("species", QVariant::fromValue(QString("Chlamyphorus truncatus")));

    model.appendItem(&objB);
    QCOMPARE(model.populated(), true);
    QCOMPARE(model.count(), 2);
    QCOMPARE(roles.count(), 6);

    const QModelIndex secondIndex(model.index(1, 0));
    QCOMPARE(model.data(secondIndex, roles.key(QByteArray("objectName"))), QVariant::fromValue(QString("objB")));
    QCOMPARE(model.data(secondIndex, roles.key(QByteArray("family"))), QVariant::fromValue(QString("Chlamyphoridae")));
    QCOMPARE(model.data(secondIndex, roles.key(QByteArray("genus"))), QVariant::fromValue(QString("Chlamyphorus Harlan")));
    QCOMPARE(model.data(secondIndex, roles.key(QByteArray("species"))), QVariant::fromValue(QString("Chlamyphorus truncatus")));
}

class TestObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString family READ family CONSTANT)
    Q_PROPERTY(QString genus READ genus CONSTANT)
    Q_PROPERTY(QString species READ species CONSTANT)
    Q_PROPERTY(QString commonName READ commonName WRITE setCommonName NOTIFY commonNameChanged)

public:
    TestObject(const QString &family, const QString &genus, const QString &species)
        : QObject(0)
        , m_family(family)
        , m_genus(genus)
        , m_species(species)
    {
    }

    QString family() const { return m_family; }
    QString genus() const { return m_genus; }
    QString species() const { return m_species; }

    QString commonName() const { return m_common; }
    void setCommonName(const QString &name) {
        if (m_common != name) {
            m_common = name;
            emit commonNameChanged();
        }
    }

signals:
    void commonNameChanged();

private:
    QString m_family;
    QString m_genus;
    QString m_species;
    QString m_common;
};

void tst_ObjectListModel::testSimpleExport()
{
    ObjectListModel model(0, true, true);

    model.appendItem(new TestObject(QString("Istiophoridae"), QString("Istiophorus Lacépède"), QString("Istiophorus albicans")));
    model.appendItem(new TestObject(QString("Chlamyphoridae"), QString("Chlamyphorus Harlan"), QString("Chlamyphorus truncatus")));

    QQmlEngine engine;
    engine.rootContext()->setContextProperty("objectListModel", &model);

    QQmlComponent component(&engine);
    component.setData("\
import QtQuick 2.0\n\
import QtQml 2.2\n\
Item {\n\
    id: root\n\
    property var result: []\n\
    Instantiator {\n\
        model: objectListModel\n\
        delegate: QtObject {\n\
            Component.onCompleted: root.result.push(model.family + ',' + model.genus + ',' + model.species)\n\
        }\n\
    }\n\
}", QUrl());

    QScopedPointer<QObject> object(component.create());
    QVERIFY(object != 0);

    const QVariantList result(object->property("result").value<QVariantList>());
    QCOMPARE(result.count(), 2);
    QCOMPARE(result.at(0).toString(), QString("Istiophoridae,Istiophorus Lacépède,Istiophorus albicans"));
    QCOMPARE(result.at(1).toString(), QString("Chlamyphoridae,Chlamyphorus Harlan,Chlamyphorus truncatus"));

    model.deleteAll();
}

void tst_ObjectListModel::testRoleMap()
{
    ObjectListModel model(0, true, true);

    model.appendItem(new TestObject(QString("Istiophoridae"), QString("Istiophorus Lacépède"), QString("Istiophorus albicans")));
    model.appendItem(new TestObject(QString("Chlamyphoridae"), QString("Chlamyphorus Harlan"), QString("Chlamyphorus truncatus")));

    QVariantMap roleMap(model.itemRoles(model.get(0)));
    QCOMPARE(roleMap.keys().count(), 5);
    QCOMPARE(roleMap.value("family").toString(), QString("Istiophoridae"));
    QCOMPARE(roleMap.value("genus").toString(), QString("Istiophorus Lacépède"));
    QCOMPARE(roleMap.value("species").toString(), QString("Istiophorus albicans"));
    QCOMPARE(roleMap.value("commonName").toString(), QString());
    QCOMPARE(roleMap.value("objectName").toString(), QString());

    roleMap = model.itemRoles(model.get(1));
    QCOMPARE(roleMap.keys().count(), 5);
    QCOMPARE(roleMap.value("family").toString(), QString("Chlamyphoridae"));
    QCOMPARE(roleMap.value("genus").toString(), QString("Chlamyphorus Harlan"));
    QCOMPARE(roleMap.value("species").toString(), QString("Chlamyphorus truncatus"));
    QCOMPARE(roleMap.value("commonName").toString(), QString());
    QCOMPARE(roleMap.value("objectName").toString(), QString());

    model.deleteAll();
}

void tst_ObjectListModel::testUpdate()
{
    ObjectListModel model(0, true, true);

    TestObject to(QString("Istiophoridae"), QString("Istiophorus Lacépède"), QString("Istiophorus albicans"));
    to.setCommonName("Sailfish");
    to.setProperty("dynamic", QVariant::fromValue(2.0));

    model.appendItem(&to);

    QVariantMap roleMap(model.itemRoles(model.get(0)));
    QCOMPARE(roleMap.keys().count(), 6);
    QCOMPARE(roleMap.value("family").toString(), QString("Istiophoridae"));
    QCOMPARE(roleMap.value("genus").toString(), QString("Istiophorus Lacépède"));
    QCOMPARE(roleMap.value("species").toString(), QString("Istiophorus albicans"));
    QCOMPARE(roleMap.value("commonName").toString(), QString("Sailfish"));
    QCOMPARE(roleMap.value("dynamic").toDouble(), 2.0);
    QCOMPARE(roleMap.value("objectName").toString(), QString());

    QVariantMap newValues;
    newValues.insert(QString("family"), QVariant::fromValue(QString("Chlamyphoridae"))); // Not writable
    newValues.insert(QString("commonName"), QVariant::fromValue(QString("Shellfish")));
    newValues.insert(QString("dynamic"), QVariant::fromValue(3.0));
    newValues.insert(QString("objectName"), QVariant::fromValue(QString("Updated")));
    newValues.insert(QString("irrelevant"), QVariant::fromValue(QString("Unused")));
    model.updateItem(model.get(0), newValues);

    roleMap = model.itemRoles(model.get(0));
    QCOMPARE(roleMap.keys().count(), 6);
    QCOMPARE(roleMap.value("family").toString(), QString("Istiophoridae"));
    QCOMPARE(roleMap.value("genus").toString(), QString("Istiophorus Lacépède"));
    QCOMPARE(roleMap.value("species").toString(), QString("Istiophorus albicans"));
    QCOMPARE(roleMap.value("commonName").toString(), QString("Shellfish"));
    QCOMPARE(roleMap.value("dynamic").toDouble(), 3.0);
    QCOMPARE(roleMap.value("objectName").toString(), QString("Updated"));

    model.clear();
}

QTEST_MAIN(tst_ObjectListModel)

#include "tst_objectlistmodel.moc"

