TARGET = tst_objectlistmodel

QT += testlib

INCLUDEPATH += \
    ../../src/lib \
    ../../src/3rdparty

SOURCES += \
    tst_objectlistmodel.cpp

LIBS += \
    -L../../src/lib \
    -lnemomodels-qt5

target.path = /opt/tests/nemo-qml-plugins/models
INSTALLS += target

include(../../src/src.pri)
