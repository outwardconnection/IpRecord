#-------------------------------------------------
#
# Project created by QtCreator 2016-09-19T20:04:05
#
#-------------------------------------------------

QT       += core gui network xml multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IpRecord
TEMPLATE = app


SOURCES += main.cpp\
    ipdetect.cpp

HEADERS  += \
    ipdetect.h

FORMS    += \
    iprecordgui.ui

RESOURCES += \
    res.qrc

DISTFILES += \
    ico.rc

RC_FILE += \
    ico.rc

