/* NekoEngine
 *
 * StaticMesh.h
 * Author: Alexandru Naiman
 *
 * StaticMesh class definition 
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

#include <vector>

#include <glm/glm.hpp>

#include <Engine/Vertex.h>
#include <Resource/Resource.h>
#include <Resource/MeshResource.h>

class StaticMesh : public Resource
{
public:
	ENGINE_API StaticMesh(MeshResource *res) noexcept;

	ENGINE_API MeshResource* GetResourceInfo() noexcept { return (MeshResource*)_resourceInfo; }
	ENGINE_API size_t GetIndexCount(size_t group) noexcept { return _groupCount[group]; }

	ENGINE_API uint64_t GetVboOffset() { return _vertexOffset; }
	ENGINE_API uint64_t GetIboOffset(size_t group) { return _indexOffset + _groupOffset[group]; }

	ENGINE_API size_t GetTotalIndexCount() noexcept { return _indexCount; }
	ENGINE_API size_t GetVertexCount() noexcept { return _vertexCount; }
	ENGINE_API size_t GetTriangleCount() noexcept { return _triangleCount; }
	ENGINE_API size_t GetGroupCount() noexcept { return _groupOffset.size(); }
	ENGINE_API std::vector<Vertex>& GetVertices() { return _vertices; }
	ENGINE_API std::vector<uint32_t>& GetIndices() { return _indices; }

	ENGINE_API void SetVertexOffset(uint64_t offset) { _vertexOffset = offset; }
	ENGINE_API void SetIndexOffset(uint64_t offset) { _indexOffset = offset; }
	
	ENGINE_API void Bind() { if(_hasOwnBuffer) _arrayBuffer->Bind(); }
	ENGINE_API void Unbind() { if(_hasOwnBuffer) _arrayBuffer->Unbind(); }

	ENGINE_API virtual int Load() override;
	ENGINE_API int LoadDynamic(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	ENGINE_API int CreateBuffers(bool dynamic);

	ENGINE_API void UpdateIndices(std::vector<uint32_t>& indices);
	ENGINE_API void UpdateVertices(std::vector<Vertex> &vertices);
	
	ENGINE_API virtual void Draw(Renderer* r, size_t group);

	ENGINE_API void Release() noexcept;

	ENGINE_API virtual ~StaticMesh() noexcept;

protected:
	RBuffer* _vertexBuffer;
	RBuffer* _indexBuffer;
	RArrayBuffer* _arrayBuffer;
	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;
	std::vector<uint32_t> _groupOffset;
	std::vector<uint32_t> _groupCount;
	size_t _indexCount;
	size_t _vertexCount;
	size_t _triangleCount;
	bool _dynamic, _hasOwnBuffer;

	uint64_t _vertexOffset;
	uint64_t _indexOffset;

	void _CalculateTangents();
};

#if defined(_MSC_VER)
template class ENGINE_API NArray<StaticMesh>;
#endif