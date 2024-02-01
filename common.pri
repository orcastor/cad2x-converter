
# Store intermedia stuff somewhere else
isEmpty(GENERATED_DIR){
 GENERATED_DIR = generated
}
 # Store intermedia stuff somewhere else
OBJECTS_DIR = $${GENERATED_DIR}/obj
MOC_DIR = $${GENERATED_DIR}/moc


# Copy command
win32 {
    COPY = copy /y
} else {
    COPY = cp
}

# use c++ only
QMAKE_CC = g++
QMAKE_CFLAGS = -std=c++17

QMAKE_LFLAGS += -Wl,-rpath=.
 
# Windows compiler settings
win32 {
    QMAKE_CXXFLAGS += -U__STRICT_ANSI__
    QMAKE_CFLAGS_THREAD -= -mthreads
    QMAKE_CXXFLAGS_THREAD -= -mthreads
    QMAKE_LFLAGS_THREAD -= -mthreads
    #qt version check for mingw
    win32-g++ {
        contains(QT_VERSION, ^4\\.8\\.[0-4]) {
            DEFINES += QT_NO_CONCURRENT=0
        }
        # Silence warning: typedef '...' locally defined but not used [-Wunused-local-typedefs]
        # this was caused by boost headers and g++ 4.8.0 (Qt 5.1 / MinGW 4.8)
        greaterThan( QT_MAJOR_VERSION, 4 ) {
            QMAKE_CXXFLAGS += -Wno-unused-local-typedefs
        }
    }else{
       !build_pass:verbose:message(Setting up support for MSVC.)
       # define the M_PI etc macros for MSVC compilers.
       DEFINES += _USE_MATH_DEFINES
    }

    # The .NET 2003 compiler (at least) is touchy about its own headers ...
    win32-msvc2003 {
       # Silence "unused formal parameter" warnings about unused `_Iosbase`
       # in the header file `xloctime` (a Vc7 header after all!).
       QMAKE_CXXFLAGS += /wd4100
    }
}

unix|macx|win32-g++ {
# no such option for MSVC
QMAKE_CXXFLAGS_DEBUG += -g
QMAKE_CXXFLAGS += -g
}

# fix for GitHub Issue #880
# prevent QMake from using -isystem flag for system include path
# this breaks gcc 6 builds because of its #include_next feature
QMAKE_CFLAGS_ISYSTEM = ""

greaterThan( QT_MAJOR_VERSION, 5) {
    CONFIG += c++17
}else{
    unix|macx|win32-g++ {
        # no such option for MSVC
    QMAKE_CXXFLAGS += -std=c++17
    QMAKE_CXXFLAGS_DEBUG += -std=c++17
    }
}

# RVT July 12 2015, I believe we need these here
#macx{
#    QMAKE_CXXFLAGS_DEBUG += -mmacosx-version-min=10.8
#    QMAKE_CXXFLAGS += -mmacosx-version-min=10.8
#}

SUBDIRS += 3rdparty\qtbase\src\3rdparty\freetype

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += freetype2
}

win32 {
    # set this line to your freetype installation
    # download it from http://gnuwin32.sourceforge.net/packages/freetype.htm

    isEmpty( FREETYPE_DIR ) {
        # FREETYPE_DIR was not set in custom.pro
        FREETYPE_DIR = /Qt/freetype
    }
    !exists("$$FREETYPE_DIR") {
        # check env for FREETYPE_DIR
        exists("$$(FREETYPE_DIR)"){
            FREETYPE_DIR = "$$(FREETYPE_DIR)"
        }
    }

    exists($${FREETYPE_DIR}/include/ft2build.h) {
        INCLUDEPATH += "$${FREETYPE_DIR}/include" "$${FREETYPE_DIR}/include/freetype2"
        LIBS += -L"$${FREETYPE_DIR}/lib" -lfreetype -lz

        message(using includes in $${FREETYPE_DIR}/include and $${FREETYPE_DIR}/include/freetype2)
        message(using libs in $${FREETYPE_DIR}/lib)
    } else {
        message("freetype was not found in $${FREETYPE_DIR}, please install freetype or check settings in custom.pro!")
    }
}

INSTALLDIR = ../output
