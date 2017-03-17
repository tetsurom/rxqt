QT += core widgets testlib

CONFIG += c++11

TARGET = rxtest
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../Rx/v2/src
INCLUDEPATH += ../include

SOURCES += \
    signaltest.cpp
