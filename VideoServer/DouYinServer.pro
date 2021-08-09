TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QT       += core gui

LIBS += -lmysqlclient
LIBS += -lpthread
LIBS += -lhiredis

INCLUDEPATH +=./include

SOURCES += \
    RedisConfig.cpp \
    RedisTools.cpp \
    recommendation.cpp \
    src/Mysql.cpp \
    src/TCPKernel.cpp \
    src/TCPNet.cpp \
    src/Thread_pool.cpp \
    src/err_str.cpp \
    src/main.cpp

HEADERS += \
    RedisConfig.h \
    RedisTools.h \
    include/Mysql.h \
    include/TCPKernel.h \
    include/TCPNet.h \
    include/Thread_pool.h \
    include/err_str.h \
    include/packdef.h \
    recommendation.h
