#-------------------------------------------------
#
# Project created by QtCreator 2016-09-19T20:04:05
#
#-------------------------------------------------

QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IpRecord
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ipdetect.cpp

HEADERS  += mainwindow.h \
    ipdetect.h

FORMS    += mainwindow.ui
