QT       += network
LIBS += -lpthread libwsock32 libws2_32
LIBS += -lpthread libMswsock libMswsock

HEADERS += \
    $$PWD/qmytcpclient.h \
    $$PWD/TCPNet.h \
    $$PWD/Packdef.h \
    $$PWD/udpnet.h


SOURCES += \
    $$PWD/qmytcpclient.cpp \
    $$PWD/TCPNet.cpp \
    $$PWD/udpnet.cpp

