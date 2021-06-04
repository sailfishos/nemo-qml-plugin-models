TEMPLATE = lib
TARGET = nemomodels-qt5

CONFIG += create_pc create_prl link_pkgconfig

QT = \
    core \
    qml

PKGCONFIG += mlocale5

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

target.path = $$[QT_INSTALL_LIBS]
pkgconfig.files = $$TARGET.pc
pkgconfig.path = $$target.path/pkgconfig
headers.path = /usr/include/$$TARGET
headers.files =\
    basefiltermodel.h \
    compositemodel.h \
    filtermodel.h \
    objectlistmodel.h \
    searchmodel.h

QMAKE_PKGCONFIG_NAME = lib$$TARGET
QMAKE_PKGCONFIG_DESCRIPTION = Library containing utility models for exposing data to QML
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$headers.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig

INSTALLS += target headers pkgconfig

include(../src.pri)
