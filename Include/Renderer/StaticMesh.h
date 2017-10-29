/* NekoEngine
 *
 * StaticMesh.h
 * Author: Alexandru Naiman
 *
 * StaticMesh class definition 
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

#include <vector>

#include <Engine/Engine.h>
#include <Engine/Vertex.h>
#include <Renderer/Buffer.h>
#include <Renderer/Material.h>
#include <Renderer/Drawable.h>
#include <Renderer/Primitives.h>
#include <Resource/Resource.h>
#include <Resource/MeshResource.h>

struct MeshGroup
{
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
};

class StaticMesh : public Resource
{
	friend class TerrainComponent;

public:
	ENGINE_API StaticMesh(MeshResource *res) noexcept;
	ENGINE_API StaticMesh(PrimitiveID primitiveId) noexcept;

	ENGINE_API MeshResource* GetResourceInfo() const noexcept { return (MeshResource*)_resourceInfo; }
	ENGINE_API bool IsResident() const noexcept { return _resident; }
	ENGINE_API uint32_t GetIndexCount() const noexcept { return _indexCount; }
	ENGINE_API uint32_t GetVertexCount() const noexcept { return _vertexCount; }
	ENGINE_API uint32_t GetTriangleCount() const noexcept { return _triangleCount; }
	ENGINE_API uint32_t GetGroupCount() const noexcept { return (uint32_t)_groups.size(); }
	ENGINE_API const std::vector<Vertex> &GetVertices() const noexcept { return _vertices; }
	ENGINE_API const std::vector<uint32_t> &GetIndices() const noexcept { return _indices; }
	ENGINE_API const NBounds &GetBounds() const noexcept { return _bounds; }
	ENGINE_API uint64_t GetVertexOffset() const noexcept { return _vertexOffset; }
	ENGINE_API uint64_t GetIndexOffset() const noexcept { return _indexOffset; }
	ENGINE_API const MeshGroup &GetGroup(uint32_t group) const noexcept { return _groups[group]; }
	ENGINE_API uint32_t GetGroupVertexOffset(uint32_t group) const noexcept { return _groups[group].vertexOffset; }
	ENGINE_API uint32_t GetGroupVertexCount(uint32_t group) const noexcept { return _groups[group].vertexCount; }
	ENGINE_API uint32_t GetGroupIndexOffset(uint32_t group) const noexcept { return _groups[group].indexOffset; }
	ENGINE_API uint32_t GetGroupIndexCount(uint32_t group) const noexcept { return _groups[group].indexCount; }

	ENGINE_API virtual int Load() override;
	ENGINE_API int LoadStatic(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices, bool createGroup = true, bool calculateTangents = true, bool createBounds = true);
	ENGINE_API int LoadDynamic(std::vector<Vertex> & vertices, std::vector<uint32_t> &indices, bool createGroup = true, bool calculateTangents = true, bool createBounds = true);
	ENGINE_API int CreateBuffer(bool dynamic);
	ENGINE_API virtual void CreateBounds();

	ENGINE_API void AddGroup(MeshGroup &group) { _groups.push_back(group); }
	ENGINE_API void ResetGroups() { _groups.clear(); }

	ENGINE_API void UpdateIndices(std::vector<uint32_t>& indices);
	ENGINE_API void UpdateVertices(std::vector<Vertex> &vertices);

	ENGINE_API void Release() noexcept;

	ENGINE_API virtual ~StaticMesh() noexcept;

	Buffer *GetBuffer() const noexcept { return _buffer; }	
	VkDeviceSize GetRequiredMemorySize();

	bool Upload(Buffer *buffer = nullptr);
	VkDescriptorSet CreateDescriptorSet(VkDescriptorPool pool, Buffer *uniform);
	bool BuildDrawables(NArray<Material *> &materials, VkDescriptorSet descriptorSet, NArray<Drawable> &drawables, bool buildDepth = true, bool buildBounds = true);
	virtual void DrawShadow(VkCommandBuffer commandBuffer, uint32_t shadowId, VkDescriptorSet descriptorSet) noexcept;

protected:
	Buffer *_buffer;
	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;
	std::vector<MeshGroup> _groups;
	uint32_t _indexCount;
	uint32_t _vertexCount;
	uint32_t _triangleCount;
	bool _dynamic, _hasOwnBuffer, _resident;
	NBounds _bounds;
	PrimitiveID _primitiveId;

	VkDeviceSize _vertexOffset;
	VkDeviceSize _indexOffset;

	PipelineId _depthPipelineId;
	PipelineLayoutId _depthPipelineLayoutId;

	void _CalculateTangents();
	void _BuildBounds(uint32_t group, NBounds &bounds);
};