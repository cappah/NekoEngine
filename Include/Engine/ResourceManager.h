/* NekoEngine
 *
 * ResourceManager.h
 * Author: Alexandru Naiman
 *
 * ResourceManager class definition 
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

#pragma once

#include <map>
#include <vector>
#include <fstream>

#include <Resource/Resource.h>
#include <Resource/MeshResource.h>
#include <Resource/TextureResource.h>
#include <Resource/ShaderResource.h>
#include <Resource/AudioClipResource.h>
#include <Resource/FontResource.h>
#include <Resource/MaterialResource.h>
#include <Resource/AnimationClipResource.h>

#include <Engine/StaticMesh.h>
#include <Engine/SkeletalMesh.h>
#include <Engine/Texture.h>
#include <Engine/Shader.h>
#include <Audio/AudioClip.h>
#include <Engine/NFont.h>
#include <Engine/Material.h>
#include <Engine/AnimationClip.h>

class ResourceManager
{
public:
	ENGINE_API static int Initialize();
	ENGINE_API static Resource* GetResource(int id, ResourceType type);
	ENGINE_API static Resource* GetResourceByName(const char* name, ResourceType type);
	ENGINE_API static int GetResourceID(const char* name, ResourceType type);
	ENGINE_API static int UnloadResource(int id, ResourceType type) noexcept;
	ENGINE_API static int UnloadResourceByName(const char* name, ResourceType type) noexcept;

	ENGINE_API static size_t LoadedStaticMeshes() noexcept { return _loadedResources[ResourceType::RES_STATIC_MESH]; }
	ENGINE_API static size_t LoadedSkeletalMeshes() noexcept { return _loadedResources[ResourceType::RES_SKELETAL_MESH]; }
	ENGINE_API static size_t LoadedTextures() noexcept { return _loadedResources[ResourceType::RES_TEXTURE]; }
	ENGINE_API static size_t LoadedShaders() noexcept { return _loadedResources[ResourceType::RES_SHADER]; }
	ENGINE_API static size_t LoadedMaterials() noexcept { return _loadedResources[ResourceType::RES_MATERIAL]; }
	ENGINE_API static size_t LoadedSounds() noexcept { return _loadedResources[ResourceType::RES_AUDIOCLIP]; }
	ENGINE_API static size_t LoadedFonts() noexcept { return _loadedResources[ResourceType::RES_FONT]; }
	
	ENGINE_API static void Release() noexcept;
	
private:	
	static std::vector<ResourceInfo*> _resourceInfo;
	static std::vector<Resource*> _resources;
	static std::map<ResourceType, size_t> _loadedResources;

	static class ResourceDatabase* _db;

	static Resource* _LoadResourceByID(int id, ResourceType type);
	static Resource* _LoadResourceByName(const char* name, ResourceType type);
	static Resource* _LoadResourceInternal(ResourceInfo *ri);
	static int _LoadResources();
	static void _UnloadResources() noexcept;
	
	ResourceManager() { }
};

