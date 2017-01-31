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

#include <Platform/PlatformDetect.h>

#include <time.h>
#include <memory>
#include <string.h>
#include <stdint.h>
#include <chrono>

#include <string>
#include <unordered_map>

#if !defined(NE_DEVICE_MOBILE) && !defined(NE_PLATFORM_WIN32)
#include <png.h>
#endif

#include <glm/glm.hpp>

#include <Engine/Input.h>
#include <Engine/Engine.h>
#include <Engine/EngineUtils.h>
#include <Engine/EngineClassFactory.h>
#include <Engine/SceneManager.h>
#include <Engine/CameraManager.h>
#include <Engine/ResourceManager.h>
#include <Engine/SoundManager.h>
#include <Engine/PostProcessor.h>
#include <Engine/GameModule.h>
#include <Engine/DeferredBuffer.h>
#include <Engine/Console.h>
#include <System/Logger.h>
#include <Physics/Physics.h>
#include <PostEffects/Effect.h>
#include <System/VFS/VFS.h>
#include <GUI/GUIManager.h>

#include <PostEffects/Bloom.h>
#include <PostEffects/SMAA.h>
#include <PostEffects/FXAA.h>

#define INI_BUFF_SZ		4096

// 60 Hz logic update
#define UPDATE_DELTA		.01666

#define OGL_CTX_REQ_MAJOR	4
#define OGL_CTX_REQ_MINOR	5

#define VFS_ARCHIVE_LIST_SIZE	4096

#define ENGINE_MODULE "Engine"

using namespace std;
using namespace std::chrono;
using namespace glm;

float quadVertices[] =
{
	-1.f,  1.f,
	-1.f, -1.f,
	 1.f,  1.f,
	 1.f, -1.f
};

PlatformWindowType Engine::_engineWindow;
Configuration Engine::_config;
bool Engine::_disposed = false;
bool Engine::_printStats = false;
int Engine::_nFrames = 0;
double Engine::_lastTime = 0.f;
int Engine::_fps = 0;
GameModule* Engine::_gameModule = nullptr;
PlatformModuleType Engine::_gameModuleLibrary = nullptr;
char Engine::_gameModuleFile[PATH_SIZE] = { '\0' };
char Engine::_rendererFile[PATH_SIZE] = { '\0' };
bool Engine::_graphicsDebug = false;
RArrayBuffer* Engine::_quadVAO = nullptr;
RBuffer* Engine::_quadVBO = nullptr;
high_resolution_clock::time_point Engine::_prevTime;
Renderer* Engine::_renderer = nullptr;
PlatformModuleType Engine::_rendererLibrary = nullptr;
bool Engine::_haveMemoryInfo = false;
bool Engine::_startup = true;
bool Engine::_paused = false;
bool Engine::_editor = false;
FT_Library Engine::_ftLibrary = nullptr;
static bool iniFileLoaded = false;
static unordered_map<string, string> _rendererArguments;
static char _vfsArchiveList[VFS_ARCHIVE_LIST_SIZE];
static double _updateTime;
static double _renderTime;

ObjectClassMapType *EngineClassFactory::_objectClassMap = nullptr;
ComponentClassMapType *EngineClassFactory::_componentClassMap = nullptr;

#ifdef NE_DEVICE_MOBILE
extern "C" Renderer *createRenderer();
extern "C" GameModule *createGameModule();
#endif

void Engine::_ParseArgs(string cmdLine)
{
	vector<char*> args = EngineUtils::SplitString(cmdLine.c_str(), ' ');

	for(char* arg : args)
	{
		char *ptr = NULL;

		if (!strchr(arg, '='))
		{
			if (strstr(arg, "--gfxdbg"))
				_graphicsDebug = true;
			free(arg);
			continue;
		}

		if ((ptr = strstr(arg, "--data")))
		{
			memset(_config.Engine.DataDirectory, 0x0, PATH_SIZE);
			if (snprintf(_config.Engine.DataDirectory, PATH_SIZE, "%s", ptr+7) >= PATH_SIZE)
			{ DIE("Invalid data directory argument !"); }
		}
		else if((ptr = strstr(arg, "--log")))
		{
			memset(_config.Engine.LogFile, 0x0, PATH_SIZE);
			if (snprintf(_config.Engine.LogFile, PATH_SIZE, "%s", ptr+6) >= PATH_SIZE)
			{ DIE("Invalid renderer argument !"); }
		}
		else if ((ptr = strstr(arg, "--ini")))
			_ReadINIFile(ptr+6);
		else if ((ptr = strstr(arg, "--renderer")))
		{
			memset(_rendererFile, 0x0, PATH_SIZE);
			if (snprintf(_rendererFile, PATH_SIZE, "%s", ptr+11) >= PATH_SIZE)
			{ DIE("Invalid renderer argument !"); }
		}
		else if ((ptr = strstr(arg, "--game")))
		{
			memset(_gameModuleFile, 0x0, PATH_SIZE);
			if (snprintf(_gameModuleFile, PATH_SIZE, "%s", ptr+7) >= PATH_SIZE)
			{ DIE("Invalid renderer argument !"); }
		}

		free(arg);
	}
}

