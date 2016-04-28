/* Neko Engine
 *
 * Mesh.cpp
 * Author: Alexandru Naiman
 *
 * Mesh class implementation 
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

#include <glm/glm.hpp>

#include <Engine/SkeletalMesh.h>
#include <Engine/Vertex.h>
#include <Engine/Engine.h>
#include <Engine/EngineUtils.h>
#include <System/Logger.h>
#include <System/AssetLoader/AssetLoader.h>

#define SK_MESH_MODULE	"SkeletalMesh"

using namespace std;
using namespace glm;

SkeletalMesh::SkeletalMesh(MeshResource *res) noexcept :
	StaticMesh(res)
{
	if(res->meshType != MeshType::Skeletal)
	{ DIE("Attempt to load static mesh as skeletal !"); }
}

int SkeletalMesh::Load()
{
	string path("/");
	path.append(GetResourceInfo()->filePath);
	
	if (AssetLoader::LoadMesh(path, MeshType::Skeletal, _vertices, _indices, _groupOffset, _groupCount, &_bones) != ENGINE_OK)
	{
		Logger::Log(SK_MESH_MODULE, LOG_CRITICAL, "Failed to load mesh id=%s", _resourceInfo->name.c_str());
		return ENGINE_FAIL;
	}
	
	_indexCount = _indices.size();
	_vertexCount = _vertices.size();
	_triangleCount = _indexCount / 3;
	
	_CalculateTangents();
	
	Logger::Log(SK_MESH_MODULE, LOG_DEBUG, "Loaded mesh id %d from %s, %d vertices, %d indices", _resourceInfo->id, path.c_str(), _vertexCount, _indexCount);
	
	return ENGINE_OK;
}

void SkeletalMesh::Update(float deltaTime)
{
	
}

void SkeletalMesh::Draw(Renderer* r, size_t group)
{
	StaticMesh::Draw(r, group);
}

SkeletalMesh::~SkeletalMesh() noexcept
{
}

void SkeletalMesh::_GetNodeHierarchy(float time, void *node, glm::mat4 &parentTransform)
{
	//
}
