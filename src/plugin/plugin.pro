TARGET = nemomodels
PLUGIN_IMPORT_PATH = org/nemomobile/models

TEMPLATE = lib
CONFIG += plugin
QT += qml quick

INCLUDEPATH += ../lib
LIBS += -L../lib -lnemomodels-qt5

target.path = $$[QT_INSTALL_QML]/$$PLUGIN_IMPORT_PATH
INSTALLS += target

qmldir.files += qmldir
qmldir.path +=  $$target.path
INSTALLS += qmldir

qmltypes.files += plugin.qmltypes
qmltypes.path +=  $$target.path
INSTALLS += qmltypes

SOURCES += \
    plugin.cpp

include(../src.pri)
