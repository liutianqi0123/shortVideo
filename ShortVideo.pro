#-------------------------------------------------
#
# Project created by QtCreator 2021-03-06T20:54:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(./netapi/netapi.pri)
INCLUDEPATH += $$PWD/netapi

include(./RecordVideo/RecordVideo.pri)
INCLUDEPATH += $$PWD/RecordVideo

#include(./RecordAudio/RecordAudio.pri)
#INCLUDEPATH += $$PWD/RecordAudio

INCLUDEPATH += $$PWD/ffmpeg-4.2.2/include\
               $$PWD/SDL2-2.0.10/include

LIBS += $$PWD/ffmpeg-4.2.2/lib/avcodec.lib\
 $$PWD/ffmpeg-4.2.2/lib/avdevice.lib\
 $$PWD/ffmpeg-4.2.2/lib/avfilter.lib\
 $$PWD/ffmpeg-4.2.2/lib/avformat.lib\
 $$PWD/ffmpeg-4.2.2/lib/avutil.lib\
 $$PWD/ffmpeg-4.2.2/lib/postproc.lib\
 $$PWD/ffmpeg-4.2.2/lib/swresample.lib\
 $$PWD/ffmpeg-4.2.2/lib/swscale.lib\
 $$PWD/SDL2-2.0.10/lib/x86/SDL2.lib


TARGET = ShortVideo
TEMPLATE = app

RC_ICONS = 001.ico

FORMS += \
    videoitem.ui \
    logindialog.ui \
    broadcast.ui \
    myvideodialog.ui \
    mydouyin.ui \
    useritem.ui \
    fanslistdlg.ui

HEADERS += \
    ckernel.h \
    logindialog.h \
    mydouyin.h \
    packetqueue.h \
    videoitem.h \
    videoplayer.h \
    broadcast.h \
    myvideodialog.h \
    useritem.h \
    fanslistdlg.h \
    IMToolBox.h

SOURCES += \
    ckernel.cpp \
    logindialog.cpp \
    main.cpp \
    mydouyin.cpp \
    packetqueue.cpp \
    videoitem.cpp \
    videoplayer.cpp \
    broadcast.cpp \
    myvideodialog.cpp \
    useritem.cpp \
    fanslistdlg.cpp \
    IMToolBox.cpp


RESOURCES += \
    resource.qrc \

DISTFILES += \
    images/groupChat 2.png \
    images/groupChat 2.png
