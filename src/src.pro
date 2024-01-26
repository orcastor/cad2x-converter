TEMPLATE = app

#uncomment to enable a Debugging menu entry for basic unit testing
#DEFINES += LC_DEBUGGING

DEFINES += DWGSUPPORT

LC_VERSION="2.2.0"
VERSION=$${LC_VERSION}
DEFINES += LC_VERSION=\"$$LC_VERSION\"

# Store intermedia stuff somewhere else
GENERATED_DIR = ../generated/cad2x
# Use common project definitions.
include(../common.pri)

macx|win32|equals(build_muparser, "true")|!packagesExist(muparser){
    message("Using bundled muparser")
	MUPARSER_DIR = ../3rdparty/muparser
	DEPENDPATH += $$MUPARSER_DIR/include \
				$$MUPARSER_DIR/src
	INCLUDEPATH += $$MUPARSER_DIR/include
	GEN_LIB_DIR = ../generated/lib
	LIBS += -L$$GEN_LIB_DIR -lmuparser
	msvc {
		PRE_TARGETDEPS += $$GEN_LIB_DIR/muparser.lib
	} else {
		PRE_TARGETDEPS += $$GEN_LIB_DIR/libmuparser.a
	}
}else{
    message("Using external muparser")
    CONFIG += link_pkgconfig
    PKGCONFIG += muparser
}

QTBASE_DIR = ../3rdparty/qtbase
DEPENDPATH += $$QTBASE_DIR/include \
            $$QTBASE_DIR/src
INCLUDEPATH += $$QTBASE_DIR/include $$QTBASE_DIR/include/QtCore $$QTBASE_DIR/include/QtGui
GEN_LIB_DIR = $$QTBASE_DIR/lib
LIBS += -L$$GEN_LIB_DIR -lQt5Core -lQt5Gui
msvc {
    PRE_TARGETDEPS += $$GEN_LIB_DIR/Qt5Core.lib $$GEN_LIB_DIR/Qt5Gui.lib
} else {
    PRE_TARGETDEPS += $$GEN_LIB_DIR/libQt5Core.a $$GEN_LIB_DIR/libQt5Gui.a
}

QT = 

CONFIG += static \
    warn_on \
    link_prl \
    verbose \
    depend_includepath

CONFIG += c++17
*-g++ {
    QMAKE_CXXFLAGS += -fext-numeric-literals
}

GEN_LIB_DIR = ../generated/lib
msvc {
	PRE_TARGETDEPS += $$GEN_LIB_DIR/dxfrw.lib
} else {
	PRE_TARGETDEPS += $$GEN_LIB_DIR/libdxfrw.a
}

DESTDIR = $${INSTALLDIR}

TARGET = cad2x

# Additional 3rdparty to load
LIBS += -L../generated/lib  \
    -ldxfrw

INCLUDEPATH += \
    ../3rdparty/libdxfrw/src \
    ../src

# ################################################################################
# Library
HEADERS += \
    drw_base.h \
    drw_entities.h \
    drw_objects.h \
    dxf_format.h \
    lc_defaults.h \
    lc_looputils.h \
    lc_makercamsvg.h \
    lc_quadratic.h \
    lc_rect.h \
    lc_splinepoints.h \
    lc_xmlwriterinterface.h \
    lc_xmlwriterqxmlstreamwriter.h \
    libdwgr.h \
    rs.h \
    rs_arc.h \
    rs_atomicentity.h \
    rs_block.h \
    rs_blocklist.h \
    rs_circle.h \
    rs_color.h \
    rs_constructionline.h \
    rs_debug.h \
    rs_dimaligned.h \
    rs_dimlinear.h \
    rs_dimangular.h \
    rs_dimdiametric.h \
    rs_dimension.h \
    rs_dimradial.h \\
    rs_document.h \
    rs_ellipse.h \
    rs_entity.h \
    rs_entitycontainer.h \
    rs_fileio.h \
    rs_filterdxfrw.h \
    rs_filterdxf1.h \
    rs_filterinterface.h \
    rs_flags.h \
    rs_font.h \
    rs_fontchar.h \
    rs_fontlist.h \
    rs_graphic.h \
    rs_graphicview.h \
    rs_grid.h \
    rs_hatch.h \
    rs_image.h \
    rs_information.h \
    rs_insert.h \
    rs_layer.h \
    rs_layerlist.h \
    rs_leader.h \
    rs_line.h \
    rs_linetypepattern.h \
    rs_math.h \
    rs_mtext.h \
    rs_painter.h \
    rs_painterqt.h \
    rs_pattern.h \
    rs_patternlist.h \
    rs_pen.h \
    rs_point.h \
    rs_polyline.h \
    rs_solid.h \
    rs_spline.h \
    rs_text.h \
    rs_units.h \
    rs_utility.h \
    rs_variable.h \
    rs_variabledict.h \
    rs_vector.h

SOURCES += \
    main.cpp \
    drw_base.cpp \
    drw_entities.cpp \
    drw_objects.cpp \
    lc_looputils.cpp \
    lc_makercamsvg.cpp \
    lc_quadratic.cpp \
    lc_rect.cpp \
    lc_splinepoints.cpp \
    lc_xmlwriterqxmlstreamwriter.cpp \
    libdwgr.cpp \
    rs.cpp \
    rs_arc.cpp \
    rs_atomicentity.cpp \
    rs_block.cpp \
    rs_blocklist.cpp \
    rs_circle.cpp \
    rs_color.cpp \
    rs_constructionline.cpp \
    rs_debug.cpp \
    rs_dimaligned.cpp \
    rs_dimlinear.cpp \
    rs_dimangular.cpp \
    rs_dimdiametric.cpp \
    rs_dimension.cpp \
    rs_dimradial.cpp \
    rs_document.cpp \
    rs_ellipse.cpp \
    rs_entity.cpp \
    rs_entitycontainer.cpp \
    rs_fileio.cpp \
    rs_filterdxfrw.cpp \
    rs_filterdxf1.cpp \
    rs_flags.cpp \
    rs_font.cpp \
    rs_fontlist.cpp \
    rs_graphic.cpp \
    rs_graphicview.cpp \
    rs_grid.cpp \
    rs_hatch.cpp \
    rs_image.cpp \
    rs_information.cpp \
    rs_insert.cpp \
    rs_layer.cpp \
    rs_layerlist.cpp \
    rs_leader.cpp \
    rs_line.cpp \
    rs_linetypepattern.cpp \
    rs_math.cpp \
    rs_mtext.cpp \
    rs_painter.cpp \
    rs_painterqt.cpp \
    rs_pattern.cpp \
    rs_patternlist.cpp \
    rs_pen.cpp \
    rs_point.cpp \
    rs_polyline.cpp \
    rs_solid.cpp \
    rs_spline.cpp \
    rs_text.cpp \
    rs_units.cpp \
    rs_utility.cpp \
    rs_variabledict.cpp \
    rs_vector.cpp

# If C99 emulation is needed, add the respective source files.
contains(DEFINES, EMU_C99) {
    !build_pass:verbose:message(Emulating C99 math features.)
    SOURCES += main/emu_c99.cpp
    HEADERS += main/emu_c99.h
}
