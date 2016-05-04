/* Neko Engine
 *
 * Engine.h
 * Author: Alexandru Naiman
 *
 * Engine class definition 
 *
 * ----------------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman <alexandru dot naiman at icloud dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ALEXANDRU NAIMAN
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <Platform/Platform.h>

#ifdef ENGINE_INTERNAL
	#ifdef NE_PLATFORM_MAC
		#include <OpenAL/al.h>
		#include <OpenAL/alc.h>
	#else
		#include <AL/al.h>
		#include <AL/alc.h>
	#endif
#endif

#ifdef NE_PLATFORM_WINDOWS
	#ifdef ENGINE_INTERNAL
		#define ENGINE_API	__declspec(dllexport)
	#else
		#define ENGINE_API	__declspec(dllimport)
	#endif
#else
	#define ENGINE_API
#endif

#include <map>
#include <string>
#include <vector>
#include <chrono>
#include <stdint.h>

#include <glm/glm.hpp>

#include <Renderer/Renderer.h>
#include <Engine/EngineUtils.h>
#include <Engine/Version.h>
#include <Engine/EngineClassFactory.h>
#include <System/Logger.h>

#define ENGINE_OK			 0
#define ENGINE_FAIL			-1
#define ENGINE_LOAD_VS_FAIL		-2
#define ENGINE_LOAD_FS_FAIL		-3
#define ENGINE_LOAD_SHADER_FAIL 	-4
#define ENGINE_ATTR_MISSING		-5
#define ENGINE_UNIF_MISSING		-6
#define ENGINE_NO_SCENE			-7
#define ENGINE_NOT_FOUND		-8
#define ENGINE_IN_USE			-9
#define ENGINE_INVALID_RES		-10
#define ENGINE_IO_FAIL			-11
#define ENGINE_NO_CAMERA		-12
#define ENGINE_INVALID_ARGS		-13
#define ENGINE_NOT_LOADED		-14
#define ENGINE_MEM_FAIL			-15
#define ENGINE_LOAD_GS_FAIL		-16
#define ENGINE_OUT_OF_RESOURCES	-17

#define KEY_ESCAPE			0x1B
#define KEY_LEFTARROW			0x25
#define KEY_UPARROW			0x26
#define KEY_RIGHTARROW			0x27
#define KEY_DOWNARROW			0x28
#define KEY_LSHIFT			0x10

#define PATH_SIZE			1024

#define RENDER_QUALITY_LOW		0
#define RENDER_QUALITY_HIGH		1

#define RENDER_SSAO_LOW			0
#define RENDER_SSAO_MEDIUM		1
#define RENDER_SSAO_HIGH		2

#define RENDER_TEX_Q_LOW		0
#define RENDER_TEX_Q_MED		1
#define RENDER_TEX_Q_HIGH		2

#ifdef _DEBUG
/**
 * Check glGetError() log a critical message and close the program if an error has occured
 */
