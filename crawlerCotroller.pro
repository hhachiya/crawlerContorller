#-------------------------------------------------
#
# Project created by QtCreator 2016-04-21T22:31:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += webenginewidgets
QT += serialport
QT += multimedia multimediawidgets

TARGET = JoystickCameraQTRoboteQ
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp \
    joythread.cpp \
    RoboteqDevice.cpp

HEADERS  += \
    joythread.h \
    dialog.h \
    Constants.h \
    ErrorCodes.h \
    RoboteqDevice.h

FORMS    += dialog.ui

# Joystick
win32:LIBS += "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib/x64/winmm.lib"

#win32: LIBS += -L$$PWD/'../Roboteq/Windows VS2012_VS2015/x64/Debug/' -lRoboteqDevice
#INCLUDEPATH += $$PWD/'../Roboteq/Windows VS2012_VS2015/Files'
#DEPENDPATH += $$PWD/'../Roboteq/Windows VS2012_VS2015/Files'

#win32: LIBS += -L$$PWD/'../Roboteq/Windows VS2012/Debug/RoboteqDevice.lib'
#INCLUDEPATH += $$PWD/'../Roboteq/Windows VS2012/Files'
#DEPENDPATH += $$PWD/'../Roboteq/Windows VS2012/Files'
