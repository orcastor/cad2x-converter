#!/bin/sh
QT_VERSION=5.12.12
export QT_VERSION
QT_VER=5.12
export QT_VER
QT_VERSION_TAG=51212
export QT_VERSION_TAG
BUILDDIR=/opt/cad2x-converter/3rdparty/qtbase/src/corelib
export BUILDDIR
exec /opt/cad2x-converter/3rdparty/qtbase/bin/qdoc "$@"
