/* NekoEngine
 *
 * Init.cpp
 * Author: Alexandru Naiman
 *
 * Engine initialization
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
#include <Engine/Console.h>
#include <Engine/Version.h>
#include <Engine/GameModule.h>
#include <Engine/SoundManager.h>
#include <Engine/ResourceManager.h>
#include <Renderer/SSAO.h>
#include <Renderer/Renderer.h>
#include <Renderer/PostProcessor.h>
#include <Scene/SceneManager.h>
#include <Profiler/Profiler.h>
#include <Platform/Platform.h>
#include <Platform/CrashHandler.h>
#include <Physics/Physics.h>
#include <System/Logger.h>
#include <System/VFS/VFS.h>
#include <Audio/AudioSystem.h>

using namespace std;
using namespace std::chrono;

#define INI_BUFF_SZ				4096
#define VFS_ARCHIVE_LIST_SIZE	4096

#define ENGINE_MODULE			"Engine"

extern high_resolution_clock::time_point _prevTime;
extern PlatformWindowType _engineWindow;
extern bool _graphicsDebug;
extern char *_configFilePath;
extern char _gameModuleFile[NE_PATH_SIZE];
extern char _physicsModuleFile[NE_PATH_SIZE];
extern char _audioSystemModuleFile[NE_PATH_SIZE];
extern NString _vfsArchiveList;

static bool _enableValidation = false;

int Engine::Initialize(const char *cmdLine, bool editor)
{
	int ret = ENGINE_FAIL;

	DBG_SET_THREAD_NAME("Main Thread");

	if (CrashHandler::Initialize() != ENGINE_OK)
		return ENGINE_FAIL;

	memset(&_config, 0x0, sizeof(struct Configuration));

	NString cmd(cmdLine);

	_ParseArgs(cmd);

	if (!_iniFileLoaded)
		_ReadINIFile("./Engine.ini");

	if (_config.Renderer.Supersampling) {
		const float numPixels = (_config.Engine.ScreenWidth * _config.Engine.ScreenHeight) * 2.f;
		const float wRatio = (float)_config.Engine.ScreenWidth / (float)_config.Engine.ScreenHeight;
		const float hRatio = (float)_config.Engine.ScreenHeight / (float)_config.Engine.ScreenWidth;
		
		const float newWidth = sqrt(wRatio * numPixels);
		const float newHeight = sqrt(hRatio * numPixels);

		_scaleFactor.x = newWidth / (float)_config.Engine.ScreenWidth;
		_scaleFactor.y = newHeight / (float)_config.Engine.ScreenHeight;
	}

	Logger::Initialize(_config.Engine.LogFile, LOG_ALL);

	if (Platform::Initialize() != ENGINE_OK)
		return ENGINE_FAIL;

	_editor = editor;

	if (!editor)
		Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "NekoEngine v%s starting up...", ENGINE_VERSION_STRING);
	else
		Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "NekoEditor v%s starting up...", ENGINE_VERSION_STRING);
	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Platform: %s", Platform::GetName());
	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Platform version: %s", Platform::GetVersion());
	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Machine name: %s", Platform::GetMachineName());
	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Architecture: %s", Platform::GetMachineArchitecture());
	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Processor: %s", Platform::GetProcessorName());
	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Memory: %.02f GB", (double)Platform::GetTotalSystemMemory() / 1024.0 / 1024.0 / 1024.0);

	_engineWindow = Platform::CreateWindow(_config.Engine.ScreenWidth, _config.Engine.ScreenHeight, _config.Engine.Fullscreen);

	if (!_engineWindow)
	{
		Platform::MessageBox("Fatal Error", "Failed to create window !", MessageBoxButtons::OK, MessageBoxIcon::Error);
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to create window.");
		return ENGINE_FAIL;
	}

	Platform::SetActiveWindow(_engineWindow);

	if ((ret = _InitSystem()) != ENGINE_OK)
		return ret;

	if ((ret = Renderer::GetInstance()->Initialize(_engineWindow, _enableValidation, _graphicsDebug)) != ENGINE_OK)
	{
		Platform::MessageBox("Fatal Error", "Failed to initialize the Renderer module !", MessageBoxButtons::OK, MessageBoxIcon::Error);
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize renderer.");
		return ENGINE_FAIL;
	}

	if (Physics::InitInstance(_physicsModuleFile) != ENGINE_OK)
	{
		Platform::MessageBox("Fatal Error", "Failed to initialize the Physics module !", MessageBoxButtons::OK, MessageBoxIcon::Error);
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize physics.");
		return ENGINE_FAIL;
	}

	if (AudioSystem::InitInstance(_audioSystemModuleFile) != ENGINE_OK)
	{
		Platform::MessageBox("Fatal Error", "Failed to initialize the AudioSystem module !", MessageBoxButtons::OK, MessageBoxIcon::Error);
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize audio.");
		return ENGINE_FAIL;
	}

	if (SoundManager::Initialize() != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize the sound manager");
		return false;
	}

	if (SceneManager::Initialize() != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize the scene manager");
		return ENGINE_FAIL;
	}

	if (GUIManager::Initialize() != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize GUI");
		return ENGINE_FAIL;
	}

	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Engine startup complete");

	::atexit(CleanUp);
	::atexit(Logger::Flush);

	if (!_InitGame())
	{
		CleanUp();
		return ENGINE_FAIL;
	}

	NString title;
	if (!strncmp(_gameModule->GetModuleName(), "TestGame", 8))
	{
		title = NString::StringWithFormat(256, "NekoEngine v%s [%s %s] [%s %s] [%s]", ENGINE_VERSION_STRING,
			Renderer::GetInstance()->GetAPIName(), Renderer::GetInstance()->GetAPIVersion(), AudioSystem::GetInstance()->GetName(), AudioSystem::GetInstance()->GetVersion(), ENGINE_PLATFORM_STRING);

#if defined(NE_CONFIG_DEBUG)
		title.Append(" [Debug]");
#elif defined(NE_CONFIG_DEVELOPMENT)
		title.Append(" [Development]");
#endif
	}
	else
		title = _gameModule->GetModuleName();

	Platform::SetWindowTitle(_engineWindow, *title);

	_prevTime = high_resolution_clock::now();\
	_startup = false;

	return ENGINE_OK;
}

void Engine::_ParseArgs(NString &cmdLine)
{
	NArray<NString> args = cmdLine.Split(' ');

	for (NString &arg : args)
	{
		char *ptr = NULL;

		if (!strchr(*arg, '='))
		{
			if (strstr(*arg, "--gfxdbg"))
				_graphicsDebug = true;
			else if (strstr(*arg, "--validation"))
				_enableValidation = true;
			continue;
		}

		if ((ptr = strstr(*arg, "--data")))
		{
			memset(_config.Engine.DataDirectory, 0x0, NE_PATH_SIZE);
			if (snprintf(_config.Engine.DataDirectory, NE_PATH_SIZE, "%s", ptr + 7) >= NE_PATH_SIZE)
			{ DIE("Invalid data directory argument !"); }
		}
		else if ((ptr = strstr(*arg, "--log")))
		{
			memset(_config.Engine.LogFile, 0x0, NE_PATH_SIZE);
			if (snprintf(_config.Engine.LogFile, NE_PATH_SIZE, "%s", ptr + 6) >= NE_PATH_SIZE)
			{ DIE("Invalid renderer argument !"); }
		}
		else if ((ptr = strstr(*arg, "--ini")))
			_ReadINIFile(ptr + 6);
	/*	else if ((ptr = strstr(*arg, "--renderer")))
		{
			memset(_rendererFile, 0x0, NE_PATH_SIZE);
			if (snprintf(_rendererFile, NE_PATH_SIZE, "%s", ptr + 11) >= NE_PATH_SIZE)
			{
				DIE("Invalid renderer argument !");
			}
		}*/
		else if ((ptr = strstr(*arg, "--game")))
		{
			memset(_gameModuleFile, 0x0, NE_PATH_SIZE);
			if (snprintf(_gameModuleFile, NE_PATH_SIZE, "%s", ptr + 7) >= NE_PATH_SIZE)
			{
				DIE("Invalid game module argument !");
			}
		}
		else if ((ptr = strstr(*arg, "--physics")))
		{
			memset(_physicsModuleFile, 0x0, NE_PATH_SIZE);
			if (snprintf(_physicsModuleFile, NE_PATH_SIZE, "%s", ptr + 10) >= NE_PATH_SIZE)
			{
				DIE("Invalid Physics module argument !");
			}
		}
		else if ((ptr = strstr(*arg, "--audio")))
		{
			memset(_physicsModuleFile, 0x0, NE_PATH_SIZE);
			if (snprintf(_physicsModuleFile, NE_PATH_SIZE, "%s", ptr + 8) >= NE_PATH_SIZE)
			{
				DIE("Invalid AudioSystem module argument !");
			}
		}
	}
}

