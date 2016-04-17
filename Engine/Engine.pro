#-------------------------------------------------
#
# Project created by QtCreator 2016-02-11T04:48:06
#
#-------------------------------------------------

CONFIG += c++11

TARGET = FarrahEngine
TEMPLATE = lib

DEFINES += USE_OPENGL
DEFINES += EDITOR

INCLUDEPATH += ../Engine
INCLUDEPATH += ../Utilities

win32 {
    CONFIG(debug, debug|release) {
        LIBS += -L../Utilities/debug -L$$(AL_SDK)/lib/win64 -L$$(COMMON_SDK)/lib/win64 -L$$(GL_SDK)\lib\win64 -lopengl32 -lglew32 -llibvorbisfile -llibpng16d -lsqlited -lzlibstatic -lOpenAL32
        DEPENDPATH += ../Utilities/debug
        DEPENDPATH += $$(AL_SDK)/lib/win64
        DEPENDPATH += $$(COMMON_SDK)/lib/win64
        DEFINES += _DEBUG
    } else {
        LIBS += -L../Utilities/release -L$$(AL_SDK)/lib/win64 -L$$(COMMON_SDK)/lib/win64 -L$$(GL_SDK)\lib\win64 -lopengl32 -lglew32 -llibvorbisfile -llibpng16 -lsqlite -lzlibstatic -lOpenAL32
        DEPENDPATH += ../Utilities/release
        DEPENDPATH += $$(AL_SDK)/lib/win64
        DEPENDPATH += $$(COMMON_SDK)/lib/win64
    }

    INCLUDEPATH += $$(GL_SDK)\include
    INCLUDEPATH += $$(AL_SDK)\include
    INCLUDEPATH += $$(COMMON_SDK)\include
    DEFINES += NOMINMAX
    DEFINES += _CRT_SECURE_NO_WARNINGS
    RC_FILE = Engine.rc
}

LIBS += -lFarrahUtilities

SOURCES += AssetLoader.cpp \
    AudioClip.cpp \
    AudioSource.cpp \
    Camera.cpp \
    Effect.cpp \
    Engine.cpp \
    FireObject.cpp \
    GBuffer.cpp \
    GLExt.cpp \
    ktx_func.cpp \
    Light.cpp \
    LoadingScreen.cpp \
    Logger.cpp \
    Material.cpp \
    Model.cpp \
    MovingObject.cpp \
    nv_dds.cpp \
    Object.cpp \
    ogg_callbacks.cpp \
    PostProcessor.cpp \
    ResourceDatabase.cpp \
    ResourceManager.cpp \
    Scene.cpp \
    SceneManager.cpp \
    Shader.cpp \
    Skybox.cpp \
    SMAA.cpp \
    SoundManager.cpp \
    SSAO.cpp \
    Terrain.cpp \
    Texture.cpp \
    TextureFont.cpp \
    tga.cpp \
    VFS.cpp \
    VFSArchive.cpp \
    VFSFile.cpp

HEADERS  += AssetLoader.h \
    AudioClip.h \
    AudioClipResource.h \
    AudioSource.h \
    Camera.h \
    Effect.h \
    Engine.h \
    EngineAPI.h \
    EngineClassFactory.h \
    EngineUtils.h \
    EngineVersion.h \
    FireObject.h \
    GameModule.h \
    GBuffer.h \
    GLExt.h \
    ktx.h \
    ktxint.h \
    Light.h \
    LoadingScreen.h \
    Logger.h \
    Material.h \
    MaterialResource.h \
    Model.h \
    ModelResource.h \
    MovingObject.h \
    nv_dds.h \
    Object.h \
    ogg_callbacks.h \
    PostProcessor.h \
    Resource.h \
    ResourceDatabase.h \
    ResourceInfo.h \
    ResourceManager.h \
    Scene.h \
    SceneManager.h \
    Shader.h \
    ShaderResource.h \
    Skybox.h \
    SMAA.h \
    SoundManager.h \
    SSAO.h \
    Terrain.h \
    Texture.h \
    TextureFont.h \
    TextureFontResource.h \
    TextureResource.h \
    tga.h \
    Vertex.h \
    VFS.h \
    VFSArchive.h \
    VFSFile.h \
    SMAA/AreaTex.h \
    SMAA/SearchTex.h
