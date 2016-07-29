TARGET = nemomodels
PLUGIN_IMPORT_PATH = org/nemomobile/models

TEMPLATE = lib
CONFIG += plugin
QT += qml quick

INCLUDEPATH += ../lib
LIBS += -L../lib -lnemomodels-qt5

target.path = $$[QT_INSTALL_QML]/$$PLUGIN_IMPORT_PATH
INSTALLS += target

qmldir.files += $$_PRO_FILE_PWD_/qmldir
qmldir.path +=  $$target.path
INSTALLS += qmldir

qmltypes.files += $$_PRO_FILE_PWD_/plugin.qmltypes
qmltypes.path +=  $$target.path
INSTALLS += qmltypes

SOURCES += \
    plugin.cpp

include(../src.pri)
