#-------------------------------------------------
#
# Project created by QtCreator 2016-05-22T19:55:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = city_gen
TEMPLATE = app

INCLUDEPATH += ../../headerlib

SOURCES += main.cpp\
        mainwindow.cpp \
    gamescreen.cpp \
    people.cpp \
    firms.cpp \
    blocks.cpp \
    test.cpp

HEADERS  += mainwindow.h \
    gamescreen.h \
    people.h \
    firms.h \
    constants.h \
    blocks.h \
    test.h \
    parrelell.h

FORMS    += mainwindow.ui
