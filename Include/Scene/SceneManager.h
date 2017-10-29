/* NekoEngine
 *
 * SceneManager.h
 * Author: Alexandru Naiman
 *
 * SceneManager class definition
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

#include <vector>
#include <string>
#include <thread>

#include <Runtime/Runtime.h>
#include <Engine/Engine.h>
#include <Scene/LoadingScreen.h>
#include <Scene/Scene.h>

class SceneManager
{
public:
	ENGINE_API static int Initialize();

	ENGINE_API static Scene *GetActiveScene() noexcept { return _activeScene; }
	ENGINE_API static Scene *GetLoadingScene() noexcept { return _loadingScene; }

	ENGINE_API static int LoadScene(int id);
	ENGINE_API static int LoadDefaultScene() { return LoadScene(_defaultScene); }
	ENGINE_API static int LoadNextScene();
	ENGINE_API static void UpdateScene(double deltaTime) noexcept;
	ENGINE_API static bool IsSceneLoaded() noexcept { return _activeScene && _activeScene->IsLoaded() ? true : false; }

	ENGINE_API static void Release() noexcept;
	
private:
	static std::vector<Scene*> _scenes;
	static Scene *_activeScene, *_loadingScene;
	static int _defaultScene, _loadScene;
	static std::thread *_loadThread;

	static LoadingScreen *_loadingScreen;

	static int _ReadConfigFile(NString configFile);
	static void _UnloadScene() noexcept;
	static void _UnloadScenes() noexcept;
	static int _LoadSceneInternal(int id);
	static int _LoadSceneWorker(Scene *scn);

	SceneManager() { }
};