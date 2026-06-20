#-------------------------------------------------
#
# Project created by QtCreator 2026-06-04T00:10:03
#
#-------------------------------------------------

QT       += core gui\
            printsupport\
            widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VoiceUI
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
        scrambler.c\
        stft.c\
        wav.c\
        qcustomplot.cpp

HEADERS  += mainwindow.h\
            scrambler.h \
            stft.h \
            wav.h\
            qcustomplot.h

FORMS    += mainwindow.ui

# Link math library
unix: LIBS += -lm

QMAKE_CFLAGS += -std=c99

QMAKE_CXXFLAGS += -std=c++0x

INCLUDEPATH += /usr/local/qt-arm/tslib-1.4/include
LIBS += -L/usr/local/qt-arm/tslib-1.4/lib -lts