#define GL_CHECK(x)																														\
	x;																																			\
	if(GLenum err = glGetError())																												\
	{																																			\
		Logger::Log("OpenGL", LOG_CRITICAL, "%s call from %s, line %d returned 0x%x. Shutting down.", #x, __FILE__, __LINE__, err);				\
		Platform::MessageBox("Fatal Error", "OpenGL call failed. Please check the log file for details.\nThe program will now exit.", MessageBoxButtons::OK, MessageBoxIcon::Error); \
		exit(-1);																																\
	}

/**
 * Check alGetError() and log a warning message if an error has occured
 */
#define AL_CHECK(x)																									\
	x;																												\
	if(ALenum err = alGetError())																					\
		Logger::Log("OpenAL", LOG_WARNING, "%s call from %s, line %d returned 0x%x", #x, __FILE__, __LINE__, err)

/**
 * Check alGetError() and log a warning message if an error has occured
 * The message will be saved in the logger queue. Use this function for checking errors in Draw().
 * The logger queue will be written to the file at the end of the frame
 */
#define AL_CHECK_QUEUE(x)																											\
	x;																																\
	if(ALenum err = alGetError())																									\
		Logger::EnqueueLogMessage("OpenAL", LOG_WARNING, "%s call from %s, line %d returned 0x%x", #x, __FILE__, __LINE__, err)

/**
 * Check alGetError() log a critical message and close the program if an error has occured
 */
#define AL_CHECK_FATAL(x)																														\
	x;																																			\
	if(ALenum err = alGetError())																												\
	{																																			\
		Logger::Log("OpenAL", LOG_CRITICAL, "%s call from %s, line %d returned 0x%x. Shutting down.", #x, __FILE__, __LINE__, err);				\
		Platform::MessageBox("Fatal Error", "OpenAL call failed. Please check the log file for details.\nThe program will now exit.", MessageBoxButtons::OK, MessageBoxIcon::Error); \
		exit(-1);																																\
	}

/**
 * Check alGetError() log a critical message and return from the current function with the specified exit code if an error has occured
 */
#define AL_CHECK_RET(x, y)																								\
	x;																													\
	if(ALenum err = alGetError())																						\
	{																													\
		Logger::Log("OpenAL", LOG_CRITICAL, "%s call from %s, line %d returned 0x%x.", #x, __FILE__, __LINE__, err);	\
		return y;																										\
	}
#else
#define GL_CHECK(x) x;
#define AL_CHECK(x) x;
#define AL_CHECK_QUEUE(x) x;
#define AL_CHECK_FATAL(x) x;
#define AL_CHECK_RET(x, y) x;
#endif

#define DIE(x)											\
	Platform::MessageBox("Fatal Error", x, MessageBoxButtons::OK, MessageBoxIcon::Error);		\
	exit(-1);

/**
 * Engine configuration information
 */
struct EngineConfig
{
	int ScreenHeight;
	int ScreenWidth;
	bool Fullscreen;
	bool LoadLooseFiles;
	char DataDirectory[PATH_SIZE];
	char LogFile[PATH_SIZE];
};

/**
 * Renderer configuration information
 */
struct RendererConfig
{
	int Quality;
	bool Supersampling;
	bool Mipmaps;
	bool Anisotropic;
	int Aniso;
	bool VerticalSync;
	bool SSAO;
	int SSAOQuality;
	bool Multisampling;
	int Samples;
	int TextureQuality;
	int ShadowMapSize;
};

/**
 * Post processor configuration information
 */
struct PostProcessorConfig
{
	bool Bloom;
	bool SMAA;
	bool FXAA;
};

/**
 * Configuration information
 */
struct Configuration
{
	EngineConfig Engine;
	RendererConfig Renderer;
	PostProcessorConfig PostProcessor;
};

class GameModule;
class TextureFont;

/**
 * Neko Engine
 */
class ENGINE_API Engine
{
public:
	/**
	 * Must be called on program startup, before any other Engine functions
	 */
	static int Initialize(std::string cmdLine, bool editor);

	/**
	 * Main engine loop. Call this after the initialization.
	 * This function will return after the Engine has been shutdown.
	 */
	static int Run();

	/**
	 * Run one frame
	 */
	static void Frame() noexcept;

	/**
	 * Draw the loaded scene
	 */
	static void Draw() noexcept;

	/**
	 * Update the loaded scene. Called once per frame.
	 */
	static void Update(float deltaTime) noexcept;

	/**
	 * Called when a key is pressed or released.
	 */
	static void Key(int key, bool bIsPressed) noexcept;
	
	/**
	 * Release all resources used by the Engine.
	 * Must be called on program exit.
	 */
	static void CleanUp() noexcept;

	static void DrawString(glm::vec2 pos, glm::vec3 color, std::string text) noexcept;
	static void DrawString(glm::vec2 pos, glm::vec3 color, const char *fmt, ...) noexcept;

	static bool GetKeyDown(int key) noexcept;

	static double GetTime() noexcept;

	static int GetScreenHeight() noexcept { return _config.Engine.ScreenHeight; }
	static int GetScreenWidth() noexcept { return _config.Engine.ScreenWidth; }
	static void ScreenResized(int width, int height) noexcept;

	static GameModule *GetGameModule() noexcept { return _gameModule; }

	static Object *NewObject(const std::string &className);

	static Renderer *GetRenderer() noexcept { return _renderer; }

	static void SaveScreenshot() noexcept;

#ifdef ENGINE_INTERNAL
	/**
	 * Get the engine configuration information
	 */
	static Configuration &GetConfiguration() noexcept { return _config; }
	
	static void BindQuadVAO() noexcept { _quadVAO->Bind(); };

	/**
	 * Swap buffers
	 */
	static void SwapBuffers() noexcept { _renderer->SwapBuffers(); }

#ifdef _DEBUG
	static unsigned long _drawCalls;
#endif
#endif

private:
	static PlatformWindowType _engineWindow;
	static Configuration _config;
	static std::vector<int> *_pressedKeys;
	static bool _disposed;
	static bool _printStats;
	static TextureFont *_engineFont;
	static GameModule *_gameModule;
	static Renderer *_renderer;

#ifdef ENGINE_INTERNAL
	static int _nFrames;
	static double _lastTime;
	static int _fps;
	static PlatformModuleType _gameModuleLibrary;
	static PlatformModuleType _rendererLibrary;
	static char _gameModuleFile[PATH_SIZE];
	static char _rendererFile[PATH_SIZE];
	static bool _graphicsDebug;
	static RArrayBuffer* _quadVAO;
	static RBuffer* _quadVBO;
	static std::chrono::high_resolution_clock::time_point _prevTime;
	static bool _haveMemoryInfo;
	static bool _startup;
#endif

	static void _ParseArgs(std::string cmdLine);
	static void _ReadINIFile(const char *file);
	static void _ReadEffectConfig(const char *file);
	static void _InitializeQuadVAO();
	static bool _InitRenderer();
	static bool _InitSystem();
	static bool _InitGame();
};
