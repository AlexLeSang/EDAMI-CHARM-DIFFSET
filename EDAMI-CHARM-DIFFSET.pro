#-------------------------------------------------
#
# Project created by QtCreator 2013-05-06T14:54:02
#
#-------------------------------------------------

QT       -= core
QT       -= gui

TARGET = EDAMI-CHARM-DIFFSET
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp \
    Node.cpp \
    Charm.cpp

HEADERS += \
    Typedefs.hpp \
    Node.hpp \
    Charm.hpp \
    CSet.hpp \
    ResultSaver.hpp \
    DatabaseReader.hpp \
    Item.hpp \
    Itemset.hpp \
    Database.hpp \
    Diffset.hpp

#QMAKE_CXX = g++-4.7
