#-------------------------------------------------
#
# Project created by QtCreator 2016-02-11T04:48:06
#
#-------------------------------------------------

CONFIG += c++11

TARGET = TestGame
TEMPLATE = lib

DEFINES += USE_OPENGL
DEFINES += EDITOR

INCLUDEPATH += ../Engine
INCLUDEPATH += ../Utilities

win32 {
    CONFIG(debug, debug|release) {
        LIBS += -L../Engine/debug -L$$(AL_SDK)/lib/win64 -L$$(COMMON_SDK)/lib/win64 -L$$(GL_SDK)\lib\win64 -lopengl32 -lglew32 -llibvorbisfile -llibpng16d -lsqlited -lzlibstatic -lOpenAL32
        DEPENDPATH += ../Engine/debug
        DEPENDPATH += $$(AL_SDK)/lib/win64
        DEPENDPATH += $$(COMMON_SDK)/lib/win64
        DEFINES += _DEBUG
    } else {
        LIBS += -L../Engine/release -L$$(AL_SDK)/lib/win64 -L$$(COMMON_SDK)/lib/win64 -L$$(GL_SDK)\lib\win64 -lopengl32 -lglew32 -llibvorbisfile -llibpng16 -lsqlite -lzlibstatic -lOpenAL32
        DEPENDPATH += ../Engine/release
        DEPENDPATH += $$(AL_SDK)/lib/win64
        DEPENDPATH += $$(COMMON_SDK)/lib/win64
    }

    DEPENDPATH += ../Bin64
    INCLUDEPATH += $$(GL_SDK)\include
    INCLUDEPATH += $$(AL_SDK)\include
    DEFINES += NOMINMAX
    DEFINES += _CRT_SECURE_NO_WARNINGS
    RC_FILE = TestGame.rc
}

LIBS += -lFarrahEngine

SOURCES += TestGame.cpp
HEADERS += TestGame.h
