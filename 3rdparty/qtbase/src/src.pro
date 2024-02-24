TEMPLATE = subdirs

QT_FOR_CONFIG += core-private gui-private
include($$OUT_PWD/corelib/qtcore-config.pri)
include($$OUT_PWD/gui/qtgui-config.pri)

src_qtzlib.file = $$PWD/corelib/qtzlib.pro
src_qtzlib.target = sub-zlib

src_tools_bootstrap.subdir = tools/bootstrap
src_tools_bootstrap.target = sub-bootstrap

src_tools_moc.subdir = tools/moc
src_tools_moc.target = sub-moc
src_tools_moc.depends = src_tools_bootstrap

src_tools_qfloat16_tables.subdir = tools/qfloat16-tables
src_tools_qfloat16_tables.target = sub-qfloat16-tables
src_tools_qfloat16_tables.depends = src_tools_bootstrap

src_winmain.subdir = $$PWD/winmain
src_winmain.target = sub-winmain
src_winmain.depends = sub-corelib  # just for the module .pri file

src_corelib.subdir = $$PWD/corelib
src_corelib.target = sub-corelib
src_corelib.depends = src_tools_moc src_tools_qfloat16_tables

src_3rdparty_harfbuzzng.subdir = $$PWD/3rdparty/harfbuzz-ng
src_3rdparty_harfbuzzng.target = sub-3rdparty-harfbuzzng
src_3rdparty_harfbuzzng.depends = src_corelib   # for the Qt atomics

src_3rdparty_libpng.subdir = $$PWD/3rdparty/libpng
src_3rdparty_libpng.target = sub-3rdparty-libpng

src_3rdparty_freetype.subdir = $$PWD/3rdparty/freetype
src_3rdparty_freetype.target = sub-3rdparty-freetype

src_3rdparty_gradle.subdir = $$PWD/3rdparty/gradle
src_3rdparty_gradle.target = sub-3rdparty-gradle

src_gui.subdir = $$PWD/gui
src_gui.target = sub-gui
src_gui.depends = src_corelib

src_plugins.subdir = $$PWD/plugins
src_plugins.target = sub-plugins

src_android.subdir = $$PWD/android

# this order is important
!qtConfig(system-zlib)|cross_compile {
    SUBDIRS += src_qtzlib
    !qtConfig(system-zlib) {
        src_3rdparty_libpng.depends += src_corelib
        src_3rdparty_freetype.depends += src_corelib
    }
}
SUBDIRS += src_tools_bootstrap src_tools_moc src_tools_qfloat16_tables
TOOLS = src_tools_moc src_tools_qfloat16_tables
SUBDIRS += src_corelib
win32:SUBDIRS += src_winmain

qtConfig(gui) {
    qtConfig(harfbuzz):!qtConfig(system-harfbuzz) {
        SUBDIRS += src_3rdparty_harfbuzzng
        src_gui.depends += src_3rdparty_harfbuzzng
    }
    qtConfig(png):!qtConfig(system-png) {
        SUBDIRS += src_3rdparty_libpng
        src_3rdparty_freetype.depends += src_3rdparty_libpng
        src_gui.depends += src_3rdparty_libpng
    }
    qtConfig(freetype):!qtConfig(system-freetype) {
        SUBDIRS += src_3rdparty_freetype
        src_gui.depends += src_3rdparty_freetype
    }
    SUBDIRS += src_gui
    src_plugins.depends += src_gui
}

TR_EXCLUDE = \
    src_tools_bootstrap src_tools_moc \
    src_3rdparty_pcre2 src_3rdparty_harfbuzzng src_3rdparty_freetype

sub-tools.depends = $$TOOLS
QMAKE_EXTRA_TARGETS = sub-tools
