cmake_minimum_required (VERSION 3.0)
project (NekoEngine)

include(CheckIncludeFiles)

if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE, DEBUG)
endif(NOT CMAKE_BUILD_TYPE)

file(GLOB EngineSourceFiles "Source/Engine/*.cpp"
	"Source/Engine/Animation/*.cpp"
	"Source/Engine/Audio/*.cpp"
	"Source/Engine/Core/*.cpp"
	"Source/Engine/PostEffects/*.cpp"
	"Source/Engine/Scene/*.cpp"
	"Source/Engine/Scene/Components/*.cpp"
	"Source/Engine/System/*.cpp"
	"Source/Engine/System/AssetLoader/*.cpp"
	"Source/Engine/System/VFS/*.cpp"
	"Source/Engine/Platform/*.cpp"
	"Source/Engine/Platform/Compat/*.cpp"
	"Source/Engine/Renderer/*.cpp"
	"Source/Engine/Script/*.cpp"
	"Source/Engine/Script/Interface/*.cpp"
)
file(GLOB TestGameSourceFiles "Source/TestGame/*.cpp")

set(OPENAL_LIB openal)
set(BSD_LIB "")

include_directories(/usr/local/include)
include_directories(${PROJECT_SOURCE_DIR}/Include)
include_directories(${PROJECT_SOURCE_DIR}/Source/Include)
include_directories(${PROJECT_SOURCE_DIR}/3rdparty/include_all)
include_directories(/usr/include/freetype2)
include_directories(/usr/local/include/freetype2)
link_directories(/usr/local/lib)

if(NOT MSVC)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g2 -DNE_CONFIG_DEBUG -fno-exceptions -Wall -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -fno-math-errno -fomit-frame-pointer -funroll-loops -ffast-math -fno-exceptions -flto -Wall -Werror")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g -Og -fno-exceptions")
else(NOT MSVC)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /nologo /DNOMINMAX /D_CRT_SECURE_NO_WARNINGS /D_DEBUG /GS /Zi /debug")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /nologo /DNOMINMAX /D_CRT_SECURE_NO_WARNINGS /GS /GL /Gm /O2 /Od /Ox")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /nologo /GS /GL /Gm /O2 /Ox /Z7")
endif(NOT MSVC)

if(CMAKE_BUILD_TYPE EQUAL "RELEASE")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto")
endif(CMAKE_BUILD_TYPE EQUAL "RELEASE")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	file(GLOB ExecutableSourceFile "Source/Launcher/Windows/*.cpp")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(PLATFORM_UNIX ON)
	set(PLATFORM_X11 ON)

	set(BSD_LIB bsd)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
	set(PLATFORM_UNIX ON)
	set(PLATFORM_X11 ON)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
	set(PLATFORM_UNIX ON)
	set(PLATFORM_X11 ON)

	include_directories(/opt/csw/include/freetype2)
	link_directories(/opt/csw/lib)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
	set(PLATFORM_UNIX ON)
	set(PLATFORM_X11 ON)

	include_directories(/usr/X11R6/include)
	link_directories(/usr/X11R6/lib)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	file(GLOB ExecutableSourceFile "Source/Launcher/Mac/*.mm")
	file(GLOB PlatformSourceFiles "Source/Engine/Platform/Mac/*.mm")
	set(PLATFORM_UNIX ON)
	set(PLATFORM_MAC ON)
	set(OPENAL_LIB "")

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ObjC++")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ObjC++")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -ObjC++")

	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -framework Cocoa")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -framework Carbon -framework OpenAL -framework Cocoa -ObjC++")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if(PLATFORM_UNIX)
	file(GLOB SharedPlatformSourceFiles "Source/Engine/Platform/UNIX/*.cpp")
endif(PLATFORM_UNIX)

if(PLATFORM_X11)
	file(GLOB PlatformSourceFiles "Source/Engine/Platform/X11/*.cpp")
	file(GLOB ExecutableSourceFile "Source/Launcher/X11/*.cpp")
	set(PLATFORM_LIBS "X11")
	add_definitions(-DPLATFORM_X11)
endif(PLATFORM_X11)

if(PLATFORM_WAYLAND)
	file(GLOB PlatformSourceFiles "Source/Engine/Platform/Wayland/*.cpp")
	file(GLOB ExecutableSourceFile "Source/Launcher/X11/*.cpp")
	set(PLATFORM_LIBS "wayland-client")
	add_definitions(-DPLATFORM_WAYLAND)
endif(PLATFORM_WAYLAND)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Bin64")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Bin64")

# Engine Library
add_library(Engine SHARED ${EngineSourceFiles} ${PlatformSourceFiles} ${SharedPlatformSourceFiles})
target_compile_options(Engine PRIVATE -std=c++1z)
target_compile_options(Engine PRIVATE -fPIC)
target_compile_options(Engine PRIVATE -frtti)
target_compile_options(Engine PRIVATE -DENGINE_INTERNAL)
target_compile_options(Engine PRIVATE -DPLATFORM_INTERNAL)
target_link_libraries(Engine X11 ${PLATFORM_LIBS} ${OPENAL_LIB} z sqlite3 png freetype vulkan luajit-5.1 ${BSD_LIB})

add_custom_command(TARGET Engine POST_BUILD COMMAND mkdir -p ${PROJECT_SOURCE_DIR}/Resources/Data/Shaders)
add_custom_command(TARGET Engine POST_BUILD COMMAND ${PROJECT_SOURCE_DIR}/Source/Shaders/compile.sh WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Source/Shaders)

# Launcher Executable
add_executable(Launcher ${ExecutableSourceFile})
target_compile_options(Launcher PRIVATE -std=c++1z)
target_compile_options(Launcher PRIVATE -frtti)
target_link_libraries(Launcher Engine)

add_custom_command(TARGET Launcher POST_BUILD COMMAND cp ${PROJECT_SOURCE_DIR}/Config/Engine.ini ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Engine.ini)

# Test Game Module
include_directories(Engine)

add_library(TestGame SHARED ${TestGameSourceFiles})
target_compile_options(TestGame PRIVATE -std=c++1z)
target_compile_options(TestGame PRIVATE -fPIC)
target_compile_options(TestGame PRIVATE -frtti)
target_compile_options(Engine PRIVATE -DTESTGAME_INTERNAL)
target_link_libraries(TestGame Engine)
