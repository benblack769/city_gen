#-------------------------------------------------
#
# Project created by QtCreator 2016-05-22T19:55:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = city_gen
TEMPLATE = app
QMAKE_CXXFLAGS += -Wextra -Wall
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
    map.h \
    choices.h \
    choice_maker.h \
    intelligence_inputs.h \
    intelligence/intelligence_inputs.h \
    intelligence/choice_maker.h \
    intelligence/choices.h \
    intelligence/intelligence_orchestrator.h \
    utils.h \
    intelligence/math.h \
    intelligence_option/intelligence/choice_maker.h \
    intelligence_option/intelligence/choices.h \
    intelligence_option/intelligence/intelligence_inputs.h \
    intelligence_option/intelligence/intelligence_orchestrator.h \
    intelligence_option/intelligence/math.h \
    intelligence_option/intelligence.h \
    intelligence_option/hard_strategy/strategy.h \
    intelligence_option/choices.h

FORMS    += mainwindow.ui

DISTFILES += \
    notes.md \
    README.md \
    .gitignore \
    LICENSE \
    intelligence_option/intelligence/.choice_maker.h.swp \
    play.txt
