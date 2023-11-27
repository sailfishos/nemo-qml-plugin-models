TEMPLATE = lib
TARGET = nemomodels-qt$${QT_MAJOR_VERSION}

CONFIG += create_pc create_prl link_pkgconfig

QT = \
    core \
    qml

PKGCONFIG += mlocale$${QT_MAJOR_VERSION}

INCLUDEPATH *= ../3rdparty

SOURCES += \
    basefiltermodel.cpp \
    compositemodel.cpp \
    filtermodel.cpp \
    objectlistmodel.cpp \
    searchmodel.cpp \
    sortfiltermodel.cpp

HEADERS += \
    basefiltermodel.h \
    compositemodel.h \
    filtermodel.h \
    objectlistmodel.h \
    searchmodel.h \
    sortfiltermodel.h

DEFINES += BUILD_NEMO_QML_PLUGIN_MODELS_LIB

target.path = $$[QT_INSTALL_LIBS]
pkgconfig.files = $$TARGET.pc
pkgconfig.path = $$target.path/pkgconfig
headers.path = /usr/include/$$TARGET
headers.files =\
    basefiltermodel.h \
    compositemodel.h \
    filtermodel.h \
    objectlistmodel.h \
    searchmodel.h \
    nemomodels.h

QMAKE_PKGCONFIG_NAME = lib$$TARGET
QMAKE_PKGCONFIG_DESCRIPTION = Library containing utility models for exposing data to QML
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$headers.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig

INSTALLS += target headers pkgconfig

include(../src.pri)
