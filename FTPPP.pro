#-------------------------------------------------
#
# Project created by QtCreator 2017-10-07T15:01:42
#
#-------------------------------------------------

QT       += core gui widgets

win32:QT += winextras


TARGET = FTPPP
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += $$PWD/src

win32:INCLUDEPATH += c:/openssl/include

unix:LIBS += -lssl -lcrypto
win32:LIBS += -LC:\openssl\lib -llibeay32 -lssleay32 -lws2_32


SOURCES += src/main.cpp\
		src/MainWindow.cpp \
		src/ftplib.cpp \
    src/joinpath.cpp \
    src/LegacyWindowsStyleTreeControl.cpp \
    src/FTP.cpp \
    src/AbstractSettingForm.cpp \
    src/SettingsDialog.cpp \
    src/MySettings.cpp \
    src/WinIcon.cpp

HEADERS  += src/MainWindow.h \
		src/ftplib.h \
    src/joinpath.h \
    src/LegacyWindowsStyleTreeControl.h \
    src/FTP.h \
    src/AbstractSettingForm.h \
    src/SettingsDialog.h \
    src/main.h \
    src/MySettings.h \
    src/WinIcon.h

FORMS    += src/MainWindow.ui \
    src/SettingsDialog.ui
