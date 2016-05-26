#-------------------------------------------------
#
# Project created by QtCreator 2016-05-26T21:45:51
#
#-------------------------------------------------

QT += core gui widgets network

macx {
    QT += macextras
    ICON = icon.icns
    QMAKE_INFO_PLIST = Info.plist
}

CONFIG += c++11

TARGET = TrayGists

TEMPLATE = app

SOURCES += main.cpp\
        MainWindow.cpp

HEADERS += MainWindow.h

RESOURCES += resources.qrc
