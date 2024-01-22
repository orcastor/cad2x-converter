# -*-mode:sh-*-
# Qt image handling

# Qt kernel module

HEADERS += \
        image/qimage.h \
        image/qimage_p.h \
        image/qimageiohandler.h \
        image/qimagereader.h \
        image/qimagereaderwriterhelpers_p.h \
        image/qimagewriter.h \
        image/qpaintengine_pic_p.h \
        image/qpicture.h \
        image/qpicture_p.h \
        image/qpictureformatplugin.h

SOURCES += \
        image/qimage.cpp \
        image/qimage_conversions.cpp \
        image/qimageiohandler.cpp \
        image/qimagereader.cpp \
        image/qimagereaderwriterhelpers.cpp \
        image/qimagewriter.cpp \
        image/qpaintengine_pic.cpp \
        image/qpicture.cpp \
        image/qpictureformatplugin.cpp

darwin: OBJECTIVE_SOURCES += image/qimage_darwin.mm

NO_PCH_SOURCES += image/qimage_compat.cpp
false: SOURCES += $$NO_PCH_SOURCES # Hack for QtCreator

# Built-in image format support
HEADERS += \
        image/qbmphandler_p.h \
        image/qppmhandler_p.h \
        image/qxbmhandler_p.h \
        image/qxpmhandler_p.h

SOURCES += \
        image/qbmphandler.cpp \
        image/qppmhandler.cpp \
        image/qxbmhandler.cpp \
        image/qxpmhandler.cpp

qtConfig(png) {
    HEADERS += image/qpnghandler_p.h
    SOURCES += image/qpnghandler.cpp
    QMAKE_USE_PRIVATE += libpng
}

# SIMD
SSSE3_SOURCES += image/qimage_ssse3.cpp
NEON_SOURCES += image/qimage_neon.cpp
MIPS_DSPR2_SOURCES += image/qimage_mips_dspr2.cpp
MIPS_DSPR2_ASM += image/qimage_mips_dspr2_asm.S
