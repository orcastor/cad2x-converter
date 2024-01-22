# Qt kernel module

# Only used on platforms with CONFIG += precompile_header
PRECOMPILED_HEADER = kernel/qt_gui_pch.h


KERNEL_P= kernel
HEADERS += \
        kernel/qtguiglobal.h \
        kernel/qtguiglobal_p.h \
        kernel/qgenericpluginfactory.h \
        kernel/qgenericplugin.h \
        kernel/qinternalmimedata_p.h \
        kernel/qpalette.h \
        kernel/qwindowdefs.h \
        kernel/qpixelformat.h

SOURCES += \
        kernel/qgenericpluginfactory.cpp \
        kernel/qgenericplugin.cpp \
        kernel/qinternalmimedata.cpp \
        kernel/qpalette.cpp \
        kernel/qguivariant.cpp \
        kernel/qpixelformat.cpp

win32:HEADERS+=kernel/qwindowdefs_win.h
