#-------------------------------------------------
#
# Project created by QtCreator 2017-02-11T23:01:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Editor
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../../Include
INCLUDEPATH += $$(VK_SDK_PATH)/Include

win32 {
   CONFIG(debug, debug|release) {
        LIBS += -L../../Bin64
        DEPENDPATH += ../../Bin64
        DEFINES += _DEBUG
    } else {
        LIBS += -L../../Bin64
        DEPENDPATH += ../../Bin64
    }

    DEFINES += NOMINMAX
    DEFINES += _CRT_SECURE_NO_WARNINGS
    DEFINES += EDITOR_INTERNAL
    RC_INCLUDEPATH += ..\..\Include
}

LIBS += -lEngine

SOURCES += main.cpp\
        EditorWindow.cpp \
        EngineWidget.cpp

HEADERS  += EditorWindow.h \
        EngineWidget.h

FORMS    += EditorWindow.ui

RESOURCES += \
    Editor.qrc