void Engine::_ReadEffectConfig(const char *file)
{
	if (_config.PostProcessor.Bloom)
	{
		Bloom *bloom = new Bloom();
		bloom->SetOption("Step", Platform::GetConfigFloat("PostEffects.Bloom", "fStep", .3f, file));

		PostProcessor::AddEffect(bloom);
	}

	if (_config.PostProcessor.SMAA)
	{
		SMAA *smaa = new SMAA();
		PostProcessor::AddEffect(smaa);
	}

	if (_config.PostProcessor.FXAA)
	{
		FXAA *fxaa = new FXAA();
		fxaa->SetOption("Subpix", Platform::GetConfigFloat("PostEffects.FXAA", "fSubpix", 1.f, file));
		fxaa->SetOption("EdgeThreshold", Platform::GetConfigFloat("PostEffects.FXAA", "fEdgeThreshold", .063f, file));
		fxaa->SetOption("EdgeThresholdMin", Platform::GetConfigFloat("PostEffects.FXAA", "fEdgeThresholdMin", 0.f, file));

		PostProcessor::AddEffect(fxaa);
	}
}

void Engine::_ReadInputConfig(const char *file)
{
	char buff[INI_BUFF_SZ], optBuff[INI_BUFF_SZ];
	bool end = false;
	int i = 0, optI = 0;

	memset(buff, 0x0, INI_BUFF_SZ);
	memset(optBuff, 0x0, INI_BUFF_SZ);

	end = Platform::GetConfigSection("Input.ButtonMapping", buff, INI_BUFF_SZ, file) == 0;
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

		Input::AddButtonMapping(optBuff, atoi(vptr));

		memset(optBuff, 0x0, INI_BUFF_SZ);
		optI = 0;
	}

	end = false;
	i = 0;
	memset(buff, 0x0, INI_BUFF_SZ);

	end = Platform::GetConfigSection("Input.AxisMapping", buff, INI_BUFF_SZ, file) == 0;
	while (!end)
	{
		char c, *axis_ptr, *sens_ptr;

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

		if((sens_ptr = strchr(axis_ptr, ';')) == nullptr)
			break;

		*axis_ptr++ = 0x0;
		*sens_ptr++ = 0x0;

		Input::AddAxisMapping(optBuff, atoi(axis_ptr));
		Input::SetAxisSensivity(atoi(axis_ptr), (float)atof(sens_ptr));

		memset(optBuff, 0x0, INI_BUFF_SZ);
		optI = 0;
	}
}

void Engine::_ReadRendererConfig(const char *file)
{
	char name[256], buff[INI_BUFF_SZ], optBuff[INI_BUFF_SZ];
	bool end = false;
	int i = 0, optI = 0;

	memset(name, 0x0, 256);
	memset(buff, 0x0, INI_BUFF_SZ);
	snprintf(name, 256, "Renderer.%s", _rendererFile);

	end = Platform::GetConfigSection(name, buff, INI_BUFF_SZ, file) == 0;
	while(!end)
	{
		char c, *vptr;

		while((c = buff[i]) != 0x0)
		{
			optBuff[optI] = c;
			++optI;
			++i;
		}

		if(buff[++i] == 0x0)
			end = true;

		if((vptr = strchr(optBuff, '=')) == nullptr)
			break;

		*vptr++ = 0x0;

		_rendererArguments.insert(make_pair(optBuff, vptr));

		memset(optBuff, 0x0, INI_BUFF_SZ);
		optI = 0;
	}
}

