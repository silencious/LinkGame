#-------------------------------------------------
#
# Project created by QtCreator 2014-07-09T19:47:11
#
#-------------------------------------------------

QT       += core gui multimedia xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LinkGame
TEMPLATE = app


SOURCES += main.cpp\
        linkgamegui.cpp \
    block.cpp \
    board.cpp \
    logic.cpp \
    soundplayer.cpp

HEADERS  += linkgamegui.h \
    block.h \
    board.h \
    logic.h \
    soundplayer.h

FORMS    += linkgamegui.ui

RESOURCES += \
    res.qrc

CONFIG += resources_big