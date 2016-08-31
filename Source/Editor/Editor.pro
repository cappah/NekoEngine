#-------------------------------------------------
#
# Project created by QtCreator 2016-02-11T04:48:06
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = Editor
TEMPLATE = app

DEFINES += USE_OPENGL
DEFINES += EDITOR

INCLUDEPATH += ../Engine
INCLUDEPATH += ../Utilities

win32 {
    CONFIG(debug, debug|release) {
        LIBS += -L../../Bin64
        DEPENDPATH += ../../Bin64
        DEFINES += _DEBUG
    } else {
        LIBS += -L../../Bin64
        DEPENDPATH += ../../Bin64
    }

    INCLUDEPATH += ..\..\Include
    INCLUDEPATH += ..\..\3rdparty\include_all
    RC_INCLUDEPATH += ..\..\Include
    DEFINES += NOMINMAX
    DEFINES += _CRT_SECURE_NO_WARNINGS
    DEFINES += EDITOR_INTERNAL
    RC_FILE = Editor.rc
}

unix {
    CONFIG(debug, debug|release) {
        DEFINES += _DEBUG
    }

    LIBS += -L../../Bin64
    DEPENDPATH += ../../Bin64

    INCLUDEPATH += ../../Include
    INCLUDEPATH += ../../3rdparty/include_all
    DEFINES += EDITOR_INTERNAL
    DEFINES += PLATFORM_X11
}

LIBS += -lEngine

SOURCES += main.cpp\
    EngineWidget.cpp \
    EditorWindow.cpp \
    AboutDialog.cpp

HEADERS  += \
    EngineWidget.h \
    EditorWindow.h \
    AboutDialog.h \
    Version.h

FORMS    += \
    EditorWindow.ui \
    AboutDialog.ui

RESOURCES += \
    editor.qrc