void Engine::_ReadINIFile(const char *file)
{
	char buff[INI_BUFF_SZ];
	memset(buff, 0x0, INI_BUFF_SZ);

	if (_config.Engine.DataDirectory[0] == 0x0)
	{
		Platform::GetConfigString("Engine", "sDataDirectory", "Data", buff, INI_BUFF_SZ, file);
		memset(_config.Engine.DataDirectory, 0x0, NE_PATH_SIZE);
		if (snprintf(_config.Engine.DataDirectory, NE_PATH_SIZE, "%s", buff) >= NE_PATH_SIZE)
		{
			DIE("Failed to load configuration");
		}
		memset(buff, 0x0, INI_BUFF_SZ);
	}

	if (_config.Engine.LogFile[0] == 0x0)
	{
		Platform::GetConfigString("Engine", "sLogFile", "Engine.log", buff, INI_BUFF_SZ, file);
		memset(_config.Engine.LogFile, 0x0, NE_PATH_SIZE);
		if (snprintf(_config.Engine.LogFile, NE_PATH_SIZE, "%s", buff) >= NE_PATH_SIZE)
		{
			DIE("Failed to load configuration");
		}
		memset(buff, 0x0, INI_BUFF_SZ);
	}

	if (_gameModuleFile[0] == 0x0)
	{
		Platform::GetConfigString("Engine", "sGameModule", "Game", buff, INI_BUFF_SZ, file);
		memset(_gameModuleFile, 0x0, NE_PATH_SIZE);
		if (snprintf(_gameModuleFile, NE_PATH_SIZE, "%s", buff) >= NE_PATH_SIZE)
		{ DIE("Failed to load configuration"); }
		memset(buff, 0x0, INI_BUFF_SZ);
	}

	if (_physicsModuleFile[0] == 0x0)
	{
		Platform::GetConfigString("Engine", "sPhysicsModule", "NullPhysics", buff, INI_BUFF_SZ, file);
		memset(_physicsModuleFile, 0x0, NE_PATH_SIZE);
		if (snprintf(_physicsModuleFile, NE_PATH_SIZE, "%s", buff) >= NE_PATH_SIZE)
		{ DIE("Failed to load configuration"); }
		memset(buff, 0x0, INI_BUFF_SZ);
	}

	if (_audioSystemModuleFile[0] == 0x0)
	{
		Platform::GetConfigString("Engine", "sAudioSystemModule", "NullAudio", buff, INI_BUFF_SZ, file);
		memset(_audioSystemModuleFile, 0x0, NE_PATH_SIZE);
		if (snprintf(_audioSystemModuleFile, NE_PATH_SIZE, "%s", buff) >= NE_PATH_SIZE)
		{ DIE("Failed to load configuration"); }
		memset(buff, 0x0, INI_BUFF_SZ);
	}

	/*if (_rendererFile[0] == 0x0)
	{
		Platform::GetConfigString("Engine", "sRenderer", "GLRenderer", buff, INI_BUFF_SZ, file);
		memset(_rendererFile, 0x0, NE_PATH_SIZE);
		if (snprintf(_rendererFile, NE_PATH_SIZE, "%s", buff) >= NE_PATH_SIZE)
		{
			DIE("Failed to load configuration");
		}
		memset(buff, 0x0, INI_BUFF_SZ);
	}*/

	_vfsArchiveList.Clear();
	Platform::GetConfigString("Engine", "sArchiveFiles", "", *_vfsArchiveList, VFS_ARCHIVE_LIST_SIZE, file);
	_vfsArchiveList.Count();

	_config.Engine.ScreenWidth = (int)Platform::GetConfigInt("Engine", "iWidth", 1280, file);
	_config.Engine.ScreenHeight = (int)Platform::GetConfigInt("Engine", "iHeight", 720, file);
	_config.Engine.Fullscreen = Platform::GetConfigInt("Engine", "bFullscreen", 0, file) != 0;
	_config.Engine.LoadLooseFiles = Platform::GetConfigInt("Engine", "bLoadLooseFiles", 0, file) != 0;
	_config.Engine.EnableConsole = Platform::GetConfigInt("Engine", "bEnableConsole", 0, file) != 0;

	_config.Renderer.Supersampling = Platform::GetConfigInt("Renderer", "bSupersampling", 0, file) != 0;
	_config.Renderer.Multisampling = Platform::GetConfigInt("Renderer", "bMultisampling", 1, file) != 0;
	_config.Renderer.Samples = Platform::GetConfigInt("Renderer", "iSamples", 4, file);
	_config.Renderer.Anisotropic = Platform::GetConfigInt("Renderer", "bAnisotropic", 1, file) != 0;
	_config.Renderer.Aniso = Platform::GetConfigInt("Renderer", "iAniso", 16, file);
	_config.Renderer.VerticalSync = Platform::GetConfigInt("Renderer", "bVerticalSync", 1, file) != 0;
	_config.Renderer.TextureQuality = Platform::GetConfigInt("Renderer", "iTextureQuality", 2, file);
	_config.Renderer.ShadowMapSize = Platform::GetConfigInt("Renderer", "iShadowMapSize", 1024, file);
	_config.Renderer.MaxLights = Platform::GetConfigInt("Renderer", "iMaxLights", 1024, file);
	_config.Renderer.MaxShadowMaps = Platform::GetConfigInt("Renderer", "iMaxShadowMaps", 64, file);
	_config.Renderer.ShadowMultisampling = Platform::GetConfigInt("Renderer", "bShadowMultisampling", 1, file) != 0;
	_config.Renderer.ShadowSamples = Platform::GetConfigInt("Renderer", "iShadowSamples", 4, file);
	_config.Renderer.EnableAsyncCompute = Platform::GetConfigInt("Renderer", "bEnableAsyncCompute", 0, file) != 0;
	_config.Renderer.Gamma = Platform::GetConfigFloat("Renderer", "fGamma", 2.2f, file);
	_config.Renderer.UseDeviceGroup = Platform::GetConfigInt("Renderer", "bUseDeviceGroup", 0, file) != 0;

	_config.Renderer.SSAO.Enable = Platform::GetConfigInt("Renderer.SSAO", "bEnable", 1, file) != 0;
	_config.Renderer.SSAO.KernelSize = Platform::GetConfigInt("Renderer.SSAO", "iKernelSize", 128, file);
	_config.Renderer.SSAO.Radius = Platform::GetConfigFloat("Renderer.SSAO", "fRadius", 8.f, file);
	_config.Renderer.SSAO.PowerExponent = Platform::GetConfigFloat("Renderer.SSAO", "fPowerExponent", 2.f, file);
	_config.Renderer.SSAO.Threshold = Platform::GetConfigFloat("Renderer.SSAO", "fThreshold", .05f, file);
	_config.Renderer.SSAO.Bias = Platform::GetConfigFloat("Renderer.SSAO", "fBias", .025f, file);
	_config.Renderer.SSAO.Multisampling = Platform::GetConfigInt("Renderer.SSAO", "bMultisampling", 0, file) != 0;

	_config.PostProcessor.Bloom = Platform::GetConfigInt("PostProcessor", "bBloom", 1, file) != 0;
	_config.PostProcessor.BloomIntensity = Platform::GetConfigInt("PostProcessor", "iBloomIntensity", 0, file) != 0;
	_config.PostProcessor.DepthOfField = Platform::GetConfigInt("PostProcessor", "bDepthOfField", 1, file) != 0;
	_config.PostProcessor.FilmGrain = Platform::GetConfigInt("PostProcessor", "bFilmGrain", 1, file) != 0;

	_config.Audio.MasterVolume = Platform::GetConfigFloat("Audio", "fMasterVolume", 1.f, file);
	_config.Audio.EffectsVolume = Platform::GetConfigFloat("Audio", "fEffectsVolume", 1.f, file);
	_config.Audio.MusicVolume = Platform::GetConfigFloat("Audio", "fMusicVolume", 1.f, file);

	_ReadInputConfig(file);
	_ReadRendererConfig(file);

	_iniFileLoaded = true;
	_configFilePath = strdup(file);

	memset(buff, 0x0, INI_BUFF_SZ);
}

