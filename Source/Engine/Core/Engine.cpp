/* NekoEngine
 *
 * Engine.cpp
 * Author: Alexandru Naiman
 *
 * Engine class implementation
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

#include <chrono>

#include <GUI/GUI.h>
#include <Input/Input.h>
#include <Engine/Debug.h>
#include <Engine/Engine.h>
#include <Engine/Events.h>
#include <Engine/Version.h>
#include <Engine/Console.h>
#include <Engine/GameModule.h>
#include <Engine/SoundManager.h>
#include <Engine/EventManager.h>
#include <Engine/ResourceManager.h>
#include <Scene/SceneManager.h>
#include <Audio/AudioSystem.h>
#include <System/Logger.h>
#include <System/VFS/VFS.h>
#include <Renderer/SSAO.h>
#include <Renderer/Renderer.h>
#include <Renderer/PostProcessor.h>
#include <Profiler/Profiler.h>
#include <Physics/Physics.h>
#include <Platform/CrashHandler.h>

 // 60 Hz logic update
#define UPDATE_DELTA			.01666
#define VFS_ARCHIVE_LIST_SIZE	4096

#define ENGINE_MODULE			"Engine"

using namespace glm;
using namespace std;
using namespace std::chrono;

Configuration Engine::_config;
DebugVariables Engine::_debugVariables
{
	false
};
GameModule *Engine::_gameModule = nullptr;
PlatformModuleType Engine::_gameModuleLibrary = nullptr;
bool Engine::_iniFileLoaded = false;
bool Engine::_paused = false;
bool Engine::_editor = false;
bool Engine::_disposed = false;
bool Engine::_drawStats = false;
bool Engine::_startup = true;
vec2 Engine::_scaleFactor{ 1.f,1.f };

high_resolution_clock::time_point _prevTime;
PlatformWindowType _engineWindow = 0;
bool _graphicsDebug = false;
char *_configFilePath{ nullptr };
char _gameModuleFile[NE_PATH_SIZE]{ '\0' };
char _physicsModuleFile[NE_PATH_SIZE]{ '\0' };
char _audioSystemModuleFile[NE_PATH_SIZE]{ '\0' };
NString _vfsArchiveList(VFS_ARCHIVE_LIST_SIZE);

static int _fps = 0;
static double _frameTime = 0.0;

ObjectClassMapType *EngineClassFactory::_objectClassMap = nullptr;
ComponentClassMapType *EngineClassFactory::_componentClassMap = nullptr;

char *ne_executable_name;

int Engine::Run()
{
	if (SceneManager::LoadDefaultScene() != ENGINE_OK)
	{
		Platform::MessageBox("Fatal Error", "Failed to load the default scene. The application will now exit.", MessageBoxButtons::OK, MessageBoxIcon::Error);
		CleanUp();
		exit(0);
	}
	
	return Platform::MainLoop();
}

void Engine::Frame() noexcept
{
	static double lastTime = GetTime(), lastFPSTime = GetTime(), nextFixedUpdateTime = GetTime();
	static int nFrames = 0;
	const double curTime = GetTime();
	const double deltaTime = curTime - lastTime;
	const double deltaFPSTime = curTime - lastFPSTime;

	++nFrames;
	
	if (deltaFPSTime > 1.f)
	{
		_fps = nFrames;
		_frameTime = (deltaFPSTime / (double)_fps) * 1000;
		lastFPSTime += deltaFPSTime;
		nFrames = 0;
	}
	
	_Update(deltaTime);
	lastTime = curTime;

	if (curTime > nextFixedUpdateTime)
	{
		_FixedUpdate();
		nextFixedUpdateTime += UPDATE_DELTA;
	}

	_Draw();

#if defined(NE_CONFIG_DEBUG) || defined(NE_CONFIG_DEVELOPMENT)
	Logger::Flush();
#endif
}

void Engine::ScreenResized(int width, int height) noexcept
{
	if (_startup)
		return;

	_config.Engine.ScreenWidth = width;
	_config.Engine.ScreenHeight = height;

	Renderer::GetInstance()->ScreenResized();
	GUIManager::ScreenResized();
}

double Engine::GetTime() noexcept
{
	const high_resolution_clock::time_point time = high_resolution_clock::now();
	high_resolution_clock::duration diff = time - _prevTime;

	return (double)diff.count() * high_resolution_clock::period::num / high_resolution_clock::period::den;
}

Object *Engine::NewObject(const std::string &className, ObjectInitializer *initializer)
{
	Object *obj = EngineClassFactory::NewObject(className, initializer);

	if (obj)
		return obj;

	if (_gameModule)
		return _gameModule->NewObject(className, initializer);

	return nullptr;
}

ObjectComponent *Engine::NewComponent(const std::string &className, ComponentInitializer *initializer)
{
	ObjectComponent *obj = EngineClassFactory::NewComponent(className, initializer);

	if (obj)
		return obj;

	if (_gameModule)
		return _gameModule->NewComponent(className, initializer);

	return nullptr;
}

bool Engine::SaveConfiguration() noexcept
{
	FILE *fp{ fopen(_configFilePath, "w") };
	if (!fp)
		return false;

	fprintf(fp, "[Engine]\n");
	fprintf(fp, "sDataDirectory=%s\n", _config.Engine.DataDirectory);
	fprintf(fp, "sLogFile=%s\n", _config.Engine.LogFile);
	fprintf(fp, "sGameModule=%s\n", _gameModuleFile);
	fprintf(fp, "sPhysicsModule=%s\n", _physicsModuleFile);
	fprintf(fp, "sAudioSystemModule=%s\n", _audioSystemModuleFile);
	fprintf(fp, "sArchiveFiles=%s\n", *_vfsArchiveList);
	fprintf(fp, "iWidth=%d\n", _config.Engine.ScreenWidth);
	fprintf(fp, "iHeight=%d\n", _config.Engine.ScreenHeight);
	fprintf(fp, "bFullscreen=%d\n", _config.Engine.Fullscreen ? 1 : 0);
	fprintf(fp, "bLoadLooseFiles=%d\n", _config.Engine.LoadLooseFiles ? 1 : 0);
	fprintf(fp, "bEnableConsole=%d\n", _config.Engine.EnableConsole ? 1 : 0);

	fprintf(fp, "[Renderer]\n");
	fprintf(fp, "bSupersampling=%d\n", _config.Renderer.Supersampling ? 1 : 0);
	fprintf(fp, "bMultisampling=%d\n", _config.Renderer.Multisampling ? 1 : 0);
	fprintf(fp, "iSamples=%d\n", _config.Renderer.Samples);
	fprintf(fp, "bAnisotropic=%d\n", _config.Renderer.Anisotropic ? 1 : 0);
	fprintf(fp, "iAniso=%d\n", _config.Renderer.Aniso);
	fprintf(fp, "bVerticalSync=%d\n", _config.Renderer.VerticalSync ? 1 : 0);
	fprintf(fp, "iTextureQuality=%d\n", _config.Renderer.TextureQuality);
	fprintf(fp, "iMaxLights=%d\n", _config.Renderer.MaxLights);
	fprintf(fp, "iShadowMapSize=%d\n", _config.Renderer.ShadowMapSize);
	fprintf(fp, "iMaxShadowMaps=%d\n", _config.Renderer.MaxShadowMaps);
	fprintf(fp, "bShadowMultisampling=%d\n", _config.Renderer.ShadowMultisampling ? 1 : 0);
	fprintf(fp, "iShadowSamples=%d\n", _config.Renderer.ShadowSamples);
	fprintf(fp, "bEnableAsyncCompute=%d\n", _config.Renderer.EnableAsyncCompute ? 1 : 0);
	fprintf(fp, "fGamma=%.02f\n", _config.Renderer.Gamma);
	fprintf(fp, "bUseDeviceGroup=%d\n", _config.Renderer.UseDeviceGroup ? 1 : 0);

	fprintf(fp, "[Renderer.SSAO]\n");
	fprintf(fp, "bEnable=%d\n", _config.Renderer.SSAO.Enable ? 1 : 0);
	fprintf(fp, "iKernelSize=%d\n", _config.Renderer.SSAO.KernelSize);
	fprintf(fp, "fRadius=%.03f\n", _config.Renderer.SSAO.Radius);
	fprintf(fp, "fPowerExponent=%.03f\n", _config.Renderer.SSAO.PowerExponent);
	fprintf(fp, "fThreshold=%.03f\n", _config.Renderer.SSAO.Threshold);
	fprintf(fp, "fBias=%.03f\n", _config.Renderer.SSAO.Bias);
	fprintf(fp, "bMultisampling=%d\n", _config.Renderer.SSAO.Multisampling ? 1 : 0);

	fprintf(fp, "[PostProcessor]\n");
	fprintf(fp, "bBloom=%d\n", _config.PostProcessor.Bloom ? 1 : 0);
	fprintf(fp, "bSupersampling=%d\n", _config.PostProcessor.BloomIntensity);
	fprintf(fp, "bDepthOfField=%d\n", _config.PostProcessor.DepthOfField ? 1 : 0);
	fprintf(fp, "bFilmGrain=%d\n", _config.PostProcessor.FilmGrain ? 1 : 0);

	fprintf(fp, "[Audio]\n");
	fprintf(fp, "fMasterVolume=%.01f\n", _config.Audio.MasterVolume);
	fprintf(fp, "fEffectsVolume=%.01f\n", _config.Audio.EffectsVolume);
	fprintf(fp, "fMusicVolume=%.01f\n", _config.Audio.MusicVolume);

	fprintf(fp, "[Input.VirtualAxis]\n");
	for (uint32_t i = 0; i < Input::GetVirtualAxisList().Count(); ++i) {
		const VirtualAxis &vAxis = Input::GetVirtualAxisList()[i];
		fprintf(fp, "v%d=%d,%d\n", i, vAxis.max, vAxis.min);
	}

	fprintf(fp, "[Input.ButtonMapping]\n");
	for (const pair<string, uint8_t> &kvp : Input::GetButtonMap())
		fprintf(fp, "%s=%d\n", kvp.first.c_str(), kvp.second);

	fprintf(fp, "[Input.AxisMapping]\n");
	for (const pair<string, uint8_t> &kvp : Input::GetAxisMap()) {
		if (kvp.second > NE_VIRT_AXIS)
			fprintf(fp, "%s=v%d\n", kvp.first.c_str(), kvp.second - NE_VIRT_AXIS);
		else
			fprintf(fp, "%s=%d;%.01f\n", kvp.first.c_str(), kvp.second, Input::GetAxisSensivity(kvp.second));
	}

	fclose(fp);

	return true;
}

void Engine::CleanUp() noexcept
{
	if (_disposed)
		return;
	
	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Shuting down...");

	EventManager::Broadcast(NE_EVT_SHUTDOWN, nullptr);
	EventManager::Release();

	Renderer::GetInstance()->WaitIdle();

	if (_gameModule)
		_gameModule->CleanUp();

	GUIManager::Release();
	SceneManager::Release();
	ResourceManager::Release();
	SoundManager::Release();
	Renderer::Release();
	AudioSystem::ReleaseInstance();
	Physics::ReleaseInstance();
	VFS::Release();
	Input::Release();
	Console::Release();

	delete _gameModule; _gameModule = nullptr;

	if (_gameModuleLibrary)
		Platform::ReleaseModule(_gameModuleLibrary);
	_gameModuleLibrary = nullptr;

	EngineClassFactory::CleanUp();
	Platform::CleanUp();

	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Shutdown complete");

	CrashHandler::Cleanup();

	EngineDebug::LogLeaks();

	_disposed = true;
}

void Engine::_FixedUpdate()
{
	//
}

void Engine::_Update(double deltaTime)
{
	PROF_BEGIN("Update", vec3(1.f, 1.f, 0.f));

	Input::Update();
	PROF_MARKER("Input", vec3(1.f, 1.f, 0.f));

	Physics::GetInstance()->Update(deltaTime);
	PROF_MARKER("Physics", vec3(1.f, 1.f, 0.f));

	SceneManager::UpdateScene(deltaTime);
	PROF_MARKER("Scene", vec3(1.f, 1.f, 0.f));

	if (_drawStats) _DrawStats();
	if (Console::IsOpen()) Console::Update();

	GUIManager::Update(deltaTime);
	PROF_MARKER("GUI", vec3(1.f, 1.f, 0.f));	

	Renderer::GetInstance()->Update(deltaTime);

	Input::ClearKeyState();
}

void Engine::_Draw()
{
	Renderer::GetInstance()->Draw();
}

void Engine::_DrawStats()
{
	const float charHeight = (float)GUIManager::GetCharacterHeight();
	const vec3 color = vec3(1.f, 1.f, 1.f);

	GUIManager::DrawString(vec2(0.f, 0.f), color, "FPS:       %d (%.02f ms)", _fps, _frameTime);
	GUIManager::DrawString(vec2(0.f, charHeight * 1), color, "Renderer:  %s %s", Renderer::GetInstance()->GetAPIName(), Renderer::GetInstance()->GetAPIVersion());
	GUIManager::DrawString(vec2(0.f, charHeight * 2), color, "Device:    %s", Renderer::GetInstance()->GetDeviceName());

	if (_config.Renderer.Supersampling)
		GUIManager::DrawString(vec2(0.f, charHeight * 3), color, "Screen:    %dx%d (%dx%d)", _config.Engine.ScreenWidth, _config.Engine.ScreenHeight, GetScreenWidth(), GetScreenHeight());
	else
		GUIManager::DrawString(vec2(0.f, charHeight * 3), color, "Screen:    %dx%d", _config.Engine.ScreenWidth, _config.Engine.ScreenHeight);

	GUIManager::DrawString(vec2(0.f, charHeight * 4), color, "Scene:     %s", SceneManager::IsSceneLoaded() ? *SceneManager::GetActiveScene()->GetName() : "No scene loaded");
	GUIManager::DrawString(vec2(0.f, charHeight * 5), color, "Verts:     %d", SceneManager::IsSceneLoaded() ? SceneManager::GetActiveScene()->GetVertexCount() : 0);
	GUIManager::DrawString(vec2(0.f, charHeight * 6), color, "Tris:      %d", SceneManager::IsSceneLoaded() ? SceneManager::GetActiveScene()->GetTriangleCount() : 0);
	GUIManager::DrawString(vec2(0.f, charHeight * 7), color, "Objects:   %d", SceneManager::IsSceneLoaded() ? SceneManager::GetActiveScene()->GetObjectCount() : 0);
	GUIManager::DrawString(vec2(0.f, charHeight * 8), color, "Lights:    %d", Renderer::GetInstance()->GetNumLights());
	GUIManager::DrawString(vec2(0.f, charHeight * 9), color, "StMeshes:  %d", ResourceManager::LoadedStaticMeshes());
	GUIManager::DrawString(vec2(0.f, charHeight * 10), color, "SkMeshes:  %d", ResourceManager::LoadedSkeletalMeshes());
	GUIManager::DrawString(vec2(0.f, charHeight * 11), color, "Textures:  %d", ResourceManager::LoadedTextures());
	GUIManager::DrawString(vec2(0.f, charHeight * 12), color, "Shaders:   %d", ResourceManager::LoadedShaderModules());
	GUIManager::DrawString(vec2(0.f, charHeight * 13), color, "Materials: %d", ResourceManager::LoadedMaterials());
	GUIManager::DrawString(vec2(0.f, charHeight * 14), color, "Sounds:    %d", ResourceManager::LoadedSounds());
	GUIManager::DrawString(vec2(0.f, charHeight * 15), color, "Fonts:     %d", ResourceManager::LoadedFonts());

	/*if (_haveMemoryInfo)
	{
	uint64_t totalMem, availableMem;

	totalMem = _renderer->GetVideoMemorySize();
	availableMem = _renderer->GetUsedVideoMemorySize();

	DrawString(vec2(0.f, charHeight * 16), color, "VRAM:      %d/%d MB", (totalMem - availableMem) / 1024, totalMem / 1024);
	}*/

	GUIManager::DrawString(vec2(0.f, _config.Engine.ScreenHeight - charHeight * 2), color, "NekoEngine");
#if defined(NE_CONFIG_DEBUG)
	GUIManager::DrawString(vec2(0.f, _config.Engine.ScreenHeight - charHeight), color, "Version: %s [%s] [Debug]", ENGINE_VERSION_STRING, ENGINE_PLATFORM_STRING);
#elif defined(NE_CONFIG_DEVELOPMENT)
	GUIManager::DrawString(vec2(0.f, _config.Engine.ScreenHeight - charHeight), color, "Version: %s [%s] [Development]", ENGINE_VERSION_STRING, ENGINE_PLATFORM_STRING);
#else
	GUIManager::DrawString(vec2(0.f, _config.Engine.ScreenHeight - charHeight), color, "Version: %s [%s]", ENGINE_VERSION_STRING, ENGINE_PLATFORM_STRING);
#endif
}
