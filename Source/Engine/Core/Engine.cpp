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
#include <Engine/Debug.h>
#include <Engine/Input.h>
#include <Engine/Engine.h>
#include <Engine/Events.h>
#include <Engine/Version.h>
#include <Engine/Console.h>
#include <Engine/GameModule.h>
#include <Engine/SoundManager.h>
#include <Engine/SceneManager.h>
#include <Engine/EventManager.h>
#include <Engine/ResourceManager.h>
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
#define UPDATE_DELTA	.01666

#define ENGINE_MODULE	"Engine"

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
static int _fps = 0;
static double _frameTime = 0.0;

ObjectClassMapType *EngineClassFactory::_objectClassMap = nullptr;
ComponentClassMapType *EngineClassFactory::_componentClassMap = nullptr;

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
	static double lastTime = GetTime(), lastFPSTime = GetTime(), nextUpdateTime = 0.0;
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
	
	//if (curTime > nextUpdateTime)
	//{
		_Update(deltaTime);
		lastTime = curTime;
	//	nextUpdateTime += UPDATE_DELTA;
//	}

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

void Engine::CleanUp() noexcept
{
	if (_disposed)
		return;
	
	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Shuting down...");

	EventManager::Broadcast(NE_EVT_SHUTDOWN, nullptr);

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
	GUIManager::DrawString(vec2(0.f, _config.Engine.ScreenHeight - charHeight), color, "Version: %s \"%s\" [%s] [Debug]", ENGINE_VERSION_STRING, ENGINE_WORKING_NAME, ENGINE_PLATFORM_STRING);
#elif defined(NE_CONFIG_DEVELOPMENT)
	GUIManager::DrawString(vec2(0.f, _config.Engine.ScreenHeight - charHeight), color, "Version: %s \"%s\" [%s] [Development]", ENGINE_VERSION_STRING, ENGINE_WORKING_NAME, ENGINE_PLATFORM_STRING);
#else
	GUIManager::DrawString(vec2(0.f, _config.Engine.ScreenHeight - charHeight), color, "Version: %s \"%s\" [%s]", ENGINE_VERSION_STRING, ENGINE_WORKING_NAME, ENGINE_PLATFORM_STRING);
#endif
}