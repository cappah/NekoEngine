/* NekoEngine
 *
 * Scene.h
 * Author: Alexandru Naiman
 *
 * Scene class definition
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

#include <string>
#include <vector>
#include <fstream>

#include <stdint.h>
#include <glm/glm.hpp>

#include <Engine/Engine.h>
#include <Scene/Object.h>
#include <Scene/Light.h>
#include <Engine/Shader.h>
#include <Engine/LoadingScreen.h>
#include <System/VFS/VFS.h>
#include <Scene/Components/CameraComponent.h>

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
		_ambientColor(0.f, 0.f, 0.f),
		_ambientColorIntensity(.2f),
		_drawLights(false),
		_loadingScreenTexture(LS_DEFAULT_TEXTURE),
		_skybox(nullptr),
		_terrain(nullptr),
		_sceneVertexBuffer(nullptr),
		_sceneIndexBuffer(nullptr),
		_sceneArrayBuffer(nullptr)
	{ };

	ENGINE_API Scene(int id, std::string file, const char* ls_texture) noexcept :
		_id(id),
		_bgMusic(-1),
		_loaded(false),
		_sceneFile(file),
		_name("Unnamed Scene"),
		_bgMusicVolume(1.f),
		_ambientColor(0.f, 0.f, 0.f),
		_ambientColorIntensity(.2f),
		_drawLights(false),
		_loadingScreenTexture(ls_texture),
		_skybox(nullptr),
		_terrain(nullptr),
		_sceneVertexBuffer(nullptr),
		_sceneIndexBuffer(nullptr),
		_sceneArrayBuffer(nullptr)
	{ };

	ENGINE_API int GetId() noexcept { return _id; }
	ENGINE_API bool IsLoaded() noexcept { return _loaded; }
	ENGINE_API std::string& GetName() noexcept { return _name; }
	ENGINE_API size_t GetObjectCount() noexcept { return _objects.size(); }
	ENGINE_API size_t GetVertexCount() noexcept;
	ENGINE_API size_t GetTriangleCount() noexcept;
	ENGINE_API float GetAmbientIntensity() noexcept { return _ambientColorIntensity; }
	ENGINE_API glm::vec3& GetAmbientColor() noexcept { return _ambientColor; }
	ENGINE_API Light* GetLight(size_t i) { return _lights[i]; }
	ENGINE_API bool GetDrawLights() noexcept { return _drawLights; }
	ENGINE_API size_t GetNumLights() noexcept { return _lights.size(); }
	ENGINE_API const char* GetLoadingScreenTexture() noexcept { return _loadingScreenTexture.c_str(); }

	ENGINE_API void SetDrawLights(bool draw) noexcept { _drawLights = draw; }
	
	ENGINE_API int Load();
	ENGINE_API int CreateArrayBuffers() noexcept;
	ENGINE_API void Draw(RShader* shader, CameraComponent *camera) noexcept;
	ENGINE_API void DrawTerrain(CameraComponent *camera) noexcept;
	ENGINE_API void DrawSkybox(CameraComponent *camera) noexcept;
	ENGINE_API void Update(double deltaTime) noexcept;
	ENGINE_API void Unload() noexcept;

	ENGINE_API void AddObject(Object *obj) noexcept;
	ENGINE_API void RemoveObject(Object *obj) noexcept;

	ENGINE_API ~Scene() noexcept;

private:
	int _id, _bgMusic;
	bool _loaded;
	std::string _sceneFile, _name;
	std::vector<Object *> _objects;
	std::vector<Object *> _newObjects, _deletedObjects;
	std::vector<Light *> _lights;
	float _bgMusicVolume;
	glm::vec3 _ambientColor;
	float _ambientColorIntensity;
	bool _drawLights;
	std::string _loadingScreenTexture;
	class Skybox *_skybox;
	class Terrain *_terrain;

	std::vector<Vertex> _sceneVertices;
	std::vector<uint32_t> _sceneIndices;
	RBuffer *_sceneVertexBuffer, *_sceneIndexBuffer;
	RArrayBuffer *_sceneArrayBuffer;

	Object *_LoadObject(VFSFile *f, const std::string &className);
	void _LoadSceneInfo(VFSFile *f);
	void _LoadComponent(VFSFile *f, struct COMPONNENT_INITIALIZER_INFO *initInfo);

	void _AddVertices(std::vector<Vertex>& vertices) { _sceneVertices.insert(_sceneVertices.end(), vertices.begin(), vertices.end()); }
	void _AddIndices(std::vector<uint32_t>& indices) { _sceneIndices.insert(_sceneIndices.end(), indices.begin(), indices.end()); }
	uint64_t _GetVertexOffset() { return _sceneVertices.size(); }
	uint64_t _GetIndexOffset() { return _sceneIndices.size(); }
};

#if defined(_MSC_VER)
template ENGINE_API class NArray<Scene>;
#endif