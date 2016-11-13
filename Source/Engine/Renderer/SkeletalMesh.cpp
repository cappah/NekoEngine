/* NekoEngine
 *
 * SkeletalMesh.cpp
 * Author: Alexandru Naiman
 *
 * SkeletalMesh class implementation 
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

#include <glm/glm.hpp>

#include <Renderer/Debug.h>
#include <Renderer/VKUtil.h>
#include <Renderer/SkeletalMesh.h>
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
}

Skeleton *SkeletalMesh::CreateSkeleton()
{
	Skeleton *skel = new Skeleton(_bones, _nodes, _globalInverseTransform);
	
	if(skel->Load() != ENGINE_OK)
	{
		Logger::Log(SK_MESH_MODULE, LOG_CRITICAL, "Failed to load skeleton for mesh id=%s", _resourceInfo->name.c_str());
		return nullptr;
	}

	return skel;
}

int SkeletalMesh::Load()
{	
	_groupOffset.push_back(0);

	if (AssetLoader::LoadSkeletalMesh(GetResourceInfo()->filePath, _vertices, _indices, _groupOffset, _groupCount, _bones, _nodes, _globalInverseTransform) != ENGINE_OK)
	{
		Logger::Log(SK_MESH_MODULE, LOG_CRITICAL, "Failed to load mesh id=%s", _resourceInfo->name.c_str());
		return ENGINE_FAIL;
	}
	
	_indexCount = (uint32_t)_indices.size();
	_vertexCount = (uint32_t)_vertices.size();
	_triangleCount = _indexCount / 3;
	
	Logger::Log(SK_MESH_MODULE, LOG_DEBUG, "Loaded mesh id %d from %s, %d vertices, %d indices", _resourceInfo->id, *GetResourceInfo()->filePath, _vertexCount, _indexCount);
	
	return ENGINE_OK;
}

int SkeletalMesh::LoadStatic(std::vector<SkeletalVertex> &vertices, std::vector<uint32_t> &indices, bool createGroup, bool calculateTangents)
{
	if (createGroup)
	{
		_groupOffset.push_back(0);
		_groupCount.push_back((uint32_t)indices.size());
	}

	_vertices = vertices;
	_indices = indices;

	if(calculateTangents) _CalculateTangents();

	return CreateBuffer(false);
}

int SkeletalMesh::LoadDynamic(std::vector<SkeletalVertex> &vertices, std::vector<uint32_t> &indices, bool createGroup, bool calculateTangents)
{
	if (createGroup)
	{
		_groupOffset.push_back(0);
		_groupCount.push_back((uint32_t)indices.size());
	}

	_vertices = vertices;
	_indices = indices;

	if(calculateTangents) _CalculateTangents();

	_dynamic = true;

	return CreateBuffer(true);
}

VkDeviceSize SkeletalMesh::GetRequiredMemorySize()
{
	VkDeviceSize size = sizeof(SkeletalVertex) * _vertices.size() + sizeof(uint32_t) * _indices.size();
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
	VkDescriptorSet ret;
	VkDescriptorSetLayout objectDSL = PipelineManager::GetDescriptorSetLayout(DESC_LYT_Anim_Object);

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
	descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[0].dstSet = ret;
	descriptorWrite[0].dstBinding = 0;
	descriptorWrite[0].dstArrayElement = 0;
	descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite[0].descriptorCount = 1;
	descriptorWrite[0].pBufferInfo = &bufferInfo;
	descriptorWrite[0].pImageInfo = nullptr;
	descriptorWrite[0].pTexelBufferView = nullptr;
	descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[1].dstSet = ret;
	descriptorWrite[1].dstBinding = 1;
	descriptorWrite[1].dstArrayElement = 0;
	descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrite[1].descriptorCount = 1;
	descriptorWrite[1].pBufferInfo = &boneInfo;
	descriptorWrite[1].pImageInfo = nullptr;
	descriptorWrite[1].pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(VKUtil::GetDevice(), 2, descriptorWrite, 0, nullptr);

	return ret;
}

bool SkeletalMesh::BuildCommandBuffers(NArray<Material *> &_materials, VkDescriptorSet descriptorSet, VkCommandBuffer &depthCmdBuffer, VkCommandBuffer &sceneCmdBuffer)
{
	VkDescriptorSet sceneDescriptorSet = Renderer::GetInstance()->GetSceneDescriptorSet();

	VkCommandBufferInheritanceInfo depthInheritanceInfo = {};
	depthInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	depthInheritanceInfo.occlusionQueryEnable = VK_FALSE;
	depthInheritanceInfo.renderPass = RenderPassManager::GetRenderPass(RP_Depth);
	depthInheritanceInfo.subpass = 0;
	depthInheritanceInfo.framebuffer = Renderer::GetInstance()->GetDepthFramebuffer();

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = &depthInheritanceInfo;

	if (depthCmdBuffer != VK_NULL_HANDLE)
	{
		if (vkBeginCommandBuffer(depthCmdBuffer, &beginInfo) != VK_SUCCESS)
		{
			Logger::Log(SK_MESH_MODULE, LOG_CRITICAL, "vkBeginCommandBuffer (depth) call failed");
			return false;
		}

		DBG_MARKER_INSERT(depthCmdBuffer, _resourceInfo ? _resourceInfo->name.c_str() : "generated mesh", vec4(0.0, 0.5, 1.0, 1.0));

		vkCmdBindVertexBuffers(depthCmdBuffer, 0, 1, &_buffer->GetHandle(), &_vertexOffset);
		vkCmdBindIndexBuffer(depthCmdBuffer, _buffer->GetHandle(), _indexOffset, VK_INDEX_TYPE_UINT32);

		vkCmdBindPipeline(depthCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipeline(PIPE_Anim_Depth));
		vkCmdBindDescriptorSets(depthCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_Anim_Depth), 0, 1, &sceneDescriptorSet, 0, nullptr);
		vkCmdBindDescriptorSets(depthCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_Anim_Depth), 1, 1, &descriptorSet, 0, nullptr);

		for (size_t i = 0; i < _groupCount.size(); ++i)
			vkCmdDrawIndexed(depthCmdBuffer, _groupCount[i], 1, _groupOffset[i], 0, 0);

		if (vkEndCommandBuffer(depthCmdBuffer) != VK_SUCCESS)
		{
			Logger::Log(SK_MESH_MODULE, LOG_CRITICAL, "vkEndCommandBuffer (depth) call failed");
			return false;
		}
	}

	// Draw command buffer

	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;
	inheritanceInfo.renderPass = RenderPassManager::GetRenderPass(RP_Graphics);
	inheritanceInfo.subpass = 0;
	inheritanceInfo.framebuffer = Renderer::GetInstance()->GetDrawFramebuffer();

	beginInfo.pInheritanceInfo = &inheritanceInfo;

	if (vkBeginCommandBuffer(sceneCmdBuffer, &beginInfo) != VK_SUCCESS)
	{
		Logger::Log(SK_MESH_MODULE, LOG_CRITICAL, "vkBeginCommandBuffer (scene) call failed");
		return false;
	}

	DBG_MARKER_INSERT(sceneCmdBuffer, _resourceInfo ? _resourceInfo->name.c_str() : "generated mesh", vec4(1.0, 0.5, 0.0, 1.0));

	vkCmdBindVertexBuffers(sceneCmdBuffer, 0, 1, &_buffer->GetHandle(), &_vertexOffset);
	vkCmdBindIndexBuffer(sceneCmdBuffer, _buffer->GetHandle(), _indexOffset, VK_INDEX_TYPE_UINT32);

	for (size_t i = 0; i < _groupCount.size(); ++i)
	{
		_materials[i]->Enable(sceneCmdBuffer);

		vkCmdBindDescriptorSets(sceneCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _materials[i]->GetPipelineLayout(), 0, 1, &sceneDescriptorSet, 0, nullptr);
		vkCmdBindDescriptorSets(sceneCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _materials[i]->GetPipelineLayout(), 1, 1, &descriptorSet, 0, nullptr);

		vkCmdDrawIndexed(sceneCmdBuffer, _groupCount[i], 1, _groupOffset[i], 0, 0);
	}

	if (vkEndCommandBuffer(sceneCmdBuffer) != VK_SUCCESS)
	{
		Logger::Log(SK_MESH_MODULE, LOG_CRITICAL, "vkEndCommandBuffer (scene) call failed");
		return false;
	}

	return true;
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

SkeletalMesh::~SkeletalMesh() noexcept
{
}
