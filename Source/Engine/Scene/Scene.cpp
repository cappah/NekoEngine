/* NekoEngine
 *
 * Scene.cpp
 * Author: Alexandru Naiman
 *
 * Scene class implementation 
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
#include <iterator>
#include <algorithm>
#include <string.h>

#include <Engine/Engine.h>
#include <Engine/ResourceManager.h>
#include <Engine/SoundManager.h>
#include <Engine/EventManager.h>
#include <Engine/GameModule.h>
#include <Engine/CameraManager.h>
#include <Runtime/Runtime.h>
#include <Scene/Scene.h>
#include <Scene/Object.h>
#include <Physics/Physics.h>
#include <Profiler/Profiler.h>
#include <Renderer/Renderer.h>
#include <Renderer/DebugMarker.h>
#include <System/VFS/VFS.h>
#include <System/AssetLoader/AssetLoader.h>
#include <Scene/Components/TerrainComponent.h>
#include <Scene/Components/SkysphereComponent.h>
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

Object *Scene::GetObjectByID(uint32_t id)
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
	initializer.position = initializer.rotation = vec3(0.f);
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

		if (split[0] == "name")
			initializer.name = *split[1];
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
		
		StaticMeshComponent *stcomp = dynamic_cast<StaticMeshComponent*>(comp);
		SkeletalMeshComponent *skcomp = dynamic_cast<SkeletalMeshComponent*>(comp);
		TerrainComponent *tcomp = dynamic_cast<TerrainComponent*>(comp);
		SkysphereComponent *skycomp = dynamic_cast<SkysphereComponent*>(comp);
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
		else if (skycomp)
		{
			string &name = stcomp->GetMesh()->GetResourceInfo()->name;

			if (name.length() && (find(_loadedMeshIds.begin(), _loadedMeshIds.end(), name) == _loadedMeshIds.end()))
			{
				_bufferSize += stcomp->GetMesh()->GetRequiredMemorySize();
				_loadedMeshIds.push_back(name);
			}
		}
		else if (stcomp)
		{
			if (stcomp->GetMesh()->GetResourceInfo())
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

		if (split[0].Contains("position"))
			AssetLoader::ReadFloatArray(*split[1], 3, &initInfo->initializer.position.x);
		else if (split[0].Contains("rotation"))
			AssetLoader::ReadFloatArray(*split[1], 3, &initInfo->initializer.rotation.x);
		else if (split[0].Contains("scale"))
			AssetLoader::ReadFloatArray(*split[1], 3, &initInfo->initializer.scale.x);
		else
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

			_objects.push_back(obj);

			if (obj->GetTransformedBounds().IsValid())
				_ocTree->Add(obj);
		}
		else if (lineBuff.Contains("SceneInfo"))
			_LoadSceneInfo(f);
	}

	f->Close();

	// Upload mesh data

	uint64_t uboSize = _objects.size() * sizeof(ObjectData);

	if (uboSize)
	{
		_sceneUbo = new Buffer(uboSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (!_sceneUbo)
		{ DIE("Out of resources"); }
		VK_DBG_SET_OBJECT_NAME((uint64_t)_sceneUbo->GetHandle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, "Scene uniform buffer");
	}

	if (_bufferSize)
	{
		_sceneBuffer = new Buffer(_bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (!_sceneBuffer)
		{ DIE("Out of resources"); }
		VK_DBG_SET_OBJECT_NAME((uint64_t)_sceneBuffer->GetHandle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, "Scene vertex/index buffer");
	}

	uint64_t offset = 0, uboOffset = 0;

	for (Object *obj : _objects)
	{
		Buffer *buffer{ nullptr }, *ubo{ nullptr };

		for (SkeletalMeshComponent *skmesh : obj->GetComponentsOfType<SkeletalMeshComponent>())
		{
			ubo = new Buffer(_sceneUbo, uboOffset, sizeof(ObjectData));
			skmesh->SetUniformBuffer(ubo);
			uboOffset += sizeof(ObjectData);

			if (skmesh->GetMeshID() == SM_GENERATED || skmesh->GetMesh()->IsResident())
				continue;

			buffer = new Buffer(_sceneBuffer, offset, skmesh->GetMesh()->GetRequiredMemorySize());
			skmesh->GetMesh()->Upload(buffer);
			offset += skmesh->GetMesh()->GetRequiredMemorySize();
		}

		for (TerrainComponent *tcomp : obj->GetComponentsOfType<TerrainComponent>())
		{
			ubo = new Buffer(_sceneUbo, uboOffset, sizeof(ObjectData));
			tcomp->SetUniformBuffer(ubo);
			uboOffset += sizeof(ObjectData);

			if (tcomp->GetMesh()->IsResident())
				continue;

			buffer = new Buffer(_sceneBuffer, offset, tcomp->GetRequiredMemorySize());
			tcomp->Upload(buffer);
			offset += tcomp->GetRequiredMemorySize();
		}

		for (SkysphereComponent *skycomp : obj->GetComponentsOfType<SkysphereComponent>())
		{
			ubo = new Buffer(_sceneUbo, uboOffset, sizeof(ObjectData));
			skycomp->SetUniformBuffer(ubo);
			uboOffset += sizeof(ObjectData);

			if (skycomp->GetMesh()->IsResident())
				continue;

			buffer = new Buffer(_sceneBuffer, offset, skycomp->GetRequiredMemorySize());
			skycomp->Upload(buffer);
			offset += skycomp->GetRequiredMemorySize();
		}

		for (StaticMeshComponent *stmesh : obj->GetComponentsOfType<StaticMeshComponent>())
		{
			ubo = new Buffer(_sceneUbo, uboOffset, sizeof(ObjectData));
			stmesh->SetUniformBuffer(ubo);
			uboOffset += sizeof(ObjectData);

			if (stmesh->GetMeshID() == SM_GENERATED || stmesh->GetMesh()->IsResident())
				continue;

			buffer = new Buffer(_sceneBuffer, offset, stmesh->GetMesh()->GetRequiredMemorySize());
			stmesh->GetMesh()->Upload(buffer);
			offset += stmesh->GetMesh()->GetRequiredMemorySize();
		}
		
		// at least one mesh component exists
		if (ubo) obj->BuildCommandBuffers();
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

	EventManager::Broadcast(NE_EVT_SCN_LOADED, this);

	Physics::GetInstance()->InitScene(BroadphaseType::SAP, 1000.f, 15000000u);

	Logger::Log(SCENE_MODULE, LOG_INFORMATION, "Scene %s, id=%d loaded with %d %s and %d %s", *_name, _id, _objects.size(), _objects.size() > 1 ? "objects" : "object", CameraManager::Count(), CameraManager::Count() > 1 ? "cameras" : "camera");

	return ENGINE_OK;
}

void Scene::Update(double deltaTime) noexcept
{	
	for (Object *obj : _objects)
	{
		if (!obj->GetUpdateWhilePaused() && Engine::IsPaused())
			continue;
		obj->Update(deltaTime);
	}

	for (Object *obj : _newObjects)
	{
		if (!obj->GetUpdateWhilePaused() && Engine::IsPaused())
			continue;
		obj->Update(deltaTime);
	}

	for (Object *obj : _newObjects)
	{
		_objects.push_back(obj);
		_ocTree->Add(obj);
	}
	_newObjects.clear();

	vector<Object *> tmp;
	for (Object *obj : _deletedObjects)
	{
		if (obj->CanUnload())
		{
			_objects.erase(remove(_objects.begin(), _objects.end(), obj), _objects.end());
			_ocTree->Remove(obj);
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
	for (Object *obj : _objects)
		obj->UpdateData(buffer);
}

void Scene::DrawShadow(VkCommandBuffer buffer, uint32_t shadowId) noexcept
{
	for (const Object *obj : _objects)
		obj->DrawShadow(buffer, shadowId);
}

void Scene::PrepareCommandBuffers()
{
	NArray<const Object *> visibleObjects(_objects.size());
	vector<Drawable *> opaqueDrawables{};
	vector<Drawable *> transparentDrawables{};
	Camera *cam{ CameraManager::GetActiveCamera() };
	float minDistance = FLT_MAX;
	
	PROF_BEGIN("Culling", vec3(1.f, 0.f, 0.f));

	for (Object *obj : _objects)
		if (obj->GetNoCull()) visibleObjects.Add(obj);

	_ocTree->GetVisible(cam->GetFrustum(), visibleObjects);
	PROF_MARKER("Objects", vec3(1.f, 0.f, 0.f));
	
	for (const Object *obj : visibleObjects)
	{
		NArray<Drawable> *drawables = obj->GetDrawables();

		if (!drawables)
			continue;

		for (Drawable &drawable : *drawables)
		{
			if (!cam->GetFrustum().ContainsBounds(drawable.transformedBounds))
				continue;

			if (drawable.transparent)
			{
				float dist = distance(drawable.bounds.GetCenter(), cam->GetPosition());

				if (dist < minDistance)
				{
					minDistance = dist;
					transparentDrawables.insert(transparentDrawables.begin(), &drawable);
				}
				else
					transparentDrawables.push_back(&drawable);
			}
			else
				opaqueDrawables.push_back(&drawable);
		}
	}

	PROF_MARKER("Drawables", vec3(1.f, 0.f, 0.f));
	PROF_END();

	for (Drawable *drawable : opaqueDrawables)
	{
		if (drawable->depthCommandBuffer != VK_NULL_HANDLE)
			Renderer::GetInstance()->AddDepthCommandBuffer(drawable->depthCommandBuffer);
		Renderer::GetInstance()->AddSceneCommandBuffer(drawable->sceneCommandBuffer);

		#if defined(NE_CONFIG_DEBUG) || defined(NE_CONFIG_DEVELOPMENT)
		if (Engine::GetDebugVariables().DrawBounds)
			Renderer::GetInstance()->DrawBounds(drawable->transformedBounds);
		#endif
	}

	for (Drawable *drawable : reverse(transparentDrawables))
	{
		if (drawable->depthCommandBuffer != VK_NULL_HANDLE)
			Renderer::GetInstance()->AddDepthCommandBuffer(drawable->depthCommandBuffer);
		Renderer::GetInstance()->AddSceneCommandBuffer(drawable->sceneCommandBuffer);

		#if defined(NE_CONFIG_DEBUG) || defined(NE_CONFIG_DEVELOPMENT)
		if (Engine::GetDebugVariables().DrawBounds)
			Renderer::GetInstance()->DrawBounds(drawable->transformedBounds);
		#endif
	}
}

bool Scene::RebuildCommandBuffers()
{
	for (Object *obj : _objects)
		if (!obj->RebuildCommandBuffers())
			return false;

	for (Object *obj : _newObjects)
		if (!obj->RebuildCommandBuffers())
			return false;

	return true;
}

void Scene::Unload() noexcept
{
	if (!_loaded)
		return;

	_threadPool->Stop();
	_threadPool->Join();

	for (Object *obj : _objects)
	{
		obj->Unload();
		delete obj;
	}
	_objects.clear();

	if (_bgMusic >= 0)
	{
		SoundManager::StopBackgroundMusic();
		SoundManager::SetBackgroundMusic(BG_MUSIC_NONE);
	}

	delete _sceneUbo; _sceneUbo = nullptr;
	delete _sceneBuffer; _sceneBuffer = nullptr;
	delete _threadPool; _threadPool = nullptr;
	delete _ocTree; _ocTree = nullptr;
	
	_loaded = false;

	EventManager::Broadcast(NE_EVT_SCN_UNLOADED, this);
}

void Scene::AddObject(Object *obj) noexcept
{
	if (_loaded)
		_newObjects.push_back(obj);
	else
		_objects.push_back(obj);

	EventManager::Broadcast(NE_EVT_OBJ_ADDED, obj);
}

void Scene::RemoveObject(Object *obj) noexcept
{
	if (find(_deletedObjects.begin(), _deletedObjects.end(), obj) == _deletedObjects.end())
		_deletedObjects.push_back(obj);

	EventManager::Broadcast(NE_EVT_OBJ_REMOVED, obj);
}

Scene::~Scene() noexcept
{
	Unload();

	delete _threadPool;
}