void Engine::_ReadINIFile(const char *file)
{
	char buff[INI_BUFF_SZ];
	memset(buff, 0x0, INI_BUFF_SZ);

	if(_config.Engine.DataDirectory[0] == 0x0)
	{
		Platform::GetConfigString("Engine", "sDataDirectory", "Data", buff, INI_BUFF_SZ, file);
		memset(_config.Engine.DataDirectory, 0x0, PATH_SIZE);
		if (snprintf(_config.Engine.DataDirectory, PATH_SIZE, "%s", buff) >= PATH_SIZE)
		{ DIE("Failed to load configuration"); }
		memset(buff, 0x0, INI_BUFF_SZ);
	}

	if(_config.Engine.LogFile[0] == 0x0)
	{
		Platform::GetConfigString("Engine", "sLogFile", "Engine.log", buff, INI_BUFF_SZ, file);
		memset(_config.Engine.LogFile, 0x0, PATH_SIZE);
		if (snprintf(_config.Engine.LogFile, PATH_SIZE, "%s", buff) >= PATH_SIZE)
		{ DIE("Failed to load configuration"); }
		memset(buff, 0x0, INI_BUFF_SZ);
	}

	if(_gameModuleFile[0] == 0x0)
	{
		Platform::GetConfigString("Engine", "sGameModule", "Game", buff, INI_BUFF_SZ, file);
		memset(_gameModuleFile, 0x0, PATH_SIZE);
		if (snprintf(_gameModuleFile, PATH_SIZE, "%s", buff) >= PATH_SIZE)
		{ DIE("Failed to load configuration"); }
		memset(buff, 0x0, INI_BUFF_SZ);
	}

	if(_rendererFile[0] == 0x0)
	{
		Platform::GetConfigString("Engine", "sRenderer", "GLRenderer", buff, INI_BUFF_SZ, file);
		memset(_rendererFile, 0x0, PATH_SIZE);
		if (snprintf(_rendererFile, PATH_SIZE, "%s", buff) >= PATH_SIZE)
		{ DIE("Failed to load configuration"); }
		memset(buff, 0x0, INI_BUFF_SZ);
	}

	memset(_vfsArchiveList, 0x0, VFS_ARCHIVE_LIST_SIZE);
	Platform::GetConfigString("Engine", "sArchiveFiles", "", _vfsArchiveList, VFS_ARCHIVE_LIST_SIZE, file);

	_config.Engine.ScreenWidth = (int)Platform::GetConfigInt("Engine", "iWidth", 1280, file);
	_config.Engine.ScreenHeight = (int)Platform::GetConfigInt("Engine", "iHeight", 720, file);
	_config.Engine.Fullscreen = Platform::GetConfigInt("Engine", "bFullscreen", 0, file) != 0;
	_config.Engine.LoadLooseFiles = Platform::GetConfigInt("Engine", "bLoadLooseFiles", 0, file) != 0;
	_config.Engine.EnableConsole = Platform::GetConfigInt("Engine", "bEnableConsole", 0, file) != 0;

	_config.Renderer.Quality = Platform::GetConfigInt("Renderer", "iQuality", RENDER_QUALITY_HIGH, file);
	_config.Renderer.Supersampling = Platform::GetConfigInt("Renderer", "bSupersampling", 0, file) != 0;
	_config.Renderer.Multisampling = Platform::GetConfigInt("Renderer", "bMultisampling", 1, file) != 0;
	_config.Renderer.Samples = Platform::GetConfigInt("Renderer", "iSamples", 4, file);
	_config.Renderer.Mipmaps = Platform::GetConfigInt("Renderer", "bUseMipmaps", 1, file) != 0;
	_config.Renderer.Anisotropic = Platform::GetConfigInt("Renderer", "bAnisotropic", 1, file) != 0;
	_config.Renderer.Aniso = Platform::GetConfigInt("Renderer", "iAniso", 16, file);
	_config.Renderer.VerticalSync = Platform::GetConfigInt("Renderer", "bVerticalSync", 1, file) != 0;
	_config.Renderer.SSAO = Platform::GetConfigInt("Renderer", "bSSAO", 1, file) != 0;
	_config.Renderer.SSAOQuality = Platform::GetConfigInt("Renderer", "iSSAOQuality", RENDER_SSAO_HIGH, file);
	_config.Renderer.TextureQuality = Platform::GetConfigInt("Renderer", "iTextureQuality", RENDER_TEX_Q_HIGH, file);
	_config.Renderer.ShadowMapSize = Platform::GetConfigInt("Renderer", "iShadowMapSize", 1024, file);
	_config.Renderer.HBAO = Platform::GetConfigInt("Renderer", "bHBAO", 1, file) != 0;

	_config.PostProcessor.Bloom = Platform::GetConfigInt("PostProcessor", "bBloom", 1, file) != 0;
	_config.PostProcessor.SMAA = Platform::GetConfigInt("PostProcessor", "bSMAA", 1, file) != 0;
	_config.PostProcessor.FXAA = Platform::GetConfigInt("PostProcessor", "bFXAA", 1, file) != 0;

	_ReadEffectConfig(file);
	_ReadInputConfig(file);
	_ReadRendererConfig(file);

	iniFileLoaded = true;

	memset(buff, 0x0, INI_BUFF_SZ);
}

