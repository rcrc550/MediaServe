TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -lhiredis
SOURCES += \
    src/Mysql.cpp \
    src/TCPKernel.cpp \
    src/err_str.cpp \
    src/main.cpp \
    src/redistool.cpp \
    src/tcpnet.cpp \
    src/threadpool.cpp
HEADERS += \
    include/Mysql.h \
    include/TCPKernel.h \
    include/err_str.h \
    include/packdef.h \
    include/redistool.h \
    include/tcpnet.h \
    include/threadpool.h

INCLUDEPATH += ./include \

LIBS += -lmysqlclient -lpthread

