/* Neko Engine
 *
 * Scene.cpp
 * Author: Alexandru Naiman
 *
 * Scene class implementation 
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
#include <iterator>
#include <string.h>

#include <glm/glm.hpp>

#include <Engine/Engine.h>
#include <Engine/EngineUtils.h>
#include <Engine/ResourceManager.h>
#include <Engine/SoundManager.h>
#include <Engine/GameModule.h>
#include <Engine/DeferredBuffer.h>
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

Object *Scene::_LoadObject(VFSFile *f, const string &className)
{
	char lineBuff[SCENE_LINE_BUFF];
	memset(lineBuff, 0x0, SCENE_LINE_BUFF);

	Object *obj = nullptr;
	vec3 vec;

	obj = Engine::NewObject(className);

	if (!obj)
	{
		Logger::Log(SCENE_MODULE, LOG_CRITICAL, "NewObject() call failed for class %s", className.c_str());
		return nullptr;
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

		if (strstr(lineBuff, "EndObject"))
			break;

		vector<char*> split = EngineUtils::SplitString(lineBuff, '=');

		if (split.size() < 2)
			continue;

		size_t len = strlen(split[0]);

		if (!strncmp(split[0], "id", len))
			obj->SetId(atoi(split[1]));
		else if (!strncmp(split[0], "color", len))
		{
			EngineUtils::ReadFloatArray(split[1], 3, &vec.x);
			obj->SetColor(vec);
		}
		else if (!strncmp(split[0], "position", len))
		{
			EngineUtils::ReadFloatArray(split[1], 3, &vec.x);
			obj->SetPosition(vec);
		}
		else if (!strncmp(split[0], "rotation", len))
		{
			EngineUtils::ReadFloatArray(split[1], 3, &vec.x);
			obj->SetRotation(vec);
		}
		else if (!strncmp(split[0], "scale", len))
		{
			EngineUtils::ReadFloatArray(split[1], 3, &vec.x);
			obj->SetScale(vec);
		}
		else if (!strncmp(split[0], "opt", len))
		{
			Light *l = dynamic_cast<Light *>(obj);

			vector<char*> optSplit = EngineUtils::SplitString(split[1], ';');

			for (char* opt : optSplit)
			{
				if (strstr(opt, "fwd"))
				{
					if (strstr(opt, "posz") != nullptr)
						obj->SetForwardDirection(ForwardDirection::PositiveZ);
					if (strstr(opt, "negz") != nullptr)
						obj->SetForwardDirection(ForwardDirection::NegativeZ);
					if (strstr(opt, "posx") != nullptr)
						obj->SetForwardDirection(ForwardDirection::PositiveX);
					if (strstr(opt, "negx") != nullptr)
						obj->SetForwardDirection(ForwardDirection::NegativeX);
				}
				else if (l && strstr(opt, "lt_point"))
					l->SetType(LightType::Point);
				else if (l && strstr(opt, "lt_directional"))
					l->SetType(LightType::Directional);
				else if (l && strstr(opt, "lt_attenuation"))
				{
					const char *pch = strchr(opt, ':');
					vec2 vec;

					EngineUtils::ReadFloatArray(pch + 1, 2, &vec.x);
					l->SetAttenuation(vec);
				}
				else if(l && strstr(opt, "lt_intensity"))
				{
					const char *pch = strchr(opt, ':');
					l->SetIntensity((float)atof(pch + 1));
				}
				else if (l && strstr(opt, "lt_direction"))
				{
					const char *pch = strchr(opt, ':');
					vec3 vec;

					EngineUtils::ReadFloatArray(pch + 1, 3, &vec.x);
					l->SetDirection(vec);
				}
			}

			l = nullptr;

			if(Engine::GetGameModule())
				Engine::GetGameModule()->LoadObjectOptionalArguments(obj, optSplit);
		
			for (char* c : optSplit)
				free(c);
		}
		else if (!strncmp(split[0], "Component", len))
			obj->AddComponent(split[2], _LoadComponent(f, obj, split[1]));

		Terrain *t = nullptr;

		if((t = dynamic_cast<Terrain *>(obj)))
		{
			if (!strncmp(split[0], "numcells", len))
				t->SetNumCells((unsigned short)atoi(split[1]));
			else if (!strncmp(split[0], "cellsize", len))
				t->SetCellSize((float)atof(split[1]));
		}

		for (char* c : split)
			free(c);
	}

	if (obj->Load() != ENGINE_OK)
	{
		Logger::Log(SCENE_MODULE, LOG_CRITICAL, "Failed to load object id %d", obj->GetId());
		delete obj;
		return nullptr;
	}

	return obj;
}

Camera *Scene::_LoadCamera(VFSFile *f)
{
	char lineBuff[SCENE_LINE_BUFF];
	memset(lineBuff, 0x0, SCENE_LINE_BUFF);

	Camera *cam = new Camera();
	vec3 pos = vec3(), rot = vec3(), vec;

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

		if (strstr(lineBuff, "EndCamera"))
			break;

		if (strstr(lineBuff, "default"))
			_activeCamera = cam;
		else if (strstr(lineBuff, "fps"))
			cam->SetFPSCamera(true);

		vector<char*> split = EngineUtils::SplitString(lineBuff, '=');

		if (split.size() != 2)
		{
			for (char* c : split)
				free(c);
			continue;
		}
		
		size_t len = strlen(split[0]);

		if (!strncmp(split[0], "id", len))
			cam->SetId(atoi(split[1]));
		else if (!strncmp(split[0], "fov", len))
			cam->SetFOV((float)atof(split[1]));
		else if (!strncmp(split[0], "near", len))
			cam->SetNear((float)atof(split[1]));
		else if (!strncmp(split[0], "far", len))
			cam->SetFar((float)atof(split[1]));
		else if (!strncmp(split[0], "position", len))
			EngineUtils::ReadFloatArray(split[1], 3, &pos.x);
		else if (!strncmp(split[0], "rotation", len))
			EngineUtils::ReadFloatArray(split[1], 3, &rot.x);
		else if (!strncmp(split[0], "fog_color", len))
		{
			EngineUtils::ReadFloatArray(split[1], 3, &vec.x);
			cam->SetFogColor(vec);
		}
		else if (!strncmp(split[0], "view_distance", len))
			cam->SetViewDistance((float)atof(split[1]));
		else if (!strncmp(split[0], "fog_distance", len))
			cam->SetFogDistance((float)atof(split[1]));
		else if (!strncmp(split[0], "projection", len))
		{
			size_t len = strlen(split[1]);
			if (!strncmp(split[1], "perspective", len))
				cam->SetProjection(ProjectionType::Perspective);
			else if (!strncmp(split[1], "ortographics", len))
				cam->SetProjection(ProjectionType::Ortographic);
		}

		for (char* c : split)
			free(c);
	}

	cam->Initialize();
	cam->SetPosition(pos);
	cam->SetRotation(rot);

	return cam;
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

ObjectComponent *Scene::_LoadComponent(VFSFile *f, Object *parent, const std::string &className)
{
	ComponentInitializer initializer;
	initializer.parent = parent;

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

		initializer.arguments.insert(make_pair(ptr, split[1]));

		for (char* c : split)
			free(c);
	}

	ObjectComponent *comp = Engine::NewComponent(className, &initializer);

	if (comp->Load() != ENGINE_OK)
		return nullptr;

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
		else
			skcomp->GetMesh()->CreateBuffers(false);
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
			else
				stcomp->GetMesh()->CreateBuffers(false);
		}
	}

	return comp;
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

	string path("/");
	path.append(_sceneFile);

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
		else if (strstr(lineBuff, "Camera"))
		{
			Camera *cam = _LoadCamera(f);

			if (!cam)
			{
				Logger::Log(SCENE_MODULE, LOG_CRITICAL, "Scene load failed for scene id=%d, camera load failed", _id);
				f->Close();
				return ENGINE_FAIL;
			}

			_cameras.push_back(cam);
		}
		else if (strstr(lineBuff, "SceneInfo"))
			_LoadSceneInfo(f);
	}

	f->Close();

	if (!_activeCamera)
	{
		if(_cameras.size() > 0)
			_activeCamera = _cameras[0];
		else
		{
			Unload();
			Logger::Log(SCENE_MODULE, LOG_CRITICAL, "Load failed for scene id=%d: no camera found", _id);
			return ENGINE_NO_CAMERA;
		}
	}

	DeferredBuffer::SetFogColor(_activeCamera->GetFogColor());
	DeferredBuffer::SetFogProperties(_activeCamera->GetViewDistance(), _activeCamera->GetFogDistance());

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

	if(Engine::GetRenderer()->HasCapability(RendererCapability::DrawBaseVertex))
	{
		if((_sceneVertexBuffer = Engine::GetRenderer()->CreateBuffer(BufferType::Vertex, false, false)) == nullptr)
			return ENGINE_OUT_OF_RESOURCES;
		if((_sceneIndexBuffer = Engine::GetRenderer()->CreateBuffer(BufferType::Index, false, false)) == nullptr)
			return ENGINE_OUT_OF_RESOURCES;

		_sceneVertexBuffer->SetStorage(sizeof(Vertex) * _sceneVertices.size(), _sceneVertices.data());
		_sceneIndexBuffer->SetStorage(sizeof(uint32_t) * _sceneIndices.size(), _sceneIndices.data());

		BufferAttribute attrib;
		attrib.index = SHADER_POSITION_ATTRIBUTE;
		attrib.size = 3;
		attrib.type = BufferDataType::Float;
		attrib.normalize = false;
		attrib.stride = sizeof(Vertex);
		attrib.ptr = (void *)VERTEX_POSITION_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		attrib.index = SHADER_COLOR_ATTRIBUTE;
		attrib.ptr = (void *)VERTEX_COLOR_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		attrib.index = SHADER_NORMAL_ATTRIBUTE;
		attrib.ptr = (void *)VERTEX_NORMAL_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		attrib.index = SHADER_TANGENT_ATTRIBUTE;
		attrib.ptr = (void *)VERTEX_TANGENT_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		attrib.index = SHADER_UV_ATTRIBUTE;
		attrib.size = 2;
		attrib.ptr = (void *)VERTEX_UV_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		attrib.index = SHADER_TERRAINUV_ATTRIBUTE;
		attrib.size = 2;
		attrib.ptr = (void *)VERTEX_TUV_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		attrib.index = SHADER_INDEX_ATTRIBUTE;
		attrib.size = 4;
		attrib.type = BufferDataType::Int;
		attrib.ptr = (void *)VERTEX_INDEX_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		attrib.index = SHADER_WEIGHT_ATTRIBUTE;
		attrib.size = 4;
		attrib.type = BufferDataType::Float;
		attrib.ptr = (void *)VERTEX_WEIGHT_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		attrib.index = SHADER_NUMBONES_ATTRIBUTE;
		attrib.size = 1;
		attrib.type = BufferDataType::Int;
		attrib.ptr = (void *)VERTEX_NUMBONES_OFFSET;
		_sceneVertexBuffer->AddAttribute(attrib);

		if((_sceneArrayBuffer = Engine::GetRenderer()->CreateArrayBuffer()) == nullptr)
			return ENGINE_OUT_OF_RESOURCES;
		_sceneArrayBuffer->SetVertexBuffer(_sceneVertexBuffer);
		_sceneArrayBuffer->SetIndexBuffer(_sceneIndexBuffer);
		_sceneArrayBuffer->CommitBuffers();

		_sceneVertices.clear();
		_sceneIndices.clear();
	}

	Logger::Log(SCENE_MODULE, LOG_INFORMATION, "Scene %s, id=%d loaded with %d %s and %d %s", _name.c_str(), _id, _objects.size(), _objects.size() > 1 ? "objects" : "object", _cameras.size(), _cameras.size() > 1 ? "cameras" : "camera");

	return ENGINE_OK;
}

void Scene::Draw(RShader* shader) noexcept
{
	if (_terrain)
		_terrain->Draw(shader);

	if(_sceneArrayBuffer) _sceneArrayBuffer->Bind();

	for (Object *obj : _objects)
		if (distance(_activeCamera->GetPosition(), obj->GetPosition()) < _activeCamera->GetFogDistance() + 600)
			obj->Draw(shader);

	if (_drawLights)
		for (Light *l : _lights)
			l->Draw(shader);

	if(_sceneArrayBuffer) _sceneArrayBuffer->Unbind();
}

void Scene::DrawTerrain() noexcept
{
	if (_terrain)
		_terrain->Draw(nullptr);
}

void Scene::DrawSkybox() noexcept
{
	if(_sceneArrayBuffer) _sceneArrayBuffer->Bind();

	if (_skybox)
		_skybox->Draw(nullptr);

	if(_sceneArrayBuffer) _sceneArrayBuffer->Unbind();
}

void Scene::Update(float deltaTime) noexcept
{
	if(_terrain)
		_terrain->Update(deltaTime);
	
	if(_skybox)
		_skybox->Update(deltaTime);

	//#pragma omp parallel for
	for (int i = 0; i < _objects.size(); i++)
		_objects[i]->Update(deltaTime);

	//#pragma omp parallel for
	for (int i = 0; i < _cameras.size(); i++)
		_cameras[i]->Update(deltaTime);

	//#pragma omp parallel for
	for (int i = 0; i < _lights.size(); i++)
		_lights[i]->Update(deltaTime);
}

void Scene::Unload() noexcept
{
	delete _skybox;
	_skybox = nullptr;

	delete _terrain;
	_terrain = nullptr;

	for (Camera *cam : _cameras)
		delete cam;
	_cameras.clear();
	_activeCamera = nullptr;

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

void Scene::RenderForCamera(Camera *cam) noexcept
{
	Camera *sceneCam = _activeCamera;
	_activeCamera = cam;

	Draw(nullptr);
	DrawSkybox();

	_activeCamera = sceneCam;
}

Scene::~Scene() noexcept
{
	Unload();
}
