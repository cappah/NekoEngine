/* NekoEngine
 *
 * ResourceManager.cpp
 * Author: Alexandru Naiman
 *
 * ResourceManager implementation 
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

#include <Engine/Engine.h>
#include <Engine/ResourceManager.h>
#include <Engine/ResourceDatabase.h>
#include <Engine/EngineUtils.h>
#include <System/Logger.h>

#include <algorithm>

#define RM_MODULE	"ResourceManager"

static const char* _resourceTypes[] =
{
	"mesh",
	"skeletalmesh",
	"texture",
	"shader",
	"audioclip",
	"font",
	"material",
	"animationclip"
};

using namespace std;

std::vector<ResourceInfo*> ResourceManager::_resourceInfo;
std::vector<Resource*> ResourceManager::_resources;
std::map<ResourceType, size_t> ResourceManager::_loadedResources;

ResourceDatabase* ResourceManager::_db = nullptr;

int ResourceManager::Initialize()
{
	for (unsigned int i = 0; i < (unsigned int)ResourceType::RES_END; i++)
		_loadedResources.insert(make_pair((ResourceType)i, 0));
	
	Logger::Log(RM_MODULE, LOG_INFORMATION, "Initialized");

	return _LoadResources();
}

int ResourceManager::_LoadResources()
{
	string databaseFile = Engine::GetConfiguration().Engine.DataDirectory;
	databaseFile.append("/resources.db");

	_db = new ResourceDatabase();

	if (!_db->Open(databaseFile.c_str()))
	{
		Logger::Log(RM_MODULE, LOG_CRITICAL, "Failed to open resource database");
		delete _db;
		_db = nullptr;
		return ENGINE_FAIL;
	}

	return _db->GetResources(_resourceInfo) ? ENGINE_OK : ENGINE_FAIL;
}

Resource *ResourceManager::GetResource(int id, ResourceType type)
{
	Resource *res = nullptr;

	for (Resource *r : _resources)
	{
		if (r->GetResourceInfo()->type != type)
			continue;

		if (r->GetResourceInfo()->id != id)
			continue;

		res = r;
		break;
	}

	if (res == nullptr)
	{
		res = _LoadResourceByID(id, type);

		if (res == nullptr)
			return nullptr;
	}

	res->IncrementReferenceCount();

	return res;
}

Resource* ResourceManager::GetResourceByName(const char* name, ResourceType type)
{
	Resource *res = nullptr;

	if (name == nullptr)
		return nullptr;

	size_t len = strlen(name);
	if (len <= 0)
		return nullptr;

	for (Resource *r : _resources)
	{
		if (r->GetResourceInfo()->type != type)
			continue;

		if (strncmp(r->GetResourceInfo()->name.c_str(), name, len))
			continue;

		res = r;
		break;
	}

	if (res == nullptr)
	{
		res = _LoadResourceByName(name, type);

		if (res == nullptr)
			return nullptr;
	}

	res->IncrementReferenceCount();

	return res;
}

Resource* ResourceManager::_LoadResourceByID(int id, ResourceType type)
{
	if (id < 0)
		return nullptr;

	for (ResourceInfo *ri : _resourceInfo)
	{
		if (ri->type != type)
			continue;

		if (ri->id != id)
			continue;

		return _LoadResourceInternal(ri);
	}

	return nullptr;
}

Resource* ResourceManager::_LoadResourceByName(const char* name, ResourceType type)
{
	if (name == nullptr)
		return nullptr;

	size_t len = strlen(name);
	if (len <= 0)
		return nullptr;

	for (ResourceInfo *ri : _resourceInfo)
	{
		if (ri->type != type)
			continue;

		if (strncmp(ri->name.c_str(), name, len))
			continue;

		return _LoadResourceInternal(ri);
	}

	return nullptr;
}

Resource* ResourceManager::_LoadResourceInternal(ResourceInfo *ri)
{
	Resource *res = nullptr;

	switch (ri->type)
	{
		case ResourceType::RES_STATIC_MESH:
			res = new StaticMesh((MeshResource *)ri);
		break;
		case ResourceType::RES_SKELETAL_MESH:
			res = new SkeletalMesh((MeshResource *)ri);
		break;
		case ResourceType::RES_TEXTURE:
			res = new Texture((TextureResource *)ri);
		break;
		case ResourceType::RES_SHADER:
			res = new Shader((ShaderResource *)ri);
		break;
		case ResourceType::RES_AUDIOCLIP:
			res = new AudioClip((AudioClipResource *)ri);
		break;
		case ResourceType::RES_FONT:
			res = new NFont((FontResource *)ri);
		break;
		case ResourceType::RES_MATERIAL:
			res = new Material((MaterialResource *)ri);
		break;
		case ResourceType::RES_ANIMCLIP:
			res = new AnimationClip((AnimationClipResource *)ri);
		break;
		default:
		{
			Logger::Log(RM_MODULE, LOG_WARNING, "Invalid resource type requested = %d", (int)ri->type);
			return nullptr;
		}
	}

	assert(res);
	
	int ret = res->Load();

	if (ret != ENGINE_OK)
	{
		Logger::Log(RM_MODULE, LOG_CRITICAL, "Failed to load %s resource id=%d, name=\"%s\", error code %d", _resourceTypes[(int)ri->type], ri->id, ri->name.c_str(), ret);
		delete res;
		return nullptr;
	}

	_loadedResources[ri->type]++;
	_resources.push_back(res);
	return res;
}

int ResourceManager::UnloadResource(int id, ResourceType type) noexcept
{
	Resource *deleteResource = nullptr;

	for (Resource *r : _resources)
	{
		if (r->GetResourceInfo() == nullptr)
			continue;

		if (r->GetResourceInfo()->type != type)
			continue;

		if (r->GetResourceInfo()->id != id)
			continue;

		r->DecrementReferenceCount();

		if (r->GetReferenceCount() <= 0)
		{
			deleteResource = r;
			break;
		}

		break;
	}

	if (deleteResource)
	{
		_resources.erase(remove(_resources.begin(), _resources.end(), deleteResource), _resources.end());
		delete deleteResource;
		_loadedResources[type]--;
	}

	return ENGINE_OK;
}

int ResourceManager::UnloadResourceByName(const char *name, ResourceType type) noexcept
{
	int id = GetResourceID(name, type);

	return UnloadResource(id, type);
}

int ResourceManager::GetResourceID(const char* name, ResourceType type)
{
	if (name == nullptr)
		return ENGINE_INVALID_ARGS;

	size_t len = strlen(name);
	if (len <= 0)
		return ENGINE_INVALID_ARGS;

	for (ResourceInfo *ri : _resourceInfo)
	{
		if (ri->type != type)
			continue;

		if (strncmp(ri->name.c_str(), name, len))
			continue;

		return ri->id;
	}

	return ENGINE_NOT_FOUND;
}

void ResourceManager::_UnloadResources() noexcept
{
	if(_resources.size() > 0)
	{
		for (Resource *r : _resources)
			delete r;
		_resources.clear();
	}

	if (_resourceInfo.size() > 0)
	{
		for (ResourceInfo *ri : _resourceInfo)
			delete ri;
		_resourceInfo.clear();
	}
}

void ResourceManager::Release() noexcept
{
	_UnloadResources();

	if(_db)
		delete _db;
	
	Logger::Log(RM_MODULE, LOG_INFORMATION, "Released");
}
