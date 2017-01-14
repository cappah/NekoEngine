/* NekoEngine
 *
 * Scene.h
 * Author: Alexandru Naiman
 *
 * Scene class definition
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

#include <string>
#include <vector>
#include <fstream>

#include <stdint.h>

#include <Renderer/Buffer.h>
#include <Renderer/Renderer.h>
#include <Engine/Engine.h>
#include <Scene/Object.h>
#include <Scene/OcTree.h>
#include <Scene/LoadingScreen.h>
#include <System/VFS/VFS.h>
#include <System/Logger.h>

class Scene
{
public:
	ENGINE_API Scene(int id, std::string file) noexcept :
		_id(id),
		_bgMusic(-1),
		_loaded(false),
		_sceneFile(file),
		_name("Unnamed Scene"),
		_bgMusicVolume(1.f),
		_loadingScreenTexture(LS_DEFAULT_TEXTURE),
		_sceneBuffer(nullptr),
		_sceneUbo(nullptr),
		_threadPool(new NThreadPool(Platform::GetNumberOfProcessors() - 2)),
		_ocTree(new OcTree())
	{ };

	ENGINE_API Scene(int id, std::string file, const char* ls_texture) noexcept :
		_id(id),
		_bgMusic(-1),
		_loaded(false),
		_sceneFile(file),
		_name("Unnamed Scene"),
		_bgMusicVolume(1.f),
		_loadingScreenTexture(ls_texture),
		_sceneBuffer(nullptr),
		_sceneUbo(nullptr),
		_threadPool(new NThreadPool(Platform::GetNumberOfProcessors() - 2)),
		_ocTree(new OcTree())
	{ };

	ENGINE_API int GetId() noexcept { return _id; }
	ENGINE_API bool IsLoaded() noexcept { return _loaded; }
	ENGINE_API NString &GetName() noexcept { return _name; }
	ENGINE_API Object *GetObjectByID(uint32_t id);
	ENGINE_API Object *GetObjectByName(const char *name);
	ENGINE_API size_t GetObjectCount() noexcept { return _objects.size(); }
	ENGINE_API size_t GetVertexCount() noexcept;
	ENGINE_API size_t GetTriangleCount() noexcept;
	ENGINE_API NString GetLoadingScreenTextureID() noexcept { return _loadingScreenTexture; }
	ENGINE_API OcTree *GetOcTree() noexcept { return _ocTree; }
	
	ENGINE_API int Load();
	ENGINE_API void Update(double deltaTime) noexcept;
	ENGINE_API void Unload() noexcept;

	ENGINE_API void AddObject(Object *obj) noexcept;
	ENGINE_API void RemoveObject(Object *obj) noexcept;

	ENGINE_API ~Scene() noexcept;

	void PrepareCommandBuffers();
	bool RebuildCommandBuffers();

	template<typename T>
	void GetObjectsOfType(std::vector<T *> &vec)
	{
		for (Object *obj : _objects)
		{
			T *c = dynamic_cast<T *>(obj);
			if (c)
				vec.push_back(c);
		}
	}

#ifdef ENGINE_INTERNAL
	Buffer *GetSceneBuffer() noexcept { return _sceneBuffer; }
	void UpdateData(VkCommandBuffer buffer) noexcept;
	void DrawShadow(VkCommandBuffer commandBuffer, uint32_t shadowId) noexcept;
#endif

private:
	int _id, _bgMusic;
	bool _loaded;
	NString _sceneFile, _name;
	std::vector<Object *> _objects;
	std::vector<Object *> _newObjects, _deletedObjects;
	float _bgMusicVolume;
	NString _loadingScreenTexture;
	Buffer *_sceneBuffer, *_sceneUbo;
	NThreadPool *_threadPool;
	OcTree *_ocTree;

#ifdef ENGINE_INTERNAL
	uint64_t _bufferSize;
	std::vector<std::string> _loadedMeshIds;

	Object *_LoadObject(VFSFile *f, NString &className);
	void _LoadSceneInfo(VFSFile *f);
	void _LoadComponent(VFSFile *f, struct COMPONNENT_INITIALIZER_INFO *initInfo);
#endif
};

#if defined(_MSC_VER)
template ENGINE_API class NArray<Scene *>;
#endif
