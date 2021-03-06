/* NekoEngine
 *
 * SkeletalMesh.cpp
 * Author: Alexandru Naiman
 *
 * SkeletalMesh class implementation 
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

#include <Renderer/VKUtil.h>
#include <Renderer/DebugMarker.h>
#include <Renderer/SkeletalMesh.h>
#include <Renderer/ShadowRenderer.h>
#include <Renderer/PipelineManager.h>
#include <Renderer/RenderPassManager.h>
#include <Engine/Vertex.h>
#include <Engine/Engine.h>
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

	_depthPipelineId = PIPE_Anim_Depth;
}

Skeleton *SkeletalMesh::CreateSkeleton()
{
	Skeleton *skel{ new Skeleton(_bones, _nodes, _globalInverseTransform) };
	
	if(skel->Load() != ENGINE_OK)
	{
		Logger::Log(SK_MESH_MODULE, LOG_CRITICAL, "Failed to load skeleton for mesh id=%s", _resourceInfo->name.c_str());
		return nullptr;
	}

	return skel;
}

int SkeletalMesh::Load()
{	
	if (AssetLoader::LoadSkeletalMesh(GetResourceInfo()->filePath, _vertices, _indices, _groups, _bones, _nodes, _globalInverseTransform) != ENGINE_OK)
	{
		Logger::Log(SK_MESH_MODULE, LOG_CRITICAL, "Failed to load mesh id=%s", _resourceInfo->name.c_str());
		return ENGINE_FAIL;
	}
	
	_indexCount = (uint32_t)_indices.size();
	_vertexCount = (uint32_t)_vertices.size();
	_triangleCount = _indexCount / 3;

	CreateBounds();	

	Logger::Log(SK_MESH_MODULE, LOG_DEBUG, "Loaded mesh id %d from %s, %d vertices, %d indices", _resourceInfo->id, *GetResourceInfo()->filePath, _vertexCount, _indexCount);
	
	return ENGINE_OK;
}

int SkeletalMesh::LoadStatic(std::vector<SkeletalVertex> &vertices, std::vector<uint32_t> &indices, bool createGroup, bool calculateTangents, bool createBounds)
{
	if (createGroup)
	{
		MeshGroup group{ 0, (uint32_t)vertices.size(), 0, (uint32_t)indices.size() };
		_groups.push_back(group);
	}

	_vertices = vertices;
	_indices = indices;

	_indexCount = (uint32_t)_indices.size();
	_vertexCount = (uint32_t)_vertices.size();
	_triangleCount = _indexCount / 3;

	if (calculateTangents) _CalculateTangents();
	if (createBounds) CreateBounds();

	return CreateBuffer(false);
}

int SkeletalMesh::LoadDynamic(std::vector<SkeletalVertex> &vertices, std::vector<uint32_t> &indices, bool createGroup, bool calculateTangents, bool createBounds)
{
	if (createGroup)
	{
		MeshGroup group{ 0, (uint32_t)vertices.size(), 0, (uint32_t)indices.size() };
		_groups.push_back(group);
	}

	_vertices = vertices;
	_indices = indices;
	
	_indexCount = (uint32_t)_indices.size();
	_vertexCount = (uint32_t)_vertices.size();
	_triangleCount = _indexCount / 3;

	if (calculateTangents) _CalculateTangents();
	if (createBounds) CreateBounds();

	_dynamic = true;

	return CreateBuffer(true);
}

void SkeletalMesh::CreateBounds()
{
	vec3 max{ 0.f };
	vec3 min{ 0.f };

	for (SkeletalVertex &v : _vertices)
	{
		if (v.position.x < min.x) min.x = v.position.x;
		if (v.position.x > max.x) max.x = v.position.x;
		if (v.position.y < min.y) min.y = v.position.y;
		if (v.position.y > max.y) max.y = v.position.y;
		if (v.position.z < min.z) min.z = v.position.z;
		if (v.position.z > max.z) max.z = v.position.z;
	}

	_bounds.InitBox(min, max);
}

VkDeviceSize SkeletalMesh::GetRequiredMemorySize()
{
	VkDeviceSize size{ sizeof(SkeletalVertex) * _vertices.size() + sizeof(uint32_t) * _indices.size() };
	if (size % 256)
	{
		size = size / 256;
		++size *= 256;
	}
	return size;
}

bool SkeletalMesh::Upload(Buffer *buffer)
{
	if (buffer == nullptr)
	{
		if (_buffer == nullptr)
			return false;
	}
	else
		_buffer = buffer;

	VkDeviceSize bufferSize = GetRequiredMemorySize();
	Buffer *stagingBuffer = new Buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, nullptr, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	uint8_t *ptr = stagingBuffer->Map();
	if (!ptr)
	{
		Logger::Log(SK_MESH_MODULE, LOG_CRITICAL, "Failed to map memory");
		return false;
	}

	memcpy(ptr, _vertices.data(), sizeof(_vertices[0]) * _vertices.size());
	memcpy(ptr + sizeof(_vertices[0]) * _vertices.size(), _indices.data(), sizeof(_indices[0]) * _indices.size());

	stagingBuffer->Unmap();

	stagingBuffer->Copy(_buffer, bufferSize);

	delete stagingBuffer;

	_vertexOffset = _buffer->GetParentOffset();
	_indexOffset = _vertexOffset + sizeof(_vertices[0]) * _vertices.size();

	_resident = true;

	return true;
}

VkDescriptorSet SkeletalMesh::CreateDescriptorSet(VkDescriptorPool pool, Buffer *uniform, Buffer *boneBuffer)
{
	VkDescriptorSet ret{};
	VkDescriptorSetLayout objectDSL{ PipelineManager::GetDescriptorSetLayout(DESC_LYT_Anim_Object) };

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &objectDSL;

	if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, &ret) != VK_SUCCESS)
	{
		Logger::Log(SK_MESH_MODULE, LOG_CRITICAL, "Failed to create descriptor set");
		return VK_NULL_HANDLE;
	}

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = uniform->GetHandle();
	bufferInfo.offset = uniform->GetParentOffset();
	bufferInfo.range = sizeof(ObjectData);

	VkDescriptorBufferInfo boneInfo{};
	boneInfo.buffer = boneBuffer->GetHandle();
	boneInfo.offset = boneBuffer->GetParentOffset();
	boneInfo.range = boneBuffer->GetSize();

	VkWriteDescriptorSet descriptorWrite[2]{};
	VKUtil::WriteDS(&descriptorWrite[0], 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo, ret, 0);
	VKUtil::WriteDS(&descriptorWrite[1], 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &boneInfo, ret, 1);
	vkUpdateDescriptorSets(VKUtil::GetDevice(), 2, descriptorWrite, 0, nullptr);

	return ret;
}

bool SkeletalMesh::BuildDrawables(NArray<Material *> &materials, VkDescriptorSet descriptorSet, NArray<Drawable> &drawables, bool buildDepth, bool buildBounds)
{
	VkDescriptorSet sceneDescriptorSet{ Renderer::GetInstance()->GetSceneDescriptorSet() };

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	
	for (size_t i = 0; i < _groups.size(); ++i)
	{
		Drawable drawable{};
		drawable.sceneCommandBuffer = Renderer::GetInstance()->CreateMeshCommandBuffer();
		drawable.depthCommandBuffer = VK_NULL_HANDLE;
		drawable.transparent = materials[i]->IsTransparent();

		if (buildBounds)
			_BuildBounds((uint32_t)i, drawable.bounds);

		if (buildDepth)
		{
			drawable.depthCommandBuffer = Renderer::GetInstance()->CreateMeshCommandBuffer();

			VkCommandBufferInheritanceInfo depthInheritanceInfo = {};
			depthInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			depthInheritanceInfo.occlusionQueryEnable = VK_FALSE;
			depthInheritanceInfo.renderPass = RenderPassManager::GetRenderPass(RP_Depth);
			depthInheritanceInfo.subpass = 0;
			depthInheritanceInfo.framebuffer = Renderer::GetInstance()->GetDepthFramebuffer();

			beginInfo.pInheritanceInfo = &depthInheritanceInfo;

			if (vkBeginCommandBuffer(drawable.depthCommandBuffer, &beginInfo) != VK_SUCCESS)
			{
				Logger::Log(SK_MESH_MODULE, LOG_CRITICAL, "vkBeginCommandBuffer (depth) call failed");
				return false;
			}

			VK_DBG_MARKER_INSERT(drawable.depthCommandBuffer, _resourceInfo ? _resourceInfo->name.c_str() : "generated mesh", vec4(0.0, 0.5, 1.0, 1.0));

			vkCmdBindVertexBuffers(drawable.depthCommandBuffer, 0, 1, &_buffer->GetHandle(), &_vertexOffset);
			vkCmdBindIndexBuffer(drawable.depthCommandBuffer, _buffer->GetHandle(), _indexOffset, VK_INDEX_TYPE_UINT32);

			PipelineId id{ _depthPipelineId };
			if (materials[i]->HasNormalMap())
				id = (PipelineId)(_depthPipelineId + 1);

			vkCmdBindPipeline(drawable.depthCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipeline(id));
			vkCmdBindDescriptorSets(drawable.depthCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_Anim_Depth), 0, 1, &sceneDescriptorSet, 0, nullptr);
			vkCmdBindDescriptorSets(drawable.depthCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_Anim_Depth), 1, 1, &descriptorSet, 0, nullptr);

			materials[i]->BindNormal(drawable.depthCommandBuffer);

			vkCmdDrawIndexed(drawable.depthCommandBuffer, _groups[i].indexCount, 1, _groups[i].indexOffset, 0, 0);

			if (vkEndCommandBuffer(drawable.depthCommandBuffer) != VK_SUCCESS)
			{
				Logger::Log(SK_MESH_MODULE, LOG_CRITICAL, "vkEndCommandBuffer (depth) call failed");
				return false;
			}
		}

		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.occlusionQueryEnable = VK_FALSE;
		inheritanceInfo.renderPass = RenderPassManager::GetRenderPass(RP_Graphics);
		inheritanceInfo.subpass = 0;
		inheritanceInfo.framebuffer = Renderer::GetInstance()->GetDrawFramebuffer();

		beginInfo.pInheritanceInfo = &inheritanceInfo;

		if (vkBeginCommandBuffer(drawable.sceneCommandBuffer, &beginInfo) != VK_SUCCESS)
		{
			Logger::Log(SK_MESH_MODULE, LOG_CRITICAL, "vkBeginCommandBuffer (scene) call failed");
			return false;
		}

		VK_DBG_MARKER_INSERT(drawable.sceneCommandBuffer, _resourceInfo ? _resourceInfo->name.c_str() : "generated mesh", vec4(1.0, 0.5, 0.0, 1.0));

		vkCmdBindVertexBuffers(drawable.sceneCommandBuffer, 0, 1, &_buffer->GetHandle(), &_vertexOffset);
		vkCmdBindIndexBuffer(drawable.sceneCommandBuffer, _buffer->GetHandle(), _indexOffset, VK_INDEX_TYPE_UINT32);

		materials[i]->Enable(drawable.sceneCommandBuffer);

		vkCmdBindDescriptorSets(drawable.sceneCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, materials[i]->GetPipelineLayout(), 0, 1, &sceneDescriptorSet, 0, nullptr);
		vkCmdBindDescriptorSets(drawable.sceneCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, materials[i]->GetPipelineLayout(), 1, 1, &descriptorSet, 0, nullptr);

		vkCmdDrawIndexed(drawable.sceneCommandBuffer, _groups[i].indexCount, 1, _groups[i].indexOffset, 0, 0);

		if (vkEndCommandBuffer(drawable.sceneCommandBuffer) != VK_SUCCESS)
		{
			Logger::Log(SK_MESH_MODULE, LOG_CRITICAL, "vkEndCommandBuffer (scene) call failed");
			return false;
		}

		drawables.Add(drawable);
	}

	return true;
}

void SkeletalMesh::DrawShadow(VkCommandBuffer commandBuffer, uint32_t shadowId, VkDescriptorSet descriptorSet) noexcept
{
	VK_DBG_MARKER_INSERT(commandBuffer, _resourceInfo ? _resourceInfo->name.c_str() : "generated mesh", vec4(0.0, 0.5, 1.0, 1.0));

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_buffer->GetHandle(), &_vertexOffset);
	vkCmdBindIndexBuffer(commandBuffer, _buffer->GetHandle(), _indexOffset, VK_INDEX_TYPE_UINT32);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipeline(PIPE_Anim_Shadow));

	VkDescriptorSet matricesDS = ShadowRenderer::GetMatricesDescriptorSet();
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_Anim_Shadow), 0, 1, &matricesDS, 0, nullptr);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_Anim_Shadow), 1, 1, &descriptorSet, 0, nullptr);

	vkCmdPushConstants(commandBuffer, PipelineManager::GetPipelineLayout(PIPE_LYT_Anim_Shadow), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &shadowId);

	for (size_t i = 0; i < _groups.size(); ++i)
		vkCmdDrawIndexed(commandBuffer, _groups[i].indexCount, 1, _groups[i].indexOffset, 0, 0);
}

void SkeletalMesh::_CalculateTangents()
{
	for (size_t i = 0; i < _indices.size(); i += 3)
	{
		SkeletalVertex &v0 = _vertices[_indices[i]];
		SkeletalVertex &v1 = _vertices[_indices[i + 1]];
		SkeletalVertex &v2 = _vertices[_indices[i + 2]];

		vec3 edge1 = v1.position - v0.position;
		vec3 edge2 = v2.position - v0.position;

		float deltaU1 = v1.uv.x - v0.uv.x;
		float deltaV1 = v1.uv.y - v0.uv.y;
		float deltaU2 = v2.uv.x - v0.uv.x;
		float deltaV2 = v2.uv.y - v0.uv.y;

		float f = 1.f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

		vec3 tgt, bitgt;

		tgt.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
		tgt.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
		tgt.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

		v0.tangent += tgt;
		v1.tangent += tgt;
		v2.tangent += tgt;
	}

	for (size_t i = 0; i < _vertices.size(); i++)
		_vertices[i].tangent = normalize(_vertices[i].tangent);
}

void SkeletalMesh::_BuildBounds(uint32_t group, NBounds &bounds)
{
	vec3 boxMin{ 0.f }, boxMax{ 0.f };

	if (!_vertices.size())
		return;

	for (uint32_t i = 0; i < _groups[group].indexCount; ++i)
	{
		SkeletalVertex &vert = _vertices[_indices[_groups[group].indexOffset + i]];

		boxMin = min(boxMin, vert.position);
		boxMax = max(boxMax, vert.position);
	}

	bounds.InitBox(boxMin, boxMax);
}

SkeletalMesh::~SkeletalMesh() noexcept
{
}
