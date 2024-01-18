# LibreCAD project file
# (c) Ries van Twisk (librecad@rvt.dds.nl)
TEMPLATE = app

#uncomment to enable a Debugging menu entry for basic unit testing
#DEFINES += LC_DEBUGGING

DEFINES += DWGSUPPORT

LC_VERSION="2.2.0-alpha"
VERSION=$${LC_VERSION}
DEFINES += LC_VERSION=\"$$LC_VERSION\"

# Store intermedia stuff somewhere else
GENERATED_DIR = ../generated/cad2pic
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

# QT -= widgets

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

TARGET = cad2pic

# Additional 3rdparty to load
LIBS += -L../generated/lib  \
    -ldxfrw

INCLUDEPATH += \
    ../3rdparty/libdxfrw/src \
    ../src

# ################################################################################
# Library
HEADERS += \
    ../src/drw_base.h \
    ../src/drw_entities.h \
    ../src/drw_objects.h \
    ../src/dxf_format.h \
    ../src/lc_defaults.h \
    ../src/lc_dimarc.h \
    ../src/lc_looputils.h \
    ../src/lc_makercamsvg.h \
    ../src/lc_quadratic.h \
    ../src/lc_rect.h \
    ../src/lc_splinepoints.h \
    ../src/lc_xmlwriterinterface.h \
    ../src/lc_xmlwriterqxmlstreamwriter.h \
    ../src/libdwgr.h \
    ../src/rs.h \
    ../src/rs_arc.h \
    ../src/rs_atomicentity.h \
    ../src/rs_block.h \
    ../src/rs_blocklist.h \
    ../src/rs_blocklistlistener.h \
    ../src/rs_circle.h \
    ../src/rs_color.h \
    ../src/rs_constructionline.h \
    ../src/rs_debug.h \
    ../src/rs_dimaligned.h \
    ../src/rs_dimlinear.h \
    ../src/rs_dimangular.h \
    ../src/rs_dimdiametric.h \
    ../src/rs_dimension.h \
    ../src/rs_dimradial.h \\
    ../src/rs_document.h \
    ../src/rs_ellipse.h \
    ../src/rs_entity.h \
    ../src/rs_entitycontainer.h \
    ../src/rs_fileio.h \
    ../src/rs_filterdxfrw.h \
    ../src/rs_filterdxf1.h \
    ../src/rs_filterinterface.h \
    ../src/rs_flags.h \
    ../src/rs_font.h \
    ../src/rs_fontchar.h \
    ../src/rs_fontlist.h \
    ../src/rs_graphic.h \
    ../src/rs_graphicview.h \
    ../src/rs_grid.h \
    ../src/rs_hatch.h \
    ../src/rs_image.h \
    ../src/rs_information.h \
    ../src/rs_insert.h \
    ../src/rs_layer.h \
    ../src/rs_layerlist.h \
    ../src/rs_layerlistlistener.h \
    ../src/rs_leader.h \
    ../src/rs_line.h \
    ../src/rs_linetypepattern.h \
    ../src/rs_math.h \
    ../src/rs_mtext.h \
    ../src/rs_painter.h \
    ../src/rs_painterqt.h \
    ../src/rs_pattern.h \
    ../src/rs_patternlist.h \
    ../src/rs_pen.h \
    ../src/rs_point.h \
    ../src/rs_polyline.h \
    ../src/rs_solid.h \
    ../src/rs_spline.h \
    ../src/rs_text.h \
    ../src/rs_undo.h \
    ../src/rs_undoable.h \
    ../src/rs_undocycle.h \
    ../src/rs_units.h \
    ../src/rs_utility.h \
    ../src/rs_variable.h \
    ../src/rs_variabledict.h \
    ../src/rs_vector.h

SOURCES += \
    main.cpp \
    ../src/drw_base.cpp \
    ../src/drw_entities.cpp \
    ../src/drw_objects.cpp \
    ../src/lc_dimarc.cpp \
    ../src/lc_looputils.cpp \
    ../src/lc_makercamsvg.cpp \
    ../src/lc_quadratic.cpp \
    ../src/lc_rect.cpp \
    ../src/lc_splinepoints.cpp \
    ../src/lc_xmlwriterqxmlstreamwriter.cpp \
    ../src/libdwgr.cpp \
    ../src/rs.cpp \
    ../src/rs_arc.cpp \
    ../src/rs_atomicentity.cpp \
    ../src/rs_block.cpp \
    ../src/rs_blocklist.cpp \
    ../src/rs_circle.cpp \
    ../src/rs_color.cpp \
    ../src/rs_constructionline.cpp \
    ../src/rs_debug.cpp \
    ../src/rs_dimaligned.cpp \
    ../src/rs_dimlinear.cpp \
    ../src/rs_dimangular.cpp \
    ../src/rs_dimdiametric.cpp \
    ../src/rs_dimension.cpp \
    ../src/rs_dimradial.cpp \
    ../src/rs_document.cpp \
    ../src/rs_ellipse.cpp \
    ../src/rs_entity.cpp \
    ../src/rs_entitycontainer.cpp \
    ../src/rs_fileio.cpp \
    ../src/rs_filterdxfrw.cpp \
    ../src/rs_filterdxf1.cpp \
    ../src/rs_flags.cpp \
    ../src/rs_font.cpp \
    ../src/rs_fontlist.cpp \
    ../src/rs_graphic.cpp \
    ../src/rs_graphicview.cpp \
    ../src/rs_grid.cpp \
    ../src/rs_hatch.cpp \
    ../src/rs_image.cpp \
    ../src/rs_information.cpp \
    ../src/rs_insert.cpp \
    ../src/rs_layer.cpp \
    ../src/rs_layerlist.cpp \
    ../src/rs_leader.cpp \
    ../src/rs_line.cpp \
    ../src/rs_linetypepattern.cpp \
    ../src/rs_math.cpp \
    ../src/rs_mtext.cpp \
    ../src/rs_painter.cpp \
    ../src/rs_painterqt.cpp \
    ../src/rs_pattern.cpp \
    ../src/rs_patternlist.cpp \
    ../src/rs_pen.cpp \
    ../src/rs_point.cpp \
    ../src/rs_polyline.cpp \
    ../src/rs_solid.cpp \
    ../src/rs_spline.cpp \
    ../src/rs_text.cpp \
    ../src/rs_undo.cpp \
    ../src/rs_undoable.cpp \
    ../src/rs_undocycle.cpp \
    ../src/rs_units.cpp \
    ../src/rs_utility.cpp \
    ../src/rs_variabledict.cpp \
    ../src/rs_vector.cpp

# If C99 emulation is needed, add the respective source files.
contains(DEFINES, EMU_C99) {
    !build_pass:verbose:message(Emulating C99 math features.)
    SOURCES += main/emu_c99.cpp
    HEADERS += main/emu_c99.h
}
