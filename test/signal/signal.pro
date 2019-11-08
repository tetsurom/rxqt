QT *= core testlib
QT -= gui

TARGET = signal_test
TEMPLATE = app
CONFIG *= c++14 console testcase
CONFIG -= app_bundle

INCLUDEPATH += ../../RxCpp/Rx/v2/src
INCLUDEPATH += ../../include

PRECOMPILED_HEADER = pch.hpp

SOURCES += \
    signal_test.cpp
