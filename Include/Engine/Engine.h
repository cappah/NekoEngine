/* NekoEngine
 *
 * Engine.h
 * Author: Alexandru Naiman
 *
 * Engine class definition
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2017, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <Engine/Defs.h>

#include <stdint.h>

#include <Engine/Vertex.h>
#include <Engine/EngineClassFactory.h>
#include <Renderer/Renderer.h>
#include <Platform/Platform.h>
#include <Runtime/Runtime.h>

#define DIE(x)																					\
	Platform::MessageBox("Fatal Error", x, MessageBoxButtons::OK, MessageBoxIcon::Error);		\
	exit(-1);

 /**
 * Engine configuration information
 */
struct EngineConfig
{
	uint32_t ScreenHeight;
	uint32_t ScreenWidth;
	bool Fullscreen;
	bool LoadLooseFiles;
	bool EnableConsole;
	char DataDirectory[NE_PATH_SIZE];
	char LogFile[NE_PATH_SIZE];
};

/**
 * Renderer configuration information
 */
struct RendererConfig
{
	bool Supersampling;
	bool Multisampling;
	int Samples;

	int TextureQuality;
	bool Anisotropic;
	int Aniso;

	int MaxLights;
	int ShadowMapSize;
	int MaxShadowMaps;
	bool ShadowMultisampling;
	int ShadowSamples;

	bool VerticalSync;

	bool EnableAsyncCompute;

	float Gamma;

	bool UseDeviceGroup;

	struct
	{
		bool Enable;
		int KernelSize;
		float Radius;
		float PowerExponent;
		float Threshold;
		float Bias;
		bool Multisampling;
	} SSAO;
};

/**
 * Post processor configuration information
 */
struct PostProcessorConfig
{
	bool Bloom;
	int BloomIntensity;
	bool DepthOfField;
	bool FilmGrain;
};

/**
 * Audio system configuration information
 */
struct AudioConfig
{
	float MasterVolume;
	float EffectsVolume;
	float MusicVolume;
};

/**
 * Configuration information
 */
struct Configuration
{
	EngineConfig Engine;
	RendererConfig Renderer;
	PostProcessorConfig PostProcessor;
	AudioConfig Audio;
};

/**
 * Debug variables
 */
struct DebugVariables
{
	bool DrawBounds;
};

class ENGINE_API Engine
{
public:
	static int Initialize(const char *cmdLine, bool editor);

	static bool IsEditor() { return /*_editor*/false; }
	static bool IsPaused() { return _paused; }
	static bool StatsVisible() { return _drawStats; }

	static void ToggleStats() { _drawStats = !_drawStats; }
	static void TogglePause() { _paused = !_paused; }

	static void Frame() noexcept;

	static int Run();
	
	static uint32_t GetScreenWidth() noexcept { return (uint32_t)(_config.Engine.ScreenWidth * _scaleFactor.x); }
	static uint32_t GetScreenHeight() noexcept { return (uint32_t)(_config.Engine.ScreenHeight * _scaleFactor.y); }
	static void ScreenResized(int width, int height) noexcept;

	static double GetTime() noexcept;

	static Object *NewObject(const std::string &className, ObjectInitializer *initializer = nullptr);
	static ObjectComponent *NewComponent(const std::string &className, ComponentInitializer *initializer);

	static void CleanUp() noexcept;

	static void Exit() { Platform::Exit(); }
	static void Restart() { Platform::Restart(); }

	/**
	* Get the engine configuration information
	*/
	static Configuration &GetConfiguration() noexcept { return _config; }
	static bool SaveConfiguration() noexcept;

#ifdef ENGINE_INTERNAL
	/**
	 * Get the engine debug variables
	 */
	static DebugVariables &GetDebugVariables() noexcept { return _debugVariables; }

	static GameModule *GetGameModule() noexcept { return _gameModule; }
#endif

private:
	static Configuration _config;
	static DebugVariables _debugVariables;
	static GameModule *_gameModule;
	static PlatformModuleType _gameModuleLibrary;
	static bool _iniFileLoaded;
	static bool _editor;
	static bool _paused;
	static bool _disposed;
	static bool _drawStats;
	static bool _startup;
	static glm::vec2 _scaleFactor;

	static void _FixedUpdate();
	static void _Update(double deltaTime);
	static void _Draw();
	static void _DrawStats();

	static void _ParseArgs(NString &cmdLine);
	static void _ReadINIFile(const char *file);
	static void _ReadEffectConfig(const char *file);
	static void _ReadInputConfig(const char *file);
	static void _ReadRendererConfig(const char *file);

	static int _InitSystem();
	static bool _InitGame();
};

#if defined(_MSC_VER)
// GLM
template struct ENGINE_API glm::tvec2<float, glm::highp>;
template struct ENGINE_API glm::tvec3<float, glm::highp>;
template struct ENGINE_API glm::tvec4<float, glm::highp>;
template struct ENGINE_API glm::tmat4x4<float, glm::highp>;

// Base types
template class ENGINE_API NArray<char>;
template class ENGINE_API NArray<short>;
template class ENGINE_API NArray<int>;
template class ENGINE_API NArray<unsigned int>;
template class ENGINE_API NArray<long>;
template class ENGINE_API NArray<unsigned long>;
template class ENGINE_API NArray<long long>;
template class ENGINE_API NArray<unsigned long long>;
template class ENGINE_API NArray<float>;
template class ENGINE_API NArray<double>;

// Runtime types
template class ENGINE_API NArray<NString>;

// Engine types
template class ENGINE_API NArray<Vertex>;

template class ENGINE_API NArray<std::string>;
#endif