void Engine::_InitializeQuadVAO()
{
	_quadVBO = _renderer->CreateBuffer(BufferType::Vertex, false, false);
	_quadVBO->SetStorage(sizeof(quadVertices), quadVertices);

	BufferAttribute attrib;
	attrib.index = SHADER_POSITION_ATTRIBUTE;
	attrib.sindex = 0;
	attrib.name = "POSITION";
	attrib.size = 2;
	attrib.type = BufferDataType::Float;
	attrib.normalize = false;
	attrib.stride = sizeof(float) * 2;
	attrib.ptr = (void*)0;
	_quadVBO->AddAttribute(attrib);

	_quadVAO = _renderer->CreateArrayBuffer();
	_quadVAO->SetVertexBuffer(_quadVBO);
	_quadVAO->CommitBuffers();
}

bool Engine::_InitRenderer()
{
#ifndef NE_DEVICE_MOBILE

	_rendererLibrary = Platform::LoadModule(_rendererFile);

	if (!_rendererLibrary)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Renderer not found");
		Platform::MessageBox("Fatal Error", "The specified renderer cannot be found. Exiting.", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

	RendererAPIVersionProc getRendererAPIVersion = (RendererAPIVersionProc)Platform::GetProcAddress(_rendererLibrary, "getRendererAPIVersion");

	if (!getRendererAPIVersion)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Renderer library is invalid");
		Platform::MessageBox("Fatal Error", "Renderer library is invalid !", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

	if (getRendererAPIVersion() != RENDERER_API_VERSION)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Renderer library is invalid. API version mismatch (expected %d, have %d)", RENDERER_API_VERSION, getRendererAPIVersion());
		Platform::MessageBox("Fatal Error", "Renderer API version mismatch !", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

	CreateRendererProc createRenderer = (CreateRendererProc)Platform::GetProcAddress(_rendererLibrary, "createRenderer");

	if (!createRenderer)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Renderer library is invalid");
		Platform::MessageBox("Fatal Error", "Renderer library is invalid !", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

#endif

	_renderer = createRenderer();

	if (!_renderer)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to load renderer");
		Platform::MessageBox("Fatal Error", "Failed to load renderer", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

#ifdef _DEBUG
	int ret = _renderer->Initialize(_engineWindow, &_rendererArguments, true);
#else
	int ret = _renderer->Initialize(_engineWindow, &_rendererArguments);
#endif

	if (!ret)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Renderer initialization failed");
		Platform::MessageBox("Fatal Error", "Failed to initialize renderer", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

	_renderer->SetDebugLogFunction(Logger::LogRendererDebugMessage);

	Logger::Log("Renderer", LOG_INFORMATION, "Renderer: %s", _renderer->GetName());
	Logger::Log("Renderer", LOG_INFORMATION, "Version: %d.%d", _renderer->GetMajorVersion(), _renderer->GetMinorVersion());

	_haveMemoryInfo = _renderer->HasCapability(RendererCapability::MemoryInformation);

	Shader::SetDefines(_renderer);

	return true;
}

bool Engine::_InitSystem()
{
	if (Console::Initialize() != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize the console");
		return false;
	}

	if (FT_Init_FreeType(&_ftLibrary))
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize the FreeType library");
		return false;
	}

	if (Input::Initialize(!_graphicsDebug) != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize the input manager");
		return false;
	}

	if (VFS::Initialize() != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize the virtual file system");
		return false;
	}

	vector<char*> vfsArchives = EngineUtils::SplitString(_vfsArchiveList, ';');

	for (char *archive : vfsArchives)
	{
		char buff[VFS_MAX_FILE_NAME];
		memset(buff, 0x0, VFS_MAX_FILE_NAME);
		if (snprintf(buff, VFS_MAX_FILE_NAME, "%s/%s", Engine::GetConfiguration().Engine.DataDirectory, archive) >= VFS_MAX_FILE_NAME)
			return false;

		VFS::LoadArchive(buff);

		free(archive);
	}

	if (ResourceManager::Initialize() != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize the resource manager");
		return false;
	}

	if (Physics::Initialize() != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize the physics module");
		return false;
	}

	if (SceneManager::Initialize() != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize the scene manager");
		return false;
	}

	if (DeferredBuffer::Initialize() != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize the deferred buffer");
		return false;
	}

	if (SoundManager::Initialize() != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize the sound manager");
		return false;
	}

	if (PostProcessor::Initialize() != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to initialize the post processor");
		return false;
	}

	return true;
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

	CreateGameModuleProc createGameModule = (CreateGameModuleProc)Platform::GetProcAddress(_gameModuleLibrary, "createGameModule");

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

	int ret = _gameModule->Initialize();

	if (ret != ENGINE_OK)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Game module Initialize() call failed with %d", ret);
		Platform::MessageBox("Fatal Error", "Failed to initialize game module", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Game module loaded");

	return true;
}

int Engine::Initialize(string cmdLine, bool editor)
{
	memset(&_config, 0x0, sizeof(struct Configuration));

	_ParseArgs(cmdLine);

	if (!iniFileLoaded)
		_ReadINIFile("./Engine.ini");

	Logger::Initialize(_config.Engine.LogFile, LOG_ALL);

	_editor = editor;

	if (!editor)
		Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "NekoEngine v%s starting up...", ENGINE_VERSION_STRING);
	else
		Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "NekoEditor v%s starting up...", ENGINE_VERSION_STRING);
	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Platform: %s", Platform::GetName());
	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Platform version: %s", Platform::GetVersion());
	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Machine name: %s", Platform::GetMachineName());
	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Architecture: %s", Platform::GetMachineArchitecture());

#ifndef EDITOR
	_engineWindow = Platform::CreateWindow(_config.Engine.ScreenWidth, _config.Engine.ScreenHeight, _config.Engine.Fullscreen);

	if (!_engineWindow)
	{
		Platform::MessageBox("Fatal Error", "Failed to create window !", MessageBoxButtons::OK, MessageBoxIcon::Error);
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Failed to create window.");
		return ENGINE_FAIL;
	}

	Platform::SetActiveWindow(_engineWindow);

	if (!_InitRenderer())
		return ENGINE_FAIL;
#endif

	_InitializeQuadVAO();

	if (!_InitSystem())
	{
		CleanUp();
		return ENGINE_FAIL;
	}

	const char *alVersionStr = nullptr;
	if (SoundManager::Enabled())
	{
		AL_CHECK_FATAL(alVersionStr = (const char *)alGetString(AL_VERSION));
		Logger::Log("OpenAL", LOG_INFORMATION, "Vendor: %s", alGetString(AL_VENDOR));
		Logger::Log("OpenAL", LOG_INFORMATION, "Renderer: %s", alGetString(AL_RENDERER));
		Logger::Log("OpenAL", LOG_INFORMATION, "Version: %s", alVersionStr);
		Logger::Log("OpenAL", LOG_INFORMATION, "Extensions: %s", alGetString(AL_EXTENSIONS));
	}
	else
		alVersionStr = "OpenAL disabled";

	_renderer->SetClearColor(0.f, 0.f, 0.f, 0.f);
	_renderer->Clear(R_CLEAR_COLOR | R_CLEAR_DEPTH | R_CLEAR_STENCIL);

	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Engine startup complete");

	// make sure the resources are released
	::atexit(CleanUp);

	// make sure the log queue gets written
	::atexit(Logger::Flush);

	if (!_graphicsDebug)
	{
		Platform::CapturePointer();
		Platform::SetPointerPosition(_config.Engine.ScreenWidth / 2, _config.Engine.ScreenHeight / 2);
	}

	if (_config.Renderer.VerticalSync)
		_renderer->SetSwapInterval(1);
	else
		_renderer->SetSwapInterval(0);

	GUIManager::Initialize();

	if (!_InitGame())
	{
		CleanUp();
		return ENGINE_FAIL;
	}

	NString title;
	if (!strncmp(_gameModule->GetModuleName(), "TestGame", 8))
	{
		NString alVersion(alVersionStr);
		if (alVersion.Find("OpenAL") != NString::NotFound)
			alVersion = "[";
		else
			alVersion = "[OpenAL ";
		alVersion.Append(alVersionStr);
		alVersion = alVersion.Substring(0, alVersion.FindFirst('.') + 2);

		title = NString::StringWithFormat(256, "NekoEngine v%s [%s %d.%d] %s] [%s]",
			ENGINE_VERSION_STRING, _renderer->GetName(), _renderer->GetMajorVersion(), _renderer->GetMinorVersion(),
			*alVersion, ENGINE_PLATFORM_STRING);

#ifdef _DEBUG
		title.Append(" [Debug]");
#endif
	}
	else
		title = _gameModule->GetModuleName();

	Platform::SetWindowTitle(_engineWindow, *title);

	_prevTime = high_resolution_clock::now();
	_startup = false;

	return ENGINE_OK;
}

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

void Engine::Pause(bool pause)
{
	_paused = pause;

	// TODO: let objects know the game was paused
}

void Engine::Frame() noexcept
{
	if(_startup)
		return;

	static double lastTime = GetTime();
	double temp = 0.0;
	double curTime = GetTime();
	double deltaTime = curTime - lastTime;

	_renderer->MakeCurrent(R_RENDER_CONTEXT);

	if (!_paused)
	{
		if (deltaTime > UPDATE_DELTA)
		{
			temp = GetTime();
			Update(deltaTime);
			_updateTime = GetTime() - temp;
			lastTime = curTime;
		}

		if (_printStats)
			_PrintStats();

		temp = GetTime();
		if (SceneManager::IsSceneLoaded())
			Draw();
		else
			SceneManager::DrawLoadingScreen();
		_renderTime = GetTime() - temp;
	}

	_renderer->SwapBuffers();

#ifdef _DEBUG
	Logger::Flush();
#endif

	/*else VSYNC ?
	{
		int sleepTime = (int)(1000.0 * (UPDATE_DELTA - curTime));

		if (sleepTime > 0)
			Sleep(sleepTime);
	}*/
}

void Engine::Draw() noexcept
{
	// Geometry pass

	DeferredBuffer::BindGeometry();

	_renderer->EnableDepthTest(true);
	_renderer->SetDepthMask(true);
	_renderer->EnableFaceCulling(true);
	_renderer->Clear(R_CLEAR_COLOR | R_CLEAR_DEPTH | R_CLEAR_STENCIL);

	Camera *cam = CameraManager::GetActiveCamera();

	SceneManager::DrawScene(DeferredBuffer::GetGeometryShader(), cam);
	DeferredBuffer::Unbind();

	// Lighting pass

	_renderer->SetDepthMask(false);

	DeferredBuffer::BindLighting();
	_renderer->Clear(R_CLEAR_COLOR);

	DeferredBuffer::RenderLighting();

	SceneManager::GetActiveScene()->DrawSkybox(cam);

	// SceneManager::GetActiveScene()->DrawForwardObjects();

	// Effects

	DeferredBuffer::CopyLight(PostProcessor::GetBuffer());
	DeferredBuffer::CopyColor(PostProcessor::GetColorBuffer());
	DeferredBuffer::CopyBrightness(PostProcessor::GetBrightnessBuffer());

	PostProcessor::ApplyEffects();

	_renderer->EnableFaceCulling(false);
	_renderer->EnableDepthTest(false);

	if (Console::IsOpen())
		Console::Draw();

	// GUI

	GUIManager::Draw();
}

void Engine::Update(double deltaTime) noexcept
{
	if (!SceneManager::IsSceneLoaded())
		return;

	Input::Update();

	Physics::Update(deltaTime);

	if (_config.Engine.EnableConsole)
		if (Input::GetButtonDown(NE_KEY_TILDE))
			Console::OpenConsole();

	GUIManager::Update(deltaTime);
	SceneManager::UpdateScene(deltaTime);

	Input::ClearKeyState();
}

double Engine::GetTime() noexcept
{
	high_resolution_clock::time_point time = high_resolution_clock::now();
	high_resolution_clock::duration diff = time - _prevTime;

	return (double)diff.count() * high_resolution_clock::period::num / high_resolution_clock::period::den;
}

void Engine::ScreenResized(int width, int height) noexcept
{
	if (_startup)
		return;

	_config.Engine.ScreenWidth = width;
	_config.Engine.ScreenHeight = height;

	DeferredBuffer::ScreenResized(width, height);
	PostProcessor::ScreenResized();

	GUIManager::ScreenResized();

	if(SceneManager::IsSceneLoaded())
		CameraManager::GetActiveCamera()->UpdateProjection();

	_renderer->SetViewport(0, 0, width, height);
	_renderer->ScreenResized();
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

void Engine::SaveScreenshot() noexcept
{
#if !defined(NE_DEVICE_MOBILE) && !defined(NE_PLATFORM_WIN32)
	unsigned char *pixels = nullptr;

	size_t imageSize = 3 /* BPP - RGB */ * _config.Engine.ScreenWidth * _config.Engine.ScreenHeight;
	size_t imageI = 0;

	pixels = (unsigned char *)calloc(imageSize, sizeof(unsigned char));

	if (!pixels)
	{
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Memory allocation failed");
		DIE("Memory allocation failed");
	}

	_renderer->ReadPixels(0, 0, _config.Engine.ScreenWidth, _config.Engine.ScreenHeight, TextureFormat::RGB, TextureInternalType::UnsignedByte, pixels);

	// reverse the rectangle
	for (int i = 0; i < _config.Engine.ScreenHeight / 2; i++)
	{
		int swapI = _config.Engine.ScreenHeight - i - 1;

		for (int j = 0; j < _config.Engine.ScreenWidth; j++)
		{
			int offset = 3 * (j + i * _config.Engine.ScreenWidth);
			int swapOffset = 3 * (j + swapI * _config.Engine.ScreenWidth);

			swap(pixels[offset], pixels[swapOffset]);
			swap(pixels[offset + 1], pixels[swapOffset + 1]);
			swap(pixels[offset + 2], pixels[swapOffset + 2]);
		}
	}

	// save png file
	time_t t = time(0);
	struct tm *tm = localtime(&t);

	char tmstr[256];
	memset(&tmstr, 0x0, 256);
	(void)strftime(tmstr, 256, "%m%d%Y_%H%M%S", tm);

	char file[512];
	if (snprintf(file, 512, "screenshot_%s.png", tmstr) >= 512)
	{
		Logger::Log(ENGINE_MODULE, LOG_WARNING, "Failed to create file name.");
		free(pixels);
		return;
	}

	FILE *fp = fopen(file, "wb");
	if (!fp)
	{
		Logger::Log(ENGINE_MODULE, LOG_WARNING, "Failed to open file for writing");
		free(pixels);
		return;
	}

	png_structp pngStruct = nullptr;
	png_infop pngInfo = nullptr;
	png_bytep pngRow = nullptr;

	pngStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!pngStruct)
	{
		free(pixels);
		fclose(fp);
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Memory allocation failed");
		DIE("Memory allocation failed");
	}

	pngInfo = png_create_info_struct(pngStruct);
	if (!pngInfo)
	{
		free(pixels);
		free(pngStruct);
		fclose(fp);
		Logger::Log(ENGINE_MODULE, LOG_CRITICAL, "Memory allocation failed");
		DIE("Memory allocation failed");
	}

	png_init_io(pngStruct, fp);

	png_set_IHDR(pngStruct, pngInfo, _config.Engine.ScreenWidth, _config.Engine.ScreenHeight,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_text title;
	title.compression = PNG_TEXT_COMPRESSION_NONE;
	title.key = (char *)"Title";
	title.text = (char *)"NekoEngine Screenshot";
	png_set_text(pngStruct, pngInfo, &title, 1);

	png_write_info(pngStruct, pngInfo);

	size_t rowSize = 3 * _config.Engine.ScreenWidth;
	pngRow = (png_bytep)calloc(rowSize, sizeof(png_byte));

	if (!pngRow)
	{
		fclose(fp);
		free(pixels);
		free(pngInfo);
		free(pngStruct);
		free(pngRow);

		return;
	}

	int x, y;
	for (y = 0; y < _config.Engine.ScreenHeight; y++)
	{
		for (x = 0; x < _config.Engine.ScreenWidth; x++)
		{
			png_bytep currentRow = &(pngRow[x * 3]);

			currentRow[0] = pixels[imageI];
			currentRow[1] = pixels[imageI + 1];
			currentRow[2] = pixels[imageI + 2];

			imageI += 3;
		}

		png_write_row(pngStruct, pngRow);
	}

	png_write_end(pngStruct, nullptr);

	fclose(fp);
	free(pixels);
	free(pngInfo);
	free(pngStruct);
	free(pngRow);
#endif
}

void Engine::_PrintStats()
{
	double diff = GetTime() - _lastTime;
	static double _frameTime = 0.0;
	_nFrames++;

	if (diff > 1.f)
	{
		_fps = _nFrames;
		_frameTime = (diff / (double)_fps) * 1000;
		_lastTime += diff;
		_nFrames = 0;
	}

	// Uncomment to show time per frame
	/*static double _lastFrameTime = esGetTime();
	_frameTime = (esGetTime() - _lastFrameTime) * 1000;
	_lastFrameTime = esGetTime();*/

	float charHeight = (float)GUIManager::GetCharacterHeight();
	vec3 color = vec3(1.f, 1.f, 1.f);

	GUIManager::DrawString(vec2(0.f, 0.f), color, "FPS:       %d (%.02f ms)", _fps, _frameTime);
	GUIManager::DrawString(vec2(0.f, charHeight * 1), color, "DrawCalls: %ld", _renderer->GetDrawCalls());
	_renderer->ResetDrawCalls();
	GUIManager::DrawString(vec2(0.f, charHeight * 2), color, "Renderer:  %s %d.%d", _renderer->GetName(), _renderer->GetMajorVersion(), _renderer->GetMinorVersion());
	GUIManager::DrawString(vec2(0.f, charHeight * 3), color, "Screen:    %dx%d (FBO: %dx%d)",
		_config.Engine.ScreenWidth, _config.Engine.ScreenHeight, DeferredBuffer::GetWidth(), DeferredBuffer::GetHeight());
	GUIManager::DrawString(vec2(0.f, charHeight * 4), color, "Scene:     %s", SceneManager::GetActiveScene()->GetName().c_str());
	GUIManager::DrawString(vec2(0.f, charHeight * 5), color, "Verts:     %d", SceneManager::GetActiveScene()->GetVertexCount());
	GUIManager::DrawString(vec2(0.f, charHeight * 6), color, "Tris:      %d", SceneManager::GetActiveScene()->GetTriangleCount());
	GUIManager::DrawString(vec2(0.f, charHeight * 7), color, "Objects:   %d", SceneManager::GetActiveScene()->GetObjectCount());
	GUIManager::DrawString(vec2(0.f, charHeight * 8), color, "Lights:    %d", SceneManager::GetActiveScene()->GetNumLights());
	GUIManager::DrawString(vec2(0.f, charHeight * 9), color, "StMeshes:  %d", ResourceManager::LoadedStaticMeshes());
	GUIManager::DrawString(vec2(0.f, charHeight * 10), color, "SkMeshes:  %d", ResourceManager::LoadedSkeletalMeshes());
	GUIManager::DrawString(vec2(0.f, charHeight * 11), color, "Textures:  %d", ResourceManager::LoadedTextures());
	GUIManager::DrawString(vec2(0.f, charHeight * 12), color, "Shaders:   %d", ResourceManager::LoadedShaders());
	GUIManager::DrawString(vec2(0.f, charHeight * 13), color, "Materials: %d", ResourceManager::LoadedMaterials());
	GUIManager::DrawString(vec2(0.f, charHeight * 14), color, "Sounds:    %d", ResourceManager::LoadedSounds());
	GUIManager::DrawString(vec2(0.f, charHeight * 15), color, "Fonts:     %d", ResourceManager::LoadedFonts());

	if (_haveMemoryInfo)
	{
		uint64_t totalMem, availableMem;

		totalMem = _renderer->GetVideoMemorySize();
		availableMem = _renderer->GetUsedVideoMemorySize();

		GUIManager::DrawString(vec2(0.f, charHeight * 16), color, "VRAM:      %d/%d MB", (totalMem - availableMem) / 1024, totalMem / 1024);
	}

	GUIManager::DrawString(vec2(0.f, _config.Engine.ScreenHeight - charHeight * 2), color, "NekoEngine");
#ifdef _DEBUG
	GUIManager::DrawString(vec2(0.f, _config.Engine.ScreenHeight - charHeight), color, "Version: %s [%s] [Debug]", ENGINE_VERSION_STRING, ENGINE_PLATFORM_STRING);
#else
	GUIManager::DrawString(vec2(0.f, _config.Engine.ScreenHeight - charHeight), color, "Version: %s [%s]", ENGINE_VERSION_STRING, ENGINE_PLATFORM_STRING);
#endif
}

void Engine::CleanUp() noexcept
{
	if (_disposed)
		return;

	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Shuting down...");

	if(!_graphicsDebug)
		Platform::ReleasePointer();

	if(_gameModule)
		_gameModule->CleanUp();

	GUIManager::Release();
	DeferredBuffer::Release();
	PostProcessor::Release();
	SceneManager::Release();
	Physics::Release();
	SoundManager::Release();
	ResourceManager::Release();
	VFS::Release();
	Input::Release();

	delete _gameModule; _gameModule = nullptr;

	if(_gameModuleLibrary)
		Platform::ReleaseModule(_gameModuleLibrary);
	_gameModuleLibrary = nullptr;

	delete _quadVAO; _quadVAO = nullptr;
	delete _quadVBO; _quadVBO = nullptr;

	delete _renderer; _renderer = nullptr;

	if(_rendererLibrary)
		Platform::ReleaseModule(_rendererLibrary);
	_rendererLibrary = nullptr;

	EngineClassFactory::CleanUp();

	FT_Done_FreeType(_ftLibrary);

	Logger::Log(ENGINE_MODULE, LOG_INFORMATION, "Shutdown complete");

	Platform::CleanUp();

	_disposed = true;
}
