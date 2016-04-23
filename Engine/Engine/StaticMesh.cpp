/* Neko Engine
 *
 * StaticMesh.cpp
 * Author: Alexandru Naiman
 *
 * StaticMesh class implementation 
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

#include <Engine/StaticMesh.h>
#include <Engine/Vertex.h>
#include <Engine/Engine.h>
#include <Engine/EngineUtils.h>
#include <Engine/Shader.h>
#include <System/Logger.h>
#include <System/AssetLoader/AssetLoader.h>

#include <vector>
#include <string.h>

#define MESH_MODULE	"StaticMesh"

using namespace std;
using namespace glm;

StaticMesh::StaticMesh(MeshResource *res) noexcept :
	_vertexBuffer(nullptr),
	_indexBuffer(nullptr),
	_arrayBuffer(nullptr),
	_indexCount(0),
	_vertexCount(0),
	_triangleCount(0),
	_vertexOffset(0),
	_indexOffset(0),
	_groupOffset(0),
	_dynamic(false),
	_hasOwnBuffer(false)
{	
	_resourceInfo = res;
	_groupOffset.push_back(0);
}

int StaticMesh::Load()
{
	string path("/");
	path.append(GetResourceInfo()->filePath);

	if (AssetLoader::LoadMesh(path, _vertices, _indices, _groupOffset, _groupCount) != ENGINE_OK)
	{
		Logger::Log(MESH_MODULE, LOG_CRITICAL, "Failed to load mesh id=%s", _resourceInfo->name.c_str());
		return ENGINE_FAIL;
	}

	_indexCount = _indices.size();
	_vertexCount = _vertices.size();
	_triangleCount = _indexCount / 3;

	_CalculateTangents();

	Logger::Log(MESH_MODULE, LOG_DEBUG, "Loaded mesh id %d from %s, %d vertices, %d indices", _resourceInfo->id, path.c_str(), _vertexCount, _indexCount);

	return ENGINE_OK;
}

int StaticMesh::LoadDynamic(vector<Vertex> &vertices, vector<uint32_t> &indices)
{
	_vertices = vertices;
	_indices = indices;

	_CalculateTangents();

	_dynamic = true;

	return CreateBuffers(true);
}

int StaticMesh::CreateBuffers(bool dynamic)
{
	_arrayBuffer = Engine::GetRenderer()->CreateArrayBuffer();

	_indexBuffer = Engine::GetRenderer()->CreateBuffer(BufferType::Index, true, false);
	_indexBuffer->SetStorage((sizeof(uint32_t) * _indices.size()), _indices.data());

	_indexCount = _indices.size();

	_vertexBuffer = Engine::GetRenderer()->CreateBuffer(BufferType::Vertex, true, false);
	_vertexBuffer->SetStorage((sizeof(Vertex) * _vertices.size()), _vertices.data());

	_vertexCount = _vertices.size();

	_triangleCount = _indexCount / 3;

	BufferAttribute attrib;
	attrib.index = SHADER_POSITION_ATTRIBUTE;
	attrib.size = 3;
	attrib.type = BufferDataType::Float;
	attrib.normalize = false;
	attrib.stride = sizeof(Vertex);
	attrib.ptr = (void *)VERTEX_POSITION_OFFSET;
	_vertexBuffer->AddAttribute(attrib);

	attrib.index = SHADER_COLOR_ATTRIBUTE;
	attrib.ptr = (void *)VERTEX_COLOR_OFFSET;
	_vertexBuffer->AddAttribute(attrib);

	attrib.index = SHADER_NORMAL_ATTRIBUTE;
	attrib.ptr = (void *)VERTEX_NORMAL_OFFSET;
	_vertexBuffer->AddAttribute(attrib);

	attrib.index = SHADER_TANGENT_ATTRIBUTE;
	attrib.ptr = (void *)VERTEX_TANGENT_OFFSET;
	_vertexBuffer->AddAttribute(attrib);

	attrib.index = SHADER_UV_ATTRIBUTE;
	attrib.size = 2;
	attrib.ptr = (void *)VERTEX_UV_OFFSET;
	_vertexBuffer->AddAttribute(attrib);

	attrib.index = SHADER_TERRAINUV_ATTRIBUTE;
	attrib.size = 2;
	attrib.ptr = (void *)VERTEX_TUV_OFFSET;
	_vertexBuffer->AddAttribute(attrib);

	_arrayBuffer->SetVertexBuffer(_vertexBuffer);
	_arrayBuffer->SetIndexBuffer(_indexBuffer);
	_arrayBuffer->CommitBuffers();
	
	if(_groupCount.size() == 0)
		_groupCount.push_back((uint32_t)_indices.size());

	_indexBuffer->Unbind();
	_vertexBuffer->Unbind();

	_hasOwnBuffer = true;

	return ENGINE_OK;
}

void StaticMesh::UpdateIndices(vector<uint32_t> &indices)
{
	if (!_dynamic)
		return;

	_indexBuffer->UpdateData(0, (sizeof(uint32_t) * indices.size()), (void *)indices.data());
}

void StaticMesh::UpdateVertices(vector<Vertex> &vertices)
{
	if (!_dynamic)
		return;

	_vertexBuffer->UpdateData(0, (sizeof(Vertex) * vertices.size()), (void *)vertices.data());
}

void StaticMesh::Draw(Renderer* r, size_t group)
{
	if(_hasOwnBuffer)
		r->DrawElements(PolygonMode::Triangles, (int32_t)_groupCount[group], ElementType::UnsignedInt, (void *)(_groupOffset[group] * sizeof(unsigned int)));
	else
		r->DrawElementsBaseVertex(PolygonMode::Triangles, (int32_t)_groupCount[group], ElementType::UnsignedInt, (void *)((_indexOffset + _groupOffset[group]) * sizeof(unsigned int)), (uint32_t)_vertexOffset);
}

void StaticMesh::_CalculateTangents()
{
	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		Vertex &v0 = _vertices[_indices[i]];
		Vertex &v1 = _vertices[_indices[i + 1]];
		Vertex &v2 = _vertices[_indices[i + 2]];

		vec3 edge1 = v1.pos - v0.pos;
		vec3 edge2 = v2.pos - v0.pos;

		float deltaU1 = v1.uv.x - v0.uv.x;
		float deltaV1 = v1.uv.y - v0.uv.y;
		float deltaU2 = v2.uv.x - v0.uv.x;
		float deltaV2 = v2.uv.y - v0.uv.y;

		float f = 1.f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

		vec3 tgt, bitgt;

		tgt.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
		tgt.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
		tgt.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

		v0.tgt += tgt;
		v1.tgt += tgt;
		v2.tgt += tgt;
	}

	for(size_t i = 0; i < _vertices.size(); i++)
		_vertices[i].tgt = normalize(_vertices[i].tgt);
}

void StaticMesh::Release() noexcept
{
	delete _vertexBuffer;
	delete _indexBuffer;
	delete _arrayBuffer;
}

StaticMesh::~StaticMesh() noexcept
{
	Release();
}
