cmake_minimum_required (VERSION 3.0)
project (NekoEngine)

include(CheckIncludeFiles)

if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE, DEBUG)
endif(NOT CMAKE_BUILD_TYPE)

file(GLOB EngineSourceFiles "Engine/*.cpp"
	"Engine/Audio/*.cpp"
	"Engine/Core/*.cpp"
	"Engine/PostEffects/*.cpp"
	"Engine/Scene/*.cpp"
	"Engine/Scene/Components/*.cpp"
	"Engine/System/*.cpp"
	"Engine/System/AssetLoader/*.cpp"
	"Engine/System/VFS/*.cpp"
	"Engine/Platform/*.cpp"
	"Engine/Platform/Compat/*.cpp"
)
file(GLOB TestGameSourceFiles "TestGame/*.cpp")
file(GLOB RendererSourceFiles "GLRenderer/*.cpp"
	"GLRenderer/Loaders/*.cpp"
)

set(OPENAL_LIB openal)
set(BSD_LIB "")

include_directories(/usr/local/include)
include_directories(${PROJECT_SOURCE_DIR}/Include)
include_directories(${PROJECT_SOURCE_DIR}/3rdparty/include_all)
link_directories(/usr/local/lib)

if(NOT MSVC)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g2 -D_DEBUG -fno-exceptions -Wall -O0")
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
	file(GLOB ExecutableSourceFile "Launcher/Windows/*.cpp")
	file(GLOB RendererPlatformSourceFiles "GL4Renderer/Platform/Windows/*.cpp")
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
elseif(${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
	set(PLATFORM_UNIX ON)
	set(PLATFORM_X11 ON)

	include_directories(/usr/X11R6/include)
	link_directories(/usr/X11R6/lib)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	file(GLOB ExecutableSourceFile "Launcher/Mac/*.mm")
	file(GLOB PlatformSourceFiles "Engine/Platform/Mac/*.mm")
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
	file(GLOB SharedPlatformSourceFiles "Engine/Platform/UNIX/*.cpp")
endif(PLATFORM_UNIX)

if(PLATFORM_X11)
	file(GLOB PlatformSourceFiles "Engine/Platform/X11/*.cpp")
	file(GLOB ExecutableSourceFile "Launcher/X11/*.cpp")
	file(GLOB RendererPlatformSourceFiles "GLRenderer/Platform/X11/*.cpp")
	set(PLATFORM_LIBS "X11")
	add_definitions(-DPLATFORM_X11)
endif(PLATFORM_X11)

# OpenGL Renderer Library
if(NOT PLATFORM_MAC)
	add_library(GLRenderer SHARED ${RendererSourceFiles} ${RendererPlatformSourceFiles})
	target_compile_options(GLRenderer PRIVATE -std=c++11)
	target_compile_options(GLRenderer PRIVATE -fPIC)
	target_compile_options(GLRenderer PRIVATE -frtti)
	target_link_libraries(GLRenderer GL X11)
endif(NOT PLATFORM_MAC)

# Engine Library
add_library(Engine SHARED ${EngineSourceFiles} ${PlatformSourceFiles} ${SharedPlatformSourceFiles})
target_compile_options(Engine PRIVATE -std=c++11)
target_compile_options(Engine PRIVATE -fPIC)
target_compile_options(Engine PRIVATE -frtti)
target_link_libraries(Engine ${PLATFORM_LIBS} ${OPENAL_LIB} z sqlite3 png vorbisfile ${BSD_LIB})

# Launcher Executable
add_executable(Launcher ${ExecutableSourceFile})
target_compile_options(Launcher PRIVATE -std=c++11)
target_compile_options(Launcher PRIVATE -frtti)
target_link_libraries(Launcher Engine)

# Test Game Module
include_directories(Engine)

add_library(TestGame SHARED ${TestGameSourceFiles})
target_compile_options(TestGame PRIVATE -std=c++11)
target_compile_options(TestGame PRIVATE -fPIC)
target_compile_options(TestGame PRIVATE -frtti)
target_link_libraries(TestGame Engine)
