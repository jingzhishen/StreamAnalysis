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
    unpackthread.cpp \
    packet_data_save.cpp

HEADERS  += mainwindow.h \
    third_ffmpeg.h \
    unpackthread.h \
    analysistype.h \
    packet_data_save.h

FORMS    += mainwindow.ui


INCLUDEPATH += . /usr/local/include
INCLUDEPATH += ./liboutput/ffmpeg/include

LIBS += liboutput/ffmpeg/lib/libavformat.a liboutput/ffmpeg/lib/libavcodec.a liboutput/ffmpeg/lib/libavutil.a
