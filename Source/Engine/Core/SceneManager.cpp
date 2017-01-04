/* NekoEngine
 *
 * SceneManager.cpp
 * Author: Alexandru Naiman
 *
 * SceneManager class implementation 
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

#include <fstream>

#include <Engine/Engine.h>
#include <Scene/Scene.h>
#include <System/Logger.h>
#include <Engine/EngineUtils.h>
#include <Engine/SceneManager.h>
#include <Engine/LoadingScreen.h>
#include <Engine/ResourceManager.h>
#include <System/VFS/VFS.h>

#define LINE_BUFF	1024
#define SM_MODULE	"SceneManager"

using namespace std;

std::vector<Scene*> SceneManager::_scenes;
Scene *SceneManager::_activeScene = nullptr;
Scene *SceneManager::_loadingScene = nullptr;
int SceneManager::_defaultScene = 0;
int SceneManager::_loadScene = -1;
LoadingScreen *SceneManager::_loadingScreen = nullptr;
thread *SceneManager::_loadThread = nullptr;

int SceneManager::Initialize()
{
	return _ReadConfigFile("/scenes.cfg");
}

int SceneManager::_ReadConfigFile(NString file)
{
	char lineBuff[LINE_BUFF];
	memset(lineBuff, 0x0, LINE_BUFF);

	Logger::Log(SM_MODULE, LOG_INFORMATION, "Loading configuration...");

	_UnloadScenes();

	VFSFile *f = VFS::Open(file);
	if (!f)
	{
		Logger::Log(SM_MODULE, LOG_CRITICAL, "Failed to open configuration file");
		return ENGINE_IO_FAIL;
	}

	while (!f->EoF())
	{
		memset(lineBuff, 0x0, LINE_BUFF);
		f->Gets(lineBuff, LINE_BUFF);

		if (lineBuff[0] == 0x0)
			continue;

		EngineUtils::RemoveComment(lineBuff);
		EngineUtils::RemoveNewline(lineBuff);

		if (lineBuff[0] == 0x0)
			continue;

		if (!strchr(lineBuff, '='))
			continue;

		vector<char*> split = EngineUtils::SplitString(lineBuff, '=');

		size_t len = strlen(split[0]);
		if (!strncmp(split[0], "DefaultScene", len))
			_defaultScene = atoi(split[1]);
		else if(!strncmp(split[0], "Scene", len))
		{
			vector<char*> scnSplit = EngineUtils::SplitString(split[1], ',');

			if(scnSplit.size() == 3)
				_scenes.push_back(new Scene(atoi(scnSplit[0]), scnSplit[1], scnSplit[2]));
			else
				_scenes.push_back(new Scene(atoi(scnSplit[0]), scnSplit[1]));
			
			for(char* p :scnSplit)
				free(p);
		}
		
		for(char* p : split)
			free(p);
	}

	f->Close();

	Logger::Log(SM_MODULE, LOG_INFORMATION, "Initialized");

	return ENGINE_OK;
}

int SceneManager::LoadScene(int id)
{
	if (_activeScene)
	{
		_loadScene = id;
		return ENGINE_OK;
	}

	return _LoadSceneInternal(id);
}

int SceneManager::LoadNextScene()
{
	static unsigned int scene = 0;

	scene++;

	if (scene >= _scenes.size())
		scene = 0;

	return LoadScene(scene);
}

int SceneManager::DrawScene(RShader* shader, Camera *camera) noexcept
{
	shader->Enable();

	if (_activeScene)
	{
		if (_loadingScreen)
		{
			_activeScene->CreateArrayBuffers();
			
			if (Engine::GetConfiguration().Renderer.Mipmaps)
			{
				NArray<Resource *> _textures = ResourceManager::GetResourcesOfType(ResourceType::RES_TEXTURE);
				for(Resource *r : _textures)
					((Texture *)r)->GetRTexture()->GenerateMipmaps();
			}
				
			delete _loadingScreen;
			_loadingScreen = nullptr;
		}

		_activeScene->Draw(shader, camera);
	}

	shader->Disable();

	return ENGINE_OK;
}

void SceneManager::DrawLoadingScreen() noexcept
{
	Engine::GetRenderer()->MakeCurrent(R_RENDER_CONTEXT);

	if (_loadingScreen == nullptr)
		_loadingScreen = new LoadingScreen(LS_DEFAULT_TEXTURE);

	_loadingScreen->Draw();
}

void SceneManager::UpdateScene(double deltaTime) noexcept
{
	if (_loadScene != -1)
	{
		if (_LoadSceneInternal(_loadScene) != ENGINE_OK)
		{
			DIE("Failed to load scene");
		}
		_loadScene = -1;
	}

	if (_activeScene)
		_activeScene->Update(deltaTime);
}

void SceneManager::_UnloadScene() noexcept
{
	if (_activeScene == nullptr)
		return;

	_activeScene->Unload();
	_activeScene = nullptr;
}

void SceneManager::_UnloadScenes() noexcept
{
	for (Scene *s : _scenes)
	{
		if (s->IsLoaded())
			s->Unload();

		delete s;
	}

	_scenes.clear();
}

int SceneManager::_LoadSceneInternal(int id)
{
	Logger::Log(SM_MODULE, LOG_INFORMATION, "Loading scene id=%d", id);
	Scene *scn = nullptr;

	for (Scene *s : _scenes)
	{
		if (s->GetId() == id)
			scn = s;
	}

	if (scn == nullptr)
		return ENGINE_NOT_FOUND;

	if ((_loadingScreen = new LoadingScreen(scn->GetLoadingScreenTexture())) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;
	
	if (Engine::IsEditor())
		return _LoadSceneWorker(scn);

	_loadThread = new thread(_LoadSceneWorker, scn);
	return ENGINE_OK;
}

int SceneManager::_LoadSceneWorker(Scene *scn)
{
	Engine::GetRenderer()->MakeCurrent(R_LOAD_CONTEXT);
	RFence *fence = Engine::GetRenderer()->CreateFence();

	if (scn == nullptr)
		return ENGINE_NOT_FOUND;

	_loadingScene = scn;

	if (_activeScene != nullptr)
		_UnloadScene();

	int ret = scn->Load();

	fence->Wait();
	delete fence;

	if (ret == ENGINE_OK)
		_activeScene = scn;
	else
		scn = nullptr;

	_loadingScene = nullptr;

	return ret;
}

void SceneManager::Release() noexcept
{
	_UnloadScenes();
	
	Logger::Log(SM_MODULE, LOG_INFORMATION, "Released");
}
