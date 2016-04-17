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
        LIBS += -L../Engine/debug -L../Utilities/debug -L$$(AL_SDK)/lib/win64 -L$$(COMMON_SDK)/lib/win64 -L$$(GL_SDK)\lib\win64 -lopengl32 -lglew32 -llibvorbisfile -llibpng16d -lsqlited -lzlibstatic -lOpenAL32
        DEPENDPATH += ../Engine/debug
        DEPENDPATH += ../Utilities/debug
        DEPENDPATH += $$(AL_SDK)/lib/win64
        DEPENDPATH += $$(COMMON_SDK)/lib/win64
        DEFINES += _DEBUG
    } else {
        LIBS += -L../Engine/release -L../Utilities/release -L$$(AL_SDK)/lib/win64 -L$$(COMMON_SDK)/lib/win64 -L$$(GL_SDK)\lib\win64 -lopengl32 -lglew32 -llibvorbisfile -llibpng16 -lsqlite -lzlibstatic -lOpenAL32
        DEPENDPATH += ../Engine/release
        DEPENDPATH += ../Utilities/release
        DEPENDPATH += $$(AL_SDK)/lib/win64
        DEPENDPATH += $$(COMMON_SDK)/lib/win64
    }

    INCLUDEPATH += $$(GL_SDK)\include
    INCLUDEPATH += $$(AL_SDK)\include
    INCLUDEPATH += $$(COMMON_SDK)\include
    DEFINES += NOMINMAX
    DEFINES += _CRT_SECURE_NO_WARNINGS
    RC_FILE = Editor.rc
}

LIBS += -lEngine -lUtilities

SOURCES += main.cpp\
        editorwindow.cpp \
    editorglwidget.cpp

HEADERS  += editorwindow.h \
    editorglwidget.h

FORMS    += editorwindow.ui

RESOURCES += \
    editor.qrc
