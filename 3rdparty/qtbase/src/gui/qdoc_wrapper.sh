#!/bin/sh
QT_VERSION=5.12.12
export QT_VERSION
QT_VER=5.12
export QT_VER
QT_VERSION_TAG=51212
export QT_VERSION_TAG
QT_INSTALL_DOCS=/usr/share/qt5/doc
export QT_INSTALL_DOCS
BUILDDIR=/opt/cad2x-converter/3rdparty/qtbase/src/gui
export BUILDDIR
exec /usr/lib/qt5/bin/qdoc "$@"