void Engine::_ReadInputConfig(const char *file)
{
	bool end = false;
	int i = 0, optI = 0;
	char buff[INI_BUFF_SZ], optBuff[INI_BUFF_SZ];

	memset(buff, 0x0, INI_BUFF_SZ);
	memset(optBuff, 0x0, INI_BUFF_SZ);

	end = Platform::GetConfigSection("Input.ButtonMapping", buff, INI_BUFF_SZ, file) == 0;
	while (!end)
	{
		char c = 0x0, *vptr = nullptr;

		while ((c = buff[i]) != 0x0)
		{
			optBuff[optI] = c;
			++optI;
			++i;
		}

		if (buff[++i] == 0x0)
			end = true;

		optBuff[optI] = 0x0;

		if ((vptr = strchr(optBuff, '=')) == nullptr)
			break;

		*vptr++ = 0x0;

		Input::AddButtonMapping(optBuff, atoi(vptr));

		memset(optBuff, 0x0, INI_BUFF_SZ);
		optI = 0;
	}

	end = false;
	i = 0;
	memset(buff, 0x0, INI_BUFF_SZ);

	end = Platform::GetConfigSection("Input.VirtualAxis", buff, INI_BUFF_SZ, file) == 0;
	while (!end)
	{
		char c = 0x0, *max_ptr = nullptr, *min_ptr = nullptr;

		while ((c = buff[i]) != 0x0)
		{
			optBuff[optI] = c;
			++optI;
			++i;
		}

		if (buff[++i] == 0x0)
			end = true;

		if ((max_ptr = strchr(optBuff, '=')) == nullptr)
			break;

		*max_ptr++ = 0x0;

		if ((min_ptr = strchr(max_ptr, ',')) == nullptr)
			break;

		*min_ptr++ = 0x0;

		Input::AddVirtualAxis(atoi(min_ptr), atoi(max_ptr));

		memset(optBuff, 0x0, INI_BUFF_SZ);
		optI = 0;
	}

	end = false;
	i = 0;
	memset(buff, 0x0, INI_BUFF_SZ);

	end = Platform::GetConfigSection("Input.AxisMapping", buff, INI_BUFF_SZ, file) == 0;
	while (!end)
	{
		char c = 0x0, *axis_ptr = nullptr, *sens_ptr = nullptr;
		int axis = 0;
		float sens = 0.f;

		while ((c = buff[i]) != 0x0)
		{
			optBuff[optI] = c;
			++optI;
			++i;
		}

		if (buff[++i] == 0x0)
			end = true;

		if ((axis_ptr = strchr(optBuff, '=')) == nullptr)
			break;

		if ((sens_ptr = strchr(axis_ptr, ';')) == nullptr)
			sens = 0;
		else
		{
			*sens_ptr++ = 0x0;
			sens = (float)atof(sens_ptr);
		}

		*axis_ptr++ = 0x0;

		if (*axis_ptr != 'v')
			axis = atoi(axis_ptr);
		else
			axis = atoi(++axis_ptr) + NE_VIRT_AXIS;

		Input::AddAxisMapping(optBuff, axis);
		Input::SetAxisSensivity(axis, sens);

		memset(optBuff, 0x0, INI_BUFF_SZ);
		optI = 0;
	}
}

