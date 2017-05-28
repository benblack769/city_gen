#-------------------------------------------------
#
# Project created by QtCreator 2016-05-22T19:55:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = city_gen
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++11 -Wextra -Wall --pedantic
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -march=haswell -mtune=haswell

INCLUDEPATH += ../../

SOURCES += main.cpp\
        mainwindow.cpp \
    test.cpp

HEADERS  += mainwindow.h \
    gamescreen.h \
    people.h \
    constants.h \
    test.h \
    check_obj.h \
    infoholder.h \
    world.h \
    map.h

FORMS    += mainwindow.ui

DISTFILES += \
    notes.md \
    README.md
