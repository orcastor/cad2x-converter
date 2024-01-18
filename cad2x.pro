TEMPLATE = subdirs

TARGET = cad2x
CONFIG += ordered
SUBDIRS     = \
    3rdparty \
    src

# c++17 is now obligatory for cad2
message(We will be using CPP17 features)
QMAKE_CXXFLAGS += -O2 -std=c++17
