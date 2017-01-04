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
#include <string.h>

#include <glm/glm.hpp>

#include <Engine/Engine.h>
#include <Engine/EngineUtils.h>
#include <Engine/ResourceManager.h>
#include <Engine/SoundManager.h>
#include <Engine/GameModule.h>
#include <Engine/DeferredBuffer.h>
#include <Engine/CameraManager.h>
#include <Runtime/Runtime.h>
#include <Scene/Scene.h>
#include <Scene/Object.h>
#include <Scene/Terrain.h>
#include <Scene/Skybox.h>
#include <Scene/Light.h>
#include <System/VFS/VFS.h>
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

Object *Scene::_LoadObject(VFSFile *f, const string &className)
{
	char lineBuff[SCENE_LINE_BUFF];
	memset(lineBuff, 0x0, SCENE_LINE_BUFF);
	
	ObjectInitializer initializer;
	initializer.position = initializer.rotation = initializer.color = vec3(0.f);
	initializer.scale = vec3(1.f);

	vector<ComponentInitInfo> componentInitInfo;
	
	vec3 vec;

	while (!f->EoF())
	{
		memset(lineBuff, 0x0, SCENE_LINE_BUFF);
		f->Gets(lineBuff, SCENE_LINE_BUFF);

		if (lineBuff[0] == 0x0)
			continue;

		EngineUtils::RemoveComment(lineBuff);
		EngineUtils::RemoveNewline(lineBuff);

		if (lineBuff[0] == 0x0)
			continue;

		if (strstr(lineBuff, "EndObject"))
			break;

		vector<char*> split = EngineUtils::SplitString(lineBuff, '=');

		if (split.size() < 2)
			continue;

		size_t len = strlen(split[0]);

		if (!strncmp(split[0], "id", len))
			initializer.id = atoi(split[1]);
		else if (!strncmp(split[0], "color", len))
			EngineUtils::ReadFloatArray(split[1], 3, &initializer.color.x);
		else if (!strncmp(split[0], "position", len))
			EngineUtils::ReadFloatArray(split[1], 3, &initializer.position.x);
		else if (!strncmp(split[0], "rotation", len))
			EngineUtils::ReadFloatArray(split[1], 3, &initializer.rotation.x);
		else if (!strncmp(split[0], "scale", len))
			EngineUtils::ReadFloatArray(split[1], 3, &initializer.scale.x);
		else if (!strncmp(split[0], "Component", len))
		{
			ComponentInitInfo info;
			info.name = split[2];
			info.className = split[1];
			
			_LoadComponent(f, &info);
			
			componentInitInfo.push_back(info);
		}
		else
			initializer.arguments.insert(make_pair(split[0], split[1]));

		for (char* c : split)
			free(c);
	}
	
	Object *obj = nullptr;
	
	obj = Engine::NewObject(className, &initializer);
	
	if (!obj)
	{
		Logger::Log(SCENE_MODULE, LOG_CRITICAL, "NewObject() call failed for class %s", className.c_str());
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
		if (skcomp)
		{
			if (Engine::GetRenderer()->HasCapability(RendererCapability::DrawBaseVertex))
			{
				skcomp->GetMesh()->SetVertexOffset(_sceneVertices.size());
				skcomp->GetMesh()->SetIndexOffset(_sceneIndices.size());
				
				_AddVertices(skcomp->GetMesh()->GetVertices());
				_AddIndices(skcomp->GetMesh()->GetIndices());
			}
		}
		else
		{
			StaticMeshComponent *stcomp = dynamic_cast<StaticMeshComponent*>(comp);
			if (stcomp)
			{
				if (Engine::GetRenderer()->HasCapability(RendererCapability::DrawBaseVertex))
				{
					stcomp->GetMesh()->SetVertexOffset(_sceneVertices.size());
					stcomp->GetMesh()->SetIndexOffset(_sceneIndices.size());
					
					_AddVertices(stcomp->GetMesh()->GetVertices());
					_AddIndices(stcomp->GetMesh()->GetIndices());
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
	char lineBuff[SCENE_LINE_BUFF];
	memset(lineBuff, 0x0, SCENE_LINE_BUFF);

	while (!f->EoF())
	{
		memset(lineBuff, 0x0, SCENE_LINE_BUFF);
		f->Gets(lineBuff, SCENE_LINE_BUFF);

		if (lineBuff[0] == 0x0)
			continue;

		EngineUtils::RemoveComment(lineBuff);
		EngineUtils::RemoveNewline(lineBuff);

		if (lineBuff[0] == 0x0)
			continue;

		if (!strncmp(lineBuff, "EndSceneInfo", 12))
			break;

		vector<char*> split = EngineUtils::SplitString(lineBuff, '=');

		if (split.size() != 2)
			continue;

		size_t len = strlen(split[0]);

		if (!strncmp(split[0], "name", len))
			_name = split[1];
		else if (!strncmp(split[0], "bgmusic", len))
		{
			_bgMusic = ResourceManager::GetResourceID(split[1], ResourceType::RES_AUDIOCLIP);
			if (_bgMusic == ENGINE_NOT_FOUND)
				Logger::Log(SCENE_MODULE, LOG_CRITICAL, "Failed to load background music for scene id %d. Audioclip \"%s\" not found.", _id, split[1]);
		}
		else if (!strncmp(split[0], "bgmusicvol", len))
			_bgMusicVolume = (float)atof(split[1]);
		else if (!strncmp(split[0], "ambintensity", len))
			_ambientColorIntensity = (float)atof(split[1]);
		else if (!strncmp(split[0], "ambcolor", len))
			EngineUtils::ReadFloatArray(split[1], 3, &_ambientColor.x);
		else if (!strncmp(split[0], "gamemodule", len))
		{
			if (!Engine::GetGameModule())
			{
				Logger::Log(SCENE_MODULE, LOG_CRITICAL, "Scene id=%d requires %s game module, but no game module is available", _id, split[1]);
				DIE("No game module loaded. Please check the log file for details.\nThe program cannot continue.");
			}

			if (strncmp(split[1], Engine::GetGameModule()->GetModuleName(), strlen(split[1])))
			{
				Logger::Log(SCENE_MODULE, LOG_CRITICAL, "Scene id=%d requires %s game module, but %s is loaded", _id, split[1], Engine::GetGameModule()->GetModuleName());
				DIE("Wrong game module loaded. Please check the log file for details.\nThe program cannot continue.");
			}
		}

		for (char* c : split)
			free(c);
	}

	DeferredBuffer::SetAmbientColor(_ambientColor, _ambientColorIntensity);
}

void Scene::_LoadComponent(VFSFile *f, ComponentInitInfo *initInfo)
{
	char lineBuff[SCENE_LINE_BUFF];
	memset(lineBuff, 0x0, SCENE_LINE_BUFF);

	while (!f->EoF())
	{
		memset(lineBuff, 0x0, SCENE_LINE_BUFF);
		f->Gets(lineBuff, SCENE_LINE_BUFF);

		if (lineBuff[0] == 0x0)
			continue;

		EngineUtils::RemoveComment(lineBuff);
		EngineUtils::RemoveNewline(lineBuff);

		if (lineBuff[0] == 0x0)
			continue;

		if (!strncmp(lineBuff, "EndComponent", 12))
			break;

		vector<char*> split = EngineUtils::SplitString(lineBuff, '=');

		if (split.size() != 2)
		{
			for (char* c : split)
				free(c);
			continue;
		}

		char *ptr = split[0];

		// skip tabs
		while (*ptr == '\t') ptr++;

		initInfo->initializer.arguments.insert(make_pair(ptr, split[1]));

		for (char* c : split)
			free(c);
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
	char lineBuff[SCENE_LINE_BUFF];
	memset(lineBuff, 0x0, SCENE_LINE_BUFF);

	NString path("/");
	path.Append(_sceneFile);

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
		memset(lineBuff, 0x0, SCENE_LINE_BUFF);
		f->Gets(lineBuff, SCENE_LINE_BUFF);

		if (lineBuff[0] == 0x0)
			continue;

		EngineUtils::RemoveComment(lineBuff);
		EngineUtils::RemoveNewline(lineBuff);

		if (lineBuff[0] == 0x0)
			continue;

		if (strstr(lineBuff, "Object"))
		{
			vector<char*> split = EngineUtils::SplitString(lineBuff, '=');
			string className = "Object";

			if (split.size() == 2)
				className = split[1];

			Object *obj = _LoadObject(f, className);

			if (!obj)
			{
				f->Close();
				return ENGINE_FAIL;
			}

			Light *l = dynamic_cast<Light *>(obj);
			Skybox *s = dynamic_cast<Skybox *>(obj);
			Terrain *t = dynamic_cast<Terrain *>(obj);

			if (l)
				_lights.push_back(l);
			else if (!_skybox && s)
				_skybox = s;
			else if (!_terrain && t)
				_terrain = t;
			else
				_objects.push_back(obj);

			for (char* c : split)
				free(c);
		}
		else if (strstr(lineBuff, "SceneInfo"))
			_LoadSceneInfo(f);
	}

	f->Close();

	if (!CameraManager::Count())
	{
		Unload();
		Logger::Log(SCENE_MODULE, LOG_CRITICAL, "Load failed for scene id=%d: no camera found", _id);
		return ENGINE_NO_CAMERA;
	}

	DeferredBuffer::SetFogColor(CameraManager::GetActiveCamera()->GetFogColor());
	DeferredBuffer::SetFogProperties(CameraManager::GetActiveCamera()->GetViewDistance(), CameraManager::GetActiveCamera()->GetFogDistance());

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

	Logger::Log(SCENE_MODULE, LOG_INFORMATION, "Scene %s, id=%d loaded with %d %s and %d %s", _name.c_str(), _id, _objects.size(), _objects.size() > 1 ? "objects" : "object", CameraManager::Count(), CameraManager::Count() > 1 ? "cameras" : "camera");

	return ENGINE_OK;
}

int Scene::CreateArrayBuffers() noexcept
{
	Engine::GetRenderer()->MakeCurrent(R_RENDER_CONTEXT);

	if (Engine::GetRenderer()->HasCapability(RendererCapability::DrawBaseVertex))
	{
		if ((_sceneVertexBuffer = Engine::GetRenderer()->CreateBuffer(BufferType::Vertex, false, false)) == nullptr)
			return ENGINE_OUT_OF_RESOURCES;
		if ((_sceneIndexBuffer = Engine::GetRenderer()->CreateBuffer(BufferType::Index, false, false)) == nullptr)
			return ENGINE_OUT_OF_RESOURCES;

		_sceneVertexBuffer->SetStorage(sizeof(Vertex) * _sceneVertices.size(), _sceneVertices.data());
		_sceneIndexBuffer->SetStorage(sizeof(uint32_t) * _sceneIndices.size(), _sceneIndices.data());

		BufferAttribute attrib;
		attrib.name = "POSITION";
		attrib.index = SHADER_POSITION_ATTRIBUTE;
		attrib.size = 3;
		attrib.sindex = 0;
		attrib.type = BufferDataType::Float;
		attrib.normalize = false;
		attrib.stride = sizeof(Vertex);
		attrib.ptr = (void *)VERTEX_POSITION_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		attrib.name = "COLOR";
		attrib.index = SHADER_COLOR_ATTRIBUTE;
		attrib.ptr = (void *)VERTEX_COLOR_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		attrib.name = "NORMAL";
		attrib.index = SHADER_NORMAL_ATTRIBUTE;
		attrib.ptr = (void *)VERTEX_NORMAL_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		attrib.name = "TANGENT";
		attrib.index = SHADER_TANGENT_ATTRIBUTE;
		attrib.ptr = (void *)VERTEX_TANGENT_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		attrib.name = "TEXCOORD";
		attrib.index = SHADER_UV_ATTRIBUTE;
		attrib.size = 2;
		attrib.ptr = (void *)VERTEX_UV_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		attrib.name = "TEXCOORD";
		attrib.sindex = 1;
		attrib.index = SHADER_TERRAINUV_ATTRIBUTE;
		attrib.size = 2;
		attrib.ptr = (void *)VERTEX_TUV_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		attrib.name = "BLENDINDICES";
		attrib.sindex = 0;
		attrib.index = SHADER_INDEX_ATTRIBUTE;
		attrib.size = 4;
		attrib.type = BufferDataType::Int;
		attrib.ptr = (void *)VERTEX_INDEX_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		attrib.name = "BLENDWEIGHT";
		attrib.index = SHADER_WEIGHT_ATTRIBUTE;
		attrib.size = 4;
		attrib.type = BufferDataType::Float;
		attrib.ptr = (void *)VERTEX_WEIGHT_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		attrib.name = "TEXCOORD";
		attrib.sindex = 2;
		attrib.index = SHADER_NUMBONES_ATTRIBUTE;
		attrib.size = 1;
		attrib.type = BufferDataType::Int;
		attrib.ptr = (void *)VERTEX_NUMBONES_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		if ((_sceneArrayBuffer = Engine::GetRenderer()->CreateArrayBuffer()) == nullptr)
			return ENGINE_OUT_OF_RESOURCES;
		_sceneArrayBuffer->SetVertexBuffer(_sceneVertexBuffer);
		_sceneArrayBuffer->SetIndexBuffer(_sceneIndexBuffer);
		_sceneArrayBuffer->CommitBuffers();

		_sceneVertices.clear();
		_sceneIndices.clear();

		if (_terrain)
			return _terrain->CreateArrayBuffer();
	}
	else
	{
		int ret = ENGINE_OK;

		for (Object *obj : _objects)
			ret = obj->CreateArrayBuffer();

		if(_terrain)
			ret = _terrain->CreateArrayBuffer();

		if (_skybox)
			ret = _skybox->CreateArrayBuffer();

		return ret;
	}

	return ENGINE_OK;
}

void Scene::Draw(RShader* shader, Camera *camera) noexcept
{
	if (_terrain)
		_terrain->Draw(shader, camera);

	if(_sceneArrayBuffer) _sceneArrayBuffer->Bind();

	for (Object *obj : _objects)
		if (distance(camera->GetPosition(), obj->GetPosition()) < camera->GetFogDistance() + 600)
			obj->Draw(shader, camera);

	if (_drawLights)
		for (Light *l : _lights)
			l->Draw(shader, camera);

	if(_sceneArrayBuffer) _sceneArrayBuffer->Unbind();
}

void Scene::DrawTerrain(Camera *camera) noexcept
{
	if (_terrain)
		_terrain->Draw(nullptr, camera);
}

void Scene::DrawSkybox(Camera *camera) noexcept
{
	if(_sceneArrayBuffer) _sceneArrayBuffer->Bind();

	if (_skybox)
		_skybox->Draw(nullptr, camera);

	if(_sceneArrayBuffer) _sceneArrayBuffer->Unbind();
}

void Scene::Update(double deltaTime) noexcept
{
	if(_terrain && (!Engine::IsPaused() || _terrain->GetUpdateWhilePaused()))
		_terrain->Update(deltaTime);
	
	if(_skybox && (!Engine::IsPaused() || _skybox->GetUpdateWhilePaused()))
		_skybox->Update(deltaTime);

	//#pragma omp parallel for
	for (size_t i = 0; i < _objects.size(); i++)
		if(!Engine::IsPaused() || _objects[i]->GetUpdateWhilePaused())
			_objects[i]->Update(deltaTime);

	//#pragma omp parallel for
	for (size_t i = 0; i < _lights.size(); i++)
		_lights[i]->Update(deltaTime);

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

void Scene::Unload() noexcept
{
	delete _skybox;
	_skybox = nullptr;

	delete _terrain;
	_terrain = nullptr;

	for (Object *obj : _objects)
		delete obj;
	_objects.clear();

	for (Light *light : _lights)
	{
		light->Unload();
		delete light;
	}
	_lights.clear();

	if (_bgMusic >= 0)
	{
		SoundManager::StopBackgroundMusic();
		SoundManager::SetBackgroundMusic(BG_MUSIC_NONE);
	}

	delete _sceneVertexBuffer;
	_sceneVertexBuffer = nullptr;
	
	delete _sceneIndexBuffer;
	_sceneIndexBuffer = nullptr;
	
	delete _sceneArrayBuffer;
	_sceneArrayBuffer = nullptr;
	
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
}
