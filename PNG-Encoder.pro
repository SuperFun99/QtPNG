#-------------------------------------------------
#
# Project created by QtCreator 2017-02-28T21:09:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PNG-Encoder
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    KEncodePNG.cpp

HEADERS  += mainwindow.h \
    KEncodePNG.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
