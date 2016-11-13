/* NekoEngine
 *
 * Scene.cpp
 * Author: Alexandru Naiman
 *
 * Scene class implementation 
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

#include <fstream>
#include <iterator>
#include <algorithm>
#include <string.h>

#include <Engine/Engine.h>
#include <Engine/ResourceManager.h>
#include <Engine/SoundManager.h>
#include <Engine/GameModule.h>
#include <Engine/CameraManager.h>
#include <Runtime/Runtime.h>
#include <Scene/Scene.h>
#include <Scene/Object.h>
#include <Scene/Skysphere.h>
#include <Renderer/Debug.h>
#include <Renderer/Renderer.h>
#include <System/VFS/VFS.h>
#include <System/AssetLoader/AssetLoader.h>
#include <Scene/Components/TerrainComponent.h>
#include <Scene/Components/StaticMeshComponent.h>
#include <Scene/Components/SkeletalMeshComponent.h>

#define SCENE_LINE_BUFF		1024
#define SCENE_MODULE		"Scene"

using namespace std;
using namespace glm;

typedef struct COMPONNENT_INITIALIZER_INFO
{
	string name;
	string className;
	ComponentInitializer initializer;
} ComponentInitInfo;

Object *Scene::GetObjectByID(int32_t id)
{
	for (Object *obj : _objects)
		if (obj->GetId() == id)
			return obj;
	return nullptr;
}

Object *Scene::GetObjectByName(const char *name)
{
	for (Object *obj : _objects)
		if (obj->GetName() == name)
			return obj;
	return nullptr;
}

Object *Scene::_LoadObject(VFSFile *f, NString &className)
{
	NString lineBuff(SCENE_LINE_BUFF);

	ObjectInitializer initializer;
	initializer.position = initializer.rotation = initializer.color = vec3(0.f);
	initializer.scale = vec3(1.f);

	vector<ComponentInitInfo> componentInitInfo;
	
	vec3 vec;

	while (!f->EoF())
	{
		lineBuff.Clear();
		f->Gets(*lineBuff, SCENE_LINE_BUFF);

		if (lineBuff.IsEmpty())
			continue;

		lineBuff.RemoveComment();
		lineBuff.RemoveNewLine();

		if (lineBuff.IsEmpty())
			continue;

		lineBuff.Count();

		if (lineBuff == "EndObject")
			break;

		NArray<NString> split = lineBuff.Split('=');

		if (split.Count() < 2)
			continue;

		if (split[0] == "id")
			initializer.id = atoi(*split[1]);
		else if (split[0] == "parent")
			initializer.parent = GetObjectByID(atoi(*split[1]));
		else if (split[0] == "color")
			AssetLoader::ReadFloatArray(*split[1], 3, &initializer.color.x);
		else if (split[0] == "position")
			AssetLoader::ReadFloatArray(*split[1], 3, &initializer.position.x);
		else if (split[0] == "rotation")
			AssetLoader::ReadFloatArray(*split[1], 3, &initializer.rotation.x);
		else if (split[0] == "scale")
			AssetLoader::ReadFloatArray(*split[1], 3, &initializer.scale.x);
		else if (split[0] == "Component")
		{
			ComponentInitInfo info;
			info.name = *split[2];
			info.className = *split[1];
			
			_LoadComponent(f, &info);
			
			componentInitInfo.push_back(info);
		}
		else
			initializer.arguments.insert(make_pair(*split[0], *split[1]));
	}
	
	Object *obj = nullptr;
	
	obj = Engine::NewObject(*className, &initializer);
	
	if (!obj)
	{
		Logger::Log(SCENE_MODULE, LOG_CRITICAL, "NewObject() call failed for class %s", *className);
		return nullptr;
	}
	
	for(ComponentInitInfo &info : componentInitInfo)
	{
		info.initializer.parent = obj;
		ObjectComponent *comp = Engine::NewComponent(info.className, &info.initializer);
		
		if (!comp || comp->Load() != ENGINE_OK)
		{
			Logger::Log(SCENE_MODULE, LOG_CRITICAL, "Failed to load component %s of type %s for object id %d", info.name.c_str(), info.className.c_str(), obj->GetId());
			delete obj;
			return nullptr;
		}
		
		SkeletalMeshComponent *skcomp = dynamic_cast<SkeletalMeshComponent*>(comp);
		TerrainComponent *tcomp = dynamic_cast<TerrainComponent*>(comp);
		if (skcomp)
		{
			string &name = skcomp->GetMesh()->GetResourceInfo()->name;

			if (find(_loadedMeshIds.begin(), _loadedMeshIds.end(), name) == _loadedMeshIds.end())
			{
				_bufferSize += skcomp->GetMesh()->GetRequiredMemorySize();
				_loadedMeshIds.push_back(name);
			}
		}
		else if (tcomp)
			_bufferSize += tcomp->GetRequiredMemorySize();
		else
		{
			StaticMeshComponent *stcomp = dynamic_cast<StaticMeshComponent*>(comp);
			if (stcomp && stcomp->GetMesh()->GetResourceInfo())
			{
				string &name = stcomp->GetMesh()->GetResourceInfo()->name;

				if (name.length() && (find(_loadedMeshIds.begin(), _loadedMeshIds.end(), name) == _loadedMeshIds.end()))
				{
					_bufferSize += stcomp->GetMesh()->GetRequiredMemorySize();
					_loadedMeshIds.push_back(name);
				}
			}
		}
		
		obj->AddComponent(info.name.c_str(), comp);
	}
	
	if (obj->Load() != ENGINE_OK)
	{
		Logger::Log(SCENE_MODULE, LOG_CRITICAL, "Failed to load object id %d", obj->GetId());
		delete obj;
		return nullptr;
	}

	return obj;
}

void Scene::_LoadSceneInfo(VFSFile *f)
{
	NString lineBuff(SCENE_LINE_BUFF);

	vec4 ambient = vec4(1.f, 1.f, 1.f, .2f);

	while (!f->EoF())
	{
		lineBuff.Clear();
		f->Gets(*lineBuff, SCENE_LINE_BUFF);

		if (lineBuff.IsEmpty())
			continue;

		AssetLoader::RemoveComment(*lineBuff);
		AssetLoader::RemoveNewline(*lineBuff);

		if (lineBuff.IsEmpty())
			continue;

		lineBuff.Count();

		if (lineBuff == "EndSceneInfo")
			break;

		NArray<NString> split = lineBuff.Split('=');

		if (split.Count() != 2)
			continue;

		if (split[0] == "name")
			_name = split[1];
		else if (split[0] == "bgmusic")
		{
			_bgMusic = ResourceManager::GetResourceID(*split[1], ResourceType::RES_AUDIOCLIP);
			if (_bgMusic == ENGINE_NOT_FOUND)
				Logger::Log(SCENE_MODULE, LOG_CRITICAL, "Failed to load background music for scene id %d. Audioclip \"%s\" not found.", _id, *split[1]);
		}
		else if (split[0] == "bgmusicvol")
			_bgMusicVolume = (float)split[1];
		else if (split[0] == "ambcolor")
			AssetLoader::ReadFloatArray(*split[1], 3, &ambient.x);
		else if (split[0] == "ambintensity")
			ambient.w = (float)split[1];
		else if (split[0] == "gamemodule")
		{
			if (!Engine::GetGameModule())
			{
				Logger::Log(SCENE_MODULE, LOG_CRITICAL, "Scene id=%d requires %s game module, but no game module is available", _id, *split[1]);
				DIE("No game module loaded. Please check the log file for details.\nThe program cannot continue.");
			}

			if (split[1] != Engine::GetGameModule()->GetModuleName())
			{
				Logger::Log(SCENE_MODULE, LOG_CRITICAL, "Scene id=%d requires %s game module, but %s is loaded", _id, *split[1], Engine::GetGameModule()->GetModuleName());
				DIE("Wrong game module loaded. Please check the log file for details.\nThe program cannot continue.");
			}
		}
	}

	Renderer::GetInstance()->SetAmbientColor(ambient.r, ambient.g, ambient.b, ambient.w);
}

void Scene::_LoadComponent(VFSFile *f, ComponentInitInfo *initInfo)
{
	NString lineBuff(SCENE_LINE_BUFF);

	while (!f->EoF())
	{
		lineBuff.Clear();
		f->Gets(*lineBuff, SCENE_LINE_BUFF);

		if (lineBuff.IsEmpty())
			continue;

		AssetLoader::RemoveComment(*lineBuff);
		AssetLoader::RemoveNewline(*lineBuff);

		if (lineBuff.IsEmpty())
			continue;

		lineBuff.Count();

		if (lineBuff == "EndComponent")
			break;

		NArray<NString> split = lineBuff.Split('=');

		if (split.Count() != 2)
			continue;

		char *ptr = *split[0];

		// skip tabs
		while (*ptr == '\t') ptr++;

		initInfo->initializer.arguments.insert(make_pair(ptr, *split[1]));
	}
}

size_t Scene::GetVertexCount() noexcept
{
	size_t verts = 0;

	for (Object *o : _objects)
		verts += o->GetVertexCount();

	return verts;
}

size_t Scene::GetTriangleCount() noexcept
{
	size_t tris = 0;

	for (Object *o : _objects)
		tris += o->GetTriangleCount();

	return tris;
}

int Scene::Load()
{
	NString lineBuff(SCENE_LINE_BUFF);

	NString path("/");
	path.Append(_sceneFile);

	_bufferSize = 0;

	VFSFile *f = VFS::Open(path);
	if (!f)
	{
		Logger::Log(SCENE_MODULE, LOG_CRITICAL, "Failed to open scene file");
		return ENGINE_IO_FAIL;
	}

	char header[9];
	if (f->Read(header, sizeof(char), 8) != 8)
	{
		Logger::Log(SCENE_MODULE, LOG_CRITICAL, "Failed to read scene file");
		return ENGINE_IO_FAIL;
	}
	header[8] = 0x0;

	if (strncmp(header, "NSCENE1 ", 8))
	{
		Logger::Log(SCENE_MODULE, LOG_CRITICAL, "Invalid scene file header");
		return ENGINE_INVALID_RES;
	}

	while (!f->EoF())
	{
		lineBuff.Clear();
		f->Gets(lineBuff, SCENE_LINE_BUFF);

		if (lineBuff.IsEmpty())
			continue;

		lineBuff.RemoveComment();
		lineBuff.RemoveNewLine();

		if (lineBuff[0] == 0x0)
			continue;

		if (lineBuff.Contains("Object"))
		{
			NArray<NString> split = lineBuff.Split('=');
			NString className = "Object";

			if (split.Count() == 2)
				className = split[1];

			Object *obj = _LoadObject(f, className);

			if (!obj)
			{
				f->Close();
				return ENGINE_FAIL;
			}

			Skysphere *s = dynamic_cast<Skysphere *>(obj);

			if (!_skysphere && s)
				_skysphere = s;
			else
				_objects.push_back(obj);
		}
		else if (lineBuff.Contains("SceneInfo"))
			_LoadSceneInfo(f);
	}

	f->Close();

	// Upload mesh data

	uint64_t uboSize = (_objects.size() + (_skysphere ? 1 : 0)) * sizeof(ObjectData);

	if (uboSize)
	{
		_sceneUbo = new Buffer(uboSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (!_sceneUbo)
		{ DIE("Out of resources"); }
		DBG_SET_OBJECT_NAME((uint64_t)_sceneUbo->GetHandle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, "Scene uniform buffer");
	}

	if (_bufferSize)
	{
		_sceneBuffer = new Buffer(_bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (!_sceneBuffer)
		{ DIE("Out of resources"); }
		DBG_SET_OBJECT_NAME((uint64_t)_sceneBuffer->GetHandle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, "Scene vertex/index buffer");
	}

	uint64_t offset = 0, uboOffset = 0;
	if (_skysphere)
	{
		StaticMeshComponent *skyMesh = (StaticMeshComponent *)_skysphere->GetComponent("Mesh");

		skyMesh->GetMesh()->Upload(new Buffer(_sceneBuffer, offset, skyMesh->GetMesh()->GetRequiredMemorySize()));
		offset += skyMesh->GetMesh()->GetRequiredMemorySize();

		_skysphere->SetUniformBuffer(new Buffer(_sceneUbo, uboOffset, sizeof(ObjectData)));
		uboOffset += sizeof(ObjectData);
		
		_skysphere->BuildCommandBuffers();
		_skysphere->RegisterCommandBuffers();
	}

	for (Object *obj : _objects)
	{
		Buffer *buffer = nullptr;

		for (SkeletalMeshComponent *skmesh : obj->GetComponentsOfType<SkeletalMeshComponent>())
		{
			if (skmesh->GetMeshID() == SM_GENERATED || skmesh->GetMesh()->IsResident())
				continue;

			buffer = new Buffer(_sceneBuffer, offset, skmesh->GetMesh()->GetRequiredMemorySize());
			skmesh->GetMesh()->Upload(buffer);
			offset += skmesh->GetMesh()->GetRequiredMemorySize();
		}

		for (TerrainComponent *tcomp : obj->GetComponentsOfType<TerrainComponent>())
		{
			if (tcomp->GetMesh()->IsResident())
				continue;

			buffer = new Buffer(_sceneBuffer, offset, tcomp->GetRequiredMemorySize());
			tcomp->Upload(buffer);
			offset += tcomp->GetRequiredMemorySize();
		}

		for (StaticMeshComponent *stmesh : obj->GetComponentsOfType<StaticMeshComponent>())
		{
			if (stmesh->GetMeshID() == SM_GENERATED || stmesh->GetMesh()->IsResident())
				continue;

			buffer = new Buffer(_sceneBuffer, offset, stmesh->GetMesh()->GetRequiredMemorySize());
			stmesh->GetMesh()->Upload(buffer);
			offset += stmesh->GetMesh()->GetRequiredMemorySize();
		}
		
		obj->SetUniformBuffer(new Buffer(_sceneUbo, uboOffset, sizeof(ObjectData)));
		uboOffset += sizeof(ObjectData);

		obj->BuildCommandBuffers();
		obj->RegisterCommandBuffers();
	}

	if (!CameraManager::Count())
	{
		Unload();
		Logger::Log(SCENE_MODULE, LOG_CRITICAL, "Load failed for scene id=%d: no camera found", _id);
		return ENGINE_NO_CAMERA;
	}

	_loaded = true;

	if (_bgMusic >= 0)
	{
		if (SoundManager::SetBackgroundMusic(_bgMusic) == ENGINE_OK)
		{
			SoundManager::SetBackgroundMusicVolume(_bgMusicVolume);
			SoundManager::PlayBackgroundMusic();
		}
		else
			Logger::Log(SCENE_MODULE, LOG_WARNING, "Failed to load background music id=%d for scene id=%d", _bgMusic, _id);
	}

	_loadedMeshIds.clear();

	Logger::Log(SCENE_MODULE, LOG_INFORMATION, "Scene %s, id=%d loaded with %d %s and %d %s", *_name, _id, _objects.size(), _objects.size() > 1 ? "objects" : "object", CameraManager::Count(), CameraManager::Count() > 1 ? "cameras" : "camera");

	return ENGINE_OK;
}

void Scene::Update(double deltaTime) noexcept
{	
	for (size_t i = 0; i < _objects.size(); i++)
	{
		if (!Engine::IsPaused() || _objects[i]->GetUpdateWhilePaused())
		{
			Object *obj = _objects[i];
			_threadPool->Enqueue([obj, deltaTime]() {
				obj->Update(deltaTime);
			});
		}
	}

	_threadPool->Wait();

	for (Object *obj : _newObjects)
		_objects.push_back(obj);
	_newObjects.clear();

	vector<Object *> tmp;
	for (Object *obj : _deletedObjects)
	{
		if (obj->CanUnload())
		{
			_objects.erase(remove(_objects.begin(), _objects.end(), obj), _objects.end());
			delete obj;
		}
		else
			tmp.push_back(obj);
	}
	_deletedObjects.clear();

	for (Object *obj : tmp)
		_deletedObjects.push_back(obj);
}

void Scene::UpdateData(VkCommandBuffer buffer) noexcept
{
	if (_skysphere)
		_skysphere->UpdateData(buffer);

	for (Object *obj : _objects)
		obj->UpdateData(buffer);
}

void Scene::PrepareCommandBuffers()
{
	std::map<float, Object *> sortedList;
	Camera *cam = CameraManager::GetActiveCamera();

	if (_skysphere)
		_skysphere->RegisterCommandBuffers();

	for (Object *obj : _objects)
	{
		_threadPool->Enqueue([obj, cam]() {
			if (obj->GetNoCull() || distance(cam->GetPosition(), obj->GetPosition()) < cam->GetViewDistance())
				obj->RegisterCommandBuffers();
		});
	}

	_threadPool->Wait();

	for (Object *obj : _transparentObjects)
	{
		float d = distance(CameraManager::GetActiveCamera()->GetPosition(), obj->GetPosition());
		if(obj->GetNoCull() || d < cam->GetViewDistance())
			sortedList[d] = obj;
	}

	for (std::map<float, Object *>::reverse_iterator it = sortedList.rbegin(); it != sortedList.rend(); ++it)
		it->second->RegisterCommandBuffers();
}

void Scene::Unload() noexcept
{
	if (!_loaded)
		return;

	_threadPool->Stop();
	_threadPool->Join();

	delete _skysphere;
	_skysphere = nullptr;

	for (Object *obj : _objects)
		delete obj;
	_objects.clear();

	if (_bgMusic >= 0)
	{
		SoundManager::StopBackgroundMusic();
		SoundManager::SetBackgroundMusic(BG_MUSIC_NONE);
	}

	delete _sceneUbo; _sceneUbo = nullptr;
	delete _sceneBuffer; _sceneBuffer = nullptr;
	
	_loaded = false;
}

void Scene::AddObject(Object *obj) noexcept
{
	if (_loaded)
		_newObjects.push_back(obj);
	else
		_objects.push_back(obj);
}

void Scene::RemoveObject(Object *obj) noexcept
{
	if (find(_deletedObjects.begin(), _deletedObjects.end(), obj) == _deletedObjects.end())
		_deletedObjects.push_back(obj);
}

Scene::~Scene() noexcept
{
	Unload();

	delete _threadPool;
}
