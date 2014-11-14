#-------------------------------------------------
#
# Project created by QtCreator 2014-10-30T18:45:54
#
#-------------------------------------------------

QT       += core gui sql

TARGET = StreamAnalysis
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    unpackthread.cpp

HEADERS  += mainwindow.h \
    third_ffmpeg.h \
    unpackthread.h \
    analysistype.h

FORMS    += mainwindow.ui


INCLUDEPATH += . /usr/local/include
INCLUDEPATH += ./thirdparty/ffmpeg/include

LIBS += -Lthirdparty/ffmpeg/lib -lavformat -lavcodec -lavutil
