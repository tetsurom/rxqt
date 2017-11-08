QT += core widgets

CONFIG += c++14

TARGET = npleclick
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../../RxCpp/Rx/v2/src
INCLUDEPATH += ../../include

SOURCES += \
    main.cpp
