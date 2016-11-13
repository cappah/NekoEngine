#-------------------------------------------------
#
# Project created by QtCreator 2016-10-06T14:41:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ModelImporter
TEMPLATE = app

INCLUDEPATH += ../../3rdparty/include
INCLUDEPATH += ../../Include
INCLUDEPATH += $$(FBX_SDK_PATH)/include
DEPENDPATH += ../../3rdparty/lib
DEPENDPATH += $$(FBX_SDK_LIBPATH)
LIBS += -L../../3rdparty/lib
LIBS += -L$$(FBX_SDK_LIBPATH)

LIBS += -lassimp

win32 {
    LIBS += -lzlibstatic -llibfbxsdk
}

macx {
    LIBS += -L/usr/local/lib
}

unix {
    LIBS += -lz
}

SOURCES += main.cpp\
        ModelImporterWindow.cpp \
    StaticMesh.cpp \
    AboutDialog.cpp \
    SkeletalMesh.cpp \
    AssimpConverter.cpp \
    Material.cpp \
    AnimationClip.cpp \
    FBXConverter.cpp

HEADERS  += \
    ModelImporterWindow.h \
    StaticMesh.h \
    AboutDialog.h \
    SkeletalMesh.h \
    AssimpConverter.h \
    Material.h \
    AnimationClip.h \
    FBXConverter.h

FORMS    += ModelImporterWindow.ui \
    AboutDialog.ui
