#-------------------------------------------------
#
# Project created by QtCreator 2017-10-07T15:01:42
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = FTPPP
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

win32:INCLUDEPATH += c:/openssl/include

unix:LIBS += -lssl -lcrypto
win32:LIBS += -LC:\openssl\lib -llibeay32 -lssleay32 -lws2_32


SOURCES += src/main.cpp\
		src/MainWindow.cpp \
		src/ftplib.cpp \
    src/joinpath.cpp \
    src/LegacyWindowsStyleTreeControl.cpp

HEADERS  += src/MainWindow.h \
		src/ftplib.h \
    src/joinpath.h \
    src/LegacyWindowsStyleTreeControl.h

FORMS    += src/MainWindow.ui
