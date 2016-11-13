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

#include <Engine/Engine.h>
#include <Engine/Vertex.h>
#include <Renderer/Buffer.h>
#include <Renderer/Material.h>
#include <Resource/Resource.h>
#include <Resource/MeshResource.h>

class StaticMesh : public Resource
{
	friend class TerrainComponent;

public:
	ENGINE_API StaticMesh(MeshResource *res) noexcept;

	ENGINE_API MeshResource* GetResourceInfo() noexcept { return (MeshResource*)_resourceInfo; }
	ENGINE_API bool IsResident() noexcept { return _resident; }
	ENGINE_API uint32_t GetTotalIndexCount() noexcept { return _indexCount; }
	ENGINE_API uint32_t GetVertexCount() noexcept { return _vertexCount; }
	ENGINE_API uint32_t GetTriangleCount() noexcept { return _triangleCount; }
	ENGINE_API uint32_t GetGroupCount() noexcept { return (uint32_t)_groupOffset.size(); }
	ENGINE_API std::vector<Vertex> &GetVertices() { return _vertices; }
	ENGINE_API std::vector<uint32_t> &GetIndices() { return _indices; }

	ENGINE_API virtual int Load() override;
	ENGINE_API int LoadStatic(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices, bool createGroup = true, bool calculateTangents = true);
	ENGINE_API int LoadDynamic(std::vector<Vertex> & vertices, std::vector<uint32_t> &indices, bool createGroup = true, bool calculateTangents = true);
	ENGINE_API int CreateBuffer(bool dynamic);

	ENGINE_API void AddGroup(uint32_t offset, uint32_t count) { _groupOffset.push_back(offset); _groupCount.push_back(count); }
	ENGINE_API void ResetGroups() { _groupOffset.clear(); _groupCount.clear(); }

	ENGINE_API void UpdateIndices(std::vector<uint32_t>& indices);
	ENGINE_API void UpdateVertices(std::vector<Vertex> &vertices);

	ENGINE_API void Release() noexcept;

	ENGINE_API virtual ~StaticMesh() noexcept;

	Buffer *GetBuffer() { return _buffer; }
	uint32_t GetGroupOffset(uint32_t group) { return _groupOffset[group]; }
	uint32_t GetIndexCount(uint32_t group) noexcept { return _groupCount[group]; }
	VkDeviceSize GetRequiredMemorySize();

	bool Upload(Buffer *buffer = nullptr);
	VkDescriptorSet CreateDescriptorSet(VkDescriptorPool pool, Buffer *uniform);
	bool BuildCommandBuffers(NArray<Material *> &_materials, VkDescriptorSet descriptorSet, VkCommandBuffer &depthCmdBuffer, VkCommandBuffer &sceneCmdBuffer);

protected:
	Buffer *_buffer;
	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;
	std::vector<uint32_t> _groupOffset;
	std::vector<uint32_t> _groupCount;
	uint32_t _indexCount;
	uint32_t _vertexCount;
	uint32_t _triangleCount;
	bool _dynamic, _hasOwnBuffer, _resident;

	VkDeviceSize _vertexOffset;
	VkDeviceSize _indexOffset;

	PipelineId _depthPipelineId;
	PipelineLayoutId _depthPipelineLayoutId;

	void _CalculateTangents();
};