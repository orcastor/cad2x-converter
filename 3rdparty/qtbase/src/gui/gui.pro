TARGET     = QtGui
QT = core-private

DEFINES   += QT_NO_USING_NAMESPACE QT_NO_FOREACH QT_NO_CAST_FROM_ASCII

QMAKE_DOCS = $$PWD/doc/qtgui.qdocconf

# Code coverage with TestCocoon
# The following is required as extra compilers use $$QMAKE_CXX instead of $(CXX).
# Without this, testcocoon.prf is read only after $$QMAKE_CXX is used by the
# extra compilers.
testcocoon {
    load(testcocoon)
}

osx: LIBS_PRIVATE += -framework AppKit
darwin: LIBS_PRIVATE += -framework CoreGraphics

CONFIG += simd optimize_full

include(kernel/kernel.pri)
include(image/image.pri)
include(text/text.pri)
include(painting/painting.pri)
include(util/util.pri)
include(math3d/math3d.pri)

QMAKE_LIBS += $$QMAKE_LIBS_GUI

load(qt_module)
load(cmake_functions)

win32: CMAKE_WINDOWS_BUILD = True

QMAKE_DYNAMIC_LIST_FILE = $$PWD/QtGui.dynlist

###
# fontdatabases
###
darwin {
    include($$PWD/mac/coretext.pri)
}

qtConfig(freetype) {
    include($$PWD/freetype/freetype.pri)
}

unix {
    include($$PWD/genericunix/genericunix.pri)
    qtConfig(fontconfig) {
        include($$PWD/fontconfig/fontconfig.pri)
    }
}

win32:!winrt {
    include($$PWD/windows/windows.pri)
}

winrt {
    include($$PWD/winrt/winrt.pri)
}

### end