TEMPLATE = lib

CONFIG += c++20 static warn_on

DESTDIR = ../../generated/lib

DLL_NAME = charsetdetect
TARGET = $$DLL_NAME

GENERATED_DIR = ../../generated/lib/libcharsetdetect
# Use common project definitions.
include(../../common.pri)

QT =

HEADERS += \
    include/charsetdetect.h \
	include/charsetdetectPriv.h \
	include/nsUniversalDetector.h \
	include/nscore.h \
	include/prmem.h

DEPENDPATH += include
INCLUDEPATH += $$DEPENDPATH

SOURCES += \
	src/Big5Freq.tab \
	src/CharDistribution.cpp \
	src/CharDistribution.h \
	src/EUCKRFreq.tab \
	src/EUCTWFreq.tab \
	src/GB2312Freq.tab \
	src/JISFreq.tab \
	src/JpCntx.cpp \
	src/JpCntx.h \
	src/LangBulgarianModel.cpp \
	src/LangCyrillicModel.cpp \
	src/LangGreekModel.cpp \
	src/LangHebrewModel.cpp \
	src/LangHungarianModel.cpp \
	src/LangThaiModel.cpp \
	src/charsetdetect.cpp \
	src/nsBig5Prober.cpp \
	src/nsBig5Prober.h \
	src/nsCharSetProber.cpp \
	src/nsCharSetProber.h \
	src/nsCodingStateMachine.h \
	src/nsEUCJPProber.cpp \
	src/nsEUCJPProber.h \
	src/nsEUCKRProber.cpp \
	src/nsEUCKRProber.h \
	src/nsEUCTWProber.cpp \
	src/nsEUCTWProber.h \
	src/nsEscCharsetProber.cpp \
	src/nsEscCharsetProber.h \
	src/nsEscSM.cpp \
	src/nsGB2312Prober.cpp \
	src/nsGB2312Prober.h \
	src/nsHebrewProber.cpp \
	src/nsHebrewProber.h \
	src/nsLatin1Prober.cpp \
	src/nsLatin1Prober.h \
	src/nsMBCSGroupProber.cpp \
	src/nsMBCSGroupProber.h \
	src/nsMBCSSM.cpp \
	src/nsPkgInt.h \
	src/nsSBCSGroupProber.cpp \
	src/nsSBCSGroupProber.h \
	src/nsSBCharSetProber.cpp \
	src/nsSBCharSetProber.h \
	src/nsSJISProber.cpp \
	src/nsSJISProber.h \
	src/nsUTF8Prober.cpp \
	src/nsUTF8Prober.h \
	src/nsUniversalDetector.cpp
