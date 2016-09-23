#-------------------------------------------------
#
# Project created by QtCreator 2016-08-11T14:21:48
#
#-------------------------------------------------

QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Feng-QDC-DAQ
DESTDIR = bin
targetinstall.path = /usr/local/bin/Feng-QDC-DAQ
targetinstall.files = bin/Feng-QDC-DAQ

INSTALLS += targetinstall

icon.path = /usr/local/Feng-QDC-DAQ
icon.files = data/icon-Feng-QDC-DAQ.png
icon.extra = cp data/Feng-QDC-DAQ.desktop ~/Desktop/Feng-QDC-DAQ.desktop

INSTALLS += icon

TEMPLATE = app


SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/qcustomplot.cpp \
    src/histogram.cpp \ 
    src/qdc.cpp \
    src/qdcmodule.cpp

OBJECTS_DIR = build/.obj
MOC_DIR = build/.moc
RCC_DIR = build/.rcc
UI_DIR = build/.ui

INCLUDEPATH += include

HEADERS  += include/mainwindow.h \
    include/qcustomplot.h \
    include/histogram.h \ 
    include/qdc.h \
    include/qdcmodule.h

FORMS += form/mainwindow.ui \ 
    form/qdc.ui

RESOURCES += data/resources.qrc

QMAKE_CXXFLAGS += -std=c++0x -DLINUX

LIBS += -lCAENVME
