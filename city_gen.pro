#-------------------------------------------------
#
# Project created by QtCreator 2016-05-22T19:55:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = city_gen
TEMPLATE = app
QMAKE_CXXFLAGS += -Wextra -Wall --pedantic
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -march=native -mtune=native

INCLUDEPATH += ../../headerlib

SOURCES += main.cpp\
        mainwindow.cpp \
    gamescreen.cpp \
    people.cpp \
    firms.cpp \
    blocks.cpp \
    test.cpp \
    block_trans.cpp

HEADERS  += mainwindow.h \
    gamescreen.h \
    people.h \
    firms.h \
    constants.h \
    blocks.h \
    test.h \
    parrelell.h \
    check_obj.h \
    block_trans.h

FORMS    += mainwindow.ui

DISTFILES += \
    notes.md \
    README.md