void Engine::_ReadRendererConfig(const char *file)
{
	/*char name[256], buff[INI_BUFF_SZ], optBuff[INI_BUFF_SZ];
	bool end = false;
	int i = 0, optI = 0;

	memset(name, 0x0, 256);
	memset(buff, 0x0, INI_BUFF_SZ);
	snprintf(name, 256, "Renderer.%s", _rendererFile);

	end = Platform::GetConfigSection(name, buff, INI_BUFF_SZ, file) == 0;
	while (!end)
	{
		char c, *vptr;

		while ((c = buff[i]) != 0x0)
		{
			optBuff[optI] = c;
			++optI;
			++i;
		}

		if (buff[++i] == 0x0)
			end = true;

		if ((vptr = strchr(optBuff, '=')) == nullptr)
			break;

		*vptr++ = 0x0;

		_rendererArguments.insert(make_pair(optBuff, vptr));

		memset(optBuff, 0x0, INI_BUFF_SZ);
		optI = 0;
	}*/
}

int Engine::_InitSystem()
{
	int ret = ENGINE_FAIL;

	if (Console::Initialize() != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize the console");
		return false;
	}

	if ((ret = Input::Initialize(!_graphicsDebug)) != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize the input manager");
		return ret;
	}

	if (VFS::Initialize() != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize the virtual file system");
		return false;
	}

	NArray<NString> vfsArchives = _vfsArchiveList.Split(';');
	for (NString &archive : vfsArchives)
	{
		char buff[VFS_MAX_FILE_NAME];
		memset(buff, 0x0, VFS_MAX_FILE_NAME);
		if (snprintf(buff, VFS_MAX_FILE_NAME, "%s/%s", Engine::GetConfiguration().Engine.DataDirectory, *archive) >= VFS_MAX_FILE_NAME)
			return false;

		VFS::LoadArchive(buff);
	}

	if (ResourceManager::Initialize() != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize the resource manager");
		return false;
	}

	return ENGINE_OK;
}

bool Engine::_InitGame()
{
	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Loading game module");

#ifndef NE_DEVICE_MOBILE

	_gameModuleLibrary = Platform::LoadModule(_gameModuleFile);

	if (!_gameModuleLibrary)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Game module not found");

		if (Platform::MessageBox("Warning", "No game module specified or it cannot be found. Continue ?", MessageBoxButtons::YesNo, MessageBoxIcon::Warning) == MessageBoxResult::No)
			return false;

		Logger::Log(ENGINE_MODULE, LOG_WARNING, "Continuing on user action...");
		return true;
	}

	const CreateGameModuleProc createGameModule = (CreateGameModuleProc)Platform::GetProcAddress(_gameModuleLibrary, "createGameModule");

	if (!createGameModule)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Game module library is invalid");
		Platform::MessageBox("Fatal Error", "Game module library is invalid", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

#endif

	_gameModule = createGameModule();

	if (!_gameModule)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to load game module");
		Platform::MessageBox("Fatal Error", "Failed to load game module", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

	const int ret = _gameModule->Initialize();

	if (ret != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Game module Initialize() call failed with %d", ret);
		Platform::MessageBox("Fatal Error", "Failed to initialize game module", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Game module loaded");

	return true;
}
