QT       += core

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG  += c++11
QMAKE_CXXFLAGS += -std=c++0x

TARGET = multi-level-segmentation
TEMPLATE = app


SOURCES += main.cpp \
           process.cpp

HEADERS += process.h

DEFINES += APP_NAME=\\\"MultiLevelSegmentation\\\"
VERSION = 1.0.1
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

LIBS    += -lopencv_core -lopencv_imgproc -lopencv_highgui
