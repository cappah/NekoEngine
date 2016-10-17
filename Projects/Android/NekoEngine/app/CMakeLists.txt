cmake_minimum_required(VERSION 3.4.1)
project (NekoEngine)

file(GLOB EngineSourceFiles "../../../../Source/Engine/*.cpp"
	"../../../../Source/Engine/Audio/*.cpp"
	"../../../../Source/Engine/Core/*.cpp"
	"../../../../Source/Engine/PostEffects/*.cpp"
	"../../../../Source/Engine/Scene/*.cpp"
	"../../../../Source/Engine/Scene/Components/*.cpp"
	"../../../../Source/Engine/System/*.cpp"
	"../../../../Source/Engine/System/AssetLoader/*.cpp"
	"../../../../Source/Engine/System/VFS/*.cpp"
	"../../../../Source/Engine/Platform/*.cpp"
	"../../../../Source/Engine/Platform/Compat/*.cpp"
	"../../../../Source/Engine/Platform/UNIX/*.cpp"
    "../../../../Source/Engine/Platform/Android/*.cpp"
)

file(GLOB LauncherSourceFiles
	src/main/cpp/launcher.cpp
)

file(GLOB GLESRendererSourceFiles "../../../../Source/Renderer/GLESRenderer/*.cpp"
	"../../../../Source/Renderer/GLRenderer/Loaders/tga.cpp"
	"../../../../Source/Renderer/GLESRenderer/Platform/EGL/*.cpp"
)

file(GLOB TestGameSourceFiles "../../../../Source/TestGame/*.cpp")

include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${PROJECT_SOURCE_DIR}/../../../../Include)
include_directories(${PROJECT_SOURCE_DIR}/../../../../3rdparty/include_android)
include_directories(${PROJECT_SOURCE_DIR}/../../../../Source/Renderer/GLRenderer)

find_library(log-lib log)

add_library(app-glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
add_library(sqlite3 STATIC src/main/cpp/sqlite3.c)

add_library(TestGame STATIC ${TestGameSourceFiles})
target_compile_options(TestGame PRIVATE -std=c++11)
target_compile_options(TestGame PRIVATE -fPIC)
target_compile_options(TestGame PRIVATE -frtti)
target_compile_options(TestGame PRIVATE -DTESTGAME_INTERNAL)

add_library(GLESRenderer STATIC ${GLESRendererSourceFiles})
target_compile_options(GLESRenderer PRIVATE -std=c++11)
target_compile_options(GLESRenderer PRIVATE -fPIC)
target_compile_options(GLESRenderer PRIVATE -frtti)
target_link_libraries(GLESRenderer EGL GLESv2)

add_library(Engine SHARED ${EngineSourceFiles} ${PlatformSourceFiles} ${SharedPlatformSourceFiles})
target_compile_options(Engine PRIVATE -std=c++11)
target_compile_options(Engine PRIVATE -fPIC)
target_compile_options(Engine PRIVATE -frtti)
target_compile_options(Engine PRIVATE -DENGINE_INTERNAL)
target_compile_options(Engine PRIVATE -DPLATFORM_INTERNAL)
target_link_libraries(Engine GLESRenderer TestGame app-glue z sqlite3 freetype ${log-lib})

add_library(launcher SHARED ${LauncherSourceFiles})
target_compile_options(Engine PRIVATE -std=c++11)

target_link_libraries(launcher ${log-lib})
