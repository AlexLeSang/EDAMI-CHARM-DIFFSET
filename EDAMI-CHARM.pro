#-------------------------------------------------
#
# Project created by QtCreator 2013-05-06T14:54:02
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = EDAMI-CHARM
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp \
    Tree.cpp \
    Node.cpp

HEADERS += \
    Tree.hpp \
    Typedefs.hpp \
    Node.hpp


#DEFINES += "TREE_TEST"
QMAKE_CXX = g++-4.7
