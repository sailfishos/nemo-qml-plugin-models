TEMPLATE = lib
TARGET = nemomodels-qt5

CONFIG += create_pc create_prl

SOURCES += \
    basefiltermodel.cpp \
    filtermodel.cpp
HEADERS += \
    basefiltermodel.h \
    filtermodel.h

target.path = $$[QT_INSTALL_LIBS]
pkgconfig.files = $$TARGET.pc
pkgconfig.path = $$target.path/pkgconfig
headers.path = /usr/include/$$TARGET
headers.files =\
    basefiltermodel.h \
    filtermodel.h

QMAKE_PKGCONFIG_NAME = lib$$TARGET
QMAKE_PKGCONFIG_DESCRIPTION = Library containing utility models for exposing data to QML
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$headers.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig

INSTALLS += target headers pkgconfig

include(../src.pri)
