TARGET = nemomodels
PLUGIN_IMPORT_PATH = org/nemomobile/models

TEMPLATE = lib
CONFIG += plugin
QT = \
    core \
    qml

INCLUDEPATH += ../lib
LIBS += -L../lib -lnemomodels-qt5

target.path = $$[QT_INSTALL_QML]/$$PLUGIN_IMPORT_PATH
INSTALLS += target

qmldir.files += qmldir plugin.qmltypes
qmldir.path +=  $$target.path
INSTALLS += qmldir

qmltypes.commands = qmlplugindump -nonrelocatable org.nemomobile.models 1.0 > $$PWD/plugins.qmltypes
QMAKE_EXTRA_TARGETS += qmltypes

SOURCES += \
    plugin.cpp

include(../src.pri)
