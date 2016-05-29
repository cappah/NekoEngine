/* Neko Engine
 *
 * SceneManager.cpp
 * Author: Alexandru Naiman
 *
 * SceneManager class implementation 
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

#define ENGINE_INTERNAL

#include <fstream>

#include <Engine/Engine.h>
#include <Scene/Scene.h>
#include <System/Logger.h>
#include <Engine/EngineUtils.h>
#include <Engine/SceneManager.h>
#include <Engine/LoadingScreen.h>
#include <System/VFS/VFS.h>

#define LINE_BUFF	1024
#define SM_MODULE	"SceneManager"

using namespace std;

std::vector<Scene*> SceneManager::_scenes;
Scene* SceneManager::_activeScene = nullptr;
int SceneManager::_defaultScene = 0;
LoadingScreen* SceneManager::_loadingScreen = nullptr;

int SceneManager::Initialize()
{
	return _ReadConfigFile("/scenes.cfg");
}

int SceneManager::_ReadConfigFile(string file)
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

	if (_activeScene != nullptr)
		_UnloadScene();

	int ret = scn->Load();

	if (ret == ENGINE_OK)
		_activeScene = scn;
	else
		scn = nullptr;

	delete _loadingScreen;
	_loadingScreen = nullptr;

	return ret;
}

int SceneManager::LoadNextScene()
{
	static unsigned int scene = 0;

	scene++;

	if (scene >= _scenes.size())
		scene = 0;

	return LoadScene(scene);
}

int SceneManager::DrawScene(RShader* shader) noexcept
{
	if (_activeScene == nullptr)
	{
		if (_loadingScreen == nullptr)
		{
			if((_loadingScreen = new LoadingScreen(LS_DEFAULT_TEXTURE)) == nullptr)
				return ENGINE_OUT_OF_RESOURCES;
		}
		
		_loadingScreen->Draw();
	}
	else
		_activeScene->Draw(shader);

	return ENGINE_OK;
}

void SceneManager::UpdateScene(double deltaTime) noexcept
{
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

void SceneManager::Release() noexcept
{
	_UnloadScenes();
	
	Logger::Log(SM_MODULE, LOG_INFORMATION, "Released");
}
