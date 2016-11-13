/* NekoEngine
 *
 * Engine.cpp
 * Author: Alexandru Naiman
 *
 * Engine class implementation
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman
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
#include <System/Logger.h>
#include <System/VFS/VFS.h>
#include <Renderer/SSAO.h>
#include <Renderer/Renderer.h>
#include <Renderer/PostProcessor.h>

 // 60 Hz logic update
#define UPDATE_DELTA	.01666

#define ENGINE_MODULE	"Engine"

using namespace glm;
using namespace std;
using namespace std::chrono;

Configuration Engine::_config;
GameModule *Engine::_gameModule = nullptr;
PlatformModuleType Engine::_gameModuleLibrary = nullptr;
bool Engine::_iniFileLoaded = false;
bool Engine::_paused = false;
bool Engine::_editor = false;
bool Engine::_disposed = false;
bool Engine::_drawStats = true;
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
	double curTime = GetTime();
	double deltaTime = curTime - lastTime;
	double deltaFPSTime = curTime - lastFPSTime;

	nFrames++;

	if (deltaFPSTime > 1.f)
	{
		_fps = nFrames;
		_frameTime = (deltaFPSTime / (double)_fps) * 1000;
		lastFPSTime += deltaFPSTime;
		nFrames = 0;
	}

	if (!_paused)
	{
		if (curTime > nextUpdateTime)
		{
			_Update(deltaTime);
			lastTime = curTime;
			nextUpdateTime += UPDATE_DELTA;
		}

		_Draw();
	}

#if defined(NE_CONFIG_DEBUG) || defined(NE_CONFIG_DEVELOPMENT)
	Logger::Flush();
#endif
}

void Engine::ScreenResized(int width, int height) noexcept
{
	//
}

double Engine::GetTime() noexcept
{
	high_resolution_clock::time_point time = high_resolution_clock::now();
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

	if (_config.Renderer.SSAO.Enable) SSAO::Release();
	if (_config.PostProcessor.Enable) PostProcessor::Release();

	GUI::Release();
	SceneManager::Release();
	ResourceManager::Release();
	SoundManager::Release();
	Renderer::Release();
	VFS::Release();
	Input::Release();
	Console::Release();

	delete _gameModule; _gameModule = nullptr;

	if (_gameModuleLibrary)
		Platform::ReleaseModule(_gameModuleLibrary);
	_gameModuleLibrary = nullptr;

	EngineClassFactory::CleanUp();

	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Shutdown complete");

	Platform::CleanUp();

	_disposed = true;
}

void Engine::_Update(double deltaTime)
{
	Input::Update();

	if (!_paused && SceneManager::IsSceneLoaded())
		SceneManager::UpdateScene(deltaTime);

	if (_drawStats) _DrawStats();
	if (Console::IsOpen()) Console::Update();

	GUI::Update(deltaTime);
	Renderer::GetInstance()->Update(deltaTime);

	Input::ClearKeyState();
}

void Engine::_Draw()
{
	Renderer::GetInstance()->Draw();
}

void Engine::_DrawStats()
{
	float charHeight = (float)GUI::GetCharacterHeight();
	vec3 color = vec3(1.f, 1.f, 1.f);

	GUI::DrawString(vec2(0.f, 0.f), color, "FPS:       %d (%.02f ms)", _fps, _frameTime);
	GUI::DrawString(vec2(0.f, charHeight * 1), color, "Renderer:  %s %s", Renderer::GetInstance()->GetAPIName(), Renderer::GetInstance()->GetAPIVersion());
	GUI::DrawString(vec2(0.f, charHeight * 2), color, "Device:    %s", Renderer::GetInstance()->GetDeviceName());

	if (_config.Renderer.Supersampling)
		GUI::DrawString(vec2(0.f, charHeight * 3), color, "Screen:    %dx%d (%dx%d)", _config.Engine.ScreenWidth, _config.Engine.ScreenHeight, GetScreenWidth(), GetScreenHeight());
	else
		GUI::DrawString(vec2(0.f, charHeight * 3), color, "Screen:    %dx%d", _config.Engine.ScreenWidth, _config.Engine.ScreenHeight);

	GUI::DrawString(vec2(0.f, charHeight * 4), color, "Scene:     %s", SceneManager::IsSceneLoaded() ? *SceneManager::GetActiveScene()->GetName() : "No scene loaded");
	GUI::DrawString(vec2(0.f, charHeight * 5), color, "Verts:     %d", SceneManager::IsSceneLoaded() ? SceneManager::GetActiveScene()->GetVertexCount() : 0);
	GUI::DrawString(vec2(0.f, charHeight * 6), color, "Tris:      %d", SceneManager::IsSceneLoaded() ? SceneManager::GetActiveScene()->GetTriangleCount() : 0);
	GUI::DrawString(vec2(0.f, charHeight * 7), color, "Objects:   %d", SceneManager::IsSceneLoaded() ? SceneManager::GetActiveScene()->GetObjectCount() : 0);
	GUI::DrawString(vec2(0.f, charHeight * 8), color, "Lights:    %d", Renderer::GetInstance()->GetNumLights());
	GUI::DrawString(vec2(0.f, charHeight * 9), color, "StMeshes:  %d", ResourceManager::LoadedStaticMeshes());
	GUI::DrawString(vec2(0.f, charHeight * 10), color, "SkMeshes:  %d", ResourceManager::LoadedSkeletalMeshes());
	GUI::DrawString(vec2(0.f, charHeight * 11), color, "Textures:  %d", ResourceManager::LoadedTextures());
	GUI::DrawString(vec2(0.f, charHeight * 12), color, "Shaders:   %d", ResourceManager::LoadedShaderModules());
	GUI::DrawString(vec2(0.f, charHeight * 13), color, "Materials: %d", ResourceManager::LoadedMaterials());
	GUI::DrawString(vec2(0.f, charHeight * 14), color, "Sounds:    %d", ResourceManager::LoadedSounds());
	GUI::DrawString(vec2(0.f, charHeight * 15), color, "Fonts:     %d", ResourceManager::LoadedFonts());

	/*if (_haveMemoryInfo)
	{
	uint64_t totalMem, availableMem;

	totalMem = _renderer->GetVideoMemorySize();
	availableMem = _renderer->GetUsedVideoMemorySize();

	DrawString(vec2(0.f, charHeight * 16), color, "VRAM:      %d/%d MB", (totalMem - availableMem) / 1024, totalMem / 1024);
	}*/

	GUI::DrawString(vec2(0.f, _config.Engine.ScreenHeight - charHeight * 2), color, "NekoEngine");
#if defined(NE_CONFIG_DEBUG)
	GUI::DrawString(vec2(0.f, _config.Engine.ScreenHeight - charHeight), color, "Version: %s [%s] [Debug]", ENGINE_VERSION_STRING, ENGINE_PLATFORM_STRING);
#elif defined(NE_CONFIG_DEVELOPMENT)
	GUI::DrawString(vec2(0.f, _config.Engine.ScreenHeight - charHeight), color, "Version: %s [%s] [Development]", ENGINE_VERSION_STRING, ENGINE_PLATFORM_STRING);
#else
	GUI::DrawString(vec2(0.f, _config.Engine.ScreenHeight - charHeight), color, "Version: %s [%s]", ENGINE_VERSION_STRING, ENGINE_PLATFORM_STRING);
#endif
}
