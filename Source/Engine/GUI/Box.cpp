/* NekoEngine
 *
 * Box.cpp
 * Author: Alexandru Naiman
 *
 * Box control
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

#include <GUI/Box.h>
#include <Renderer/VKUtil.h>
#include <Renderer/DebugMarker.h>
#include <Renderer/RenderPassManager.h>

#define BOX_MODULE	"GUI_Box"

using namespace glm;

void Box::SetTexture(Texture *texture)
{
	_tex = texture;

	if (_descriptorSet == VK_NULL_HANDLE)
		return;

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = _tex->GetImageView();
	imageInfo.sampler = GUIManager::GetSampler();

	VkWriteDescriptorSet textureDescriptorWrite{};
	textureDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	textureDescriptorWrite.dstSet = _descriptorSet;
	textureDescriptorWrite.dstBinding = 0;
	textureDescriptorWrite.dstArrayElement = 0;
	textureDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureDescriptorWrite.descriptorCount = 1;
	textureDescriptorWrite.pBufferInfo = nullptr;
	textureDescriptorWrite.pImageInfo = &imageInfo;
	textureDescriptorWrite.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(VKUtil::GetDevice(), 1, &textureDescriptorWrite, 0, nullptr);

	_BuildCommandBuffer();
}

void Box::_UpdateVertices()
{
	float y{ (float)Engine::GetScreenHeight() - _controlRect.y - _controlRect.h };

	_vertices[0].posAndUV = vec4((float)_controlRect.x, (float)(y + _controlRect.h), 0, 0);
	_vertices[1].posAndUV = vec4((float)_controlRect.x, y, 0, 1);
	_vertices[2].posAndUV = vec4((float)(_controlRect.x + _controlRect.w), y, 1, 1);
	_vertices[3].posAndUV = vec4((float)(_controlRect.x + _controlRect.w), (float)(y + _controlRect.h), 1, 0);
	_vertices[0].color = _vertices[1].color = _vertices[2].color = _vertices[3].color = _color;
}

int Box::_InitializeControl()
{
	_UpdateVertices();

	_buffer = GUIManager::CreateVertexBuffer(sizeof(GUIVertex) * 4, (uint8_t *)_vertices);

	if (!_tex) _tex = Renderer::GetInstance()->GetBlankTexture();

	return ENGINE_OK;
}

int Box::_CreateDescriptorSet()
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;

	if (vkCreateDescriptorPool(VKUtil::GetDevice(), &poolInfo, VKUtil::GetAllocator(), &_descriptorPool) != VK_SUCCESS)
	{
		Logger::Log(BOX_MODULE, LOG_CRITICAL, "Failed to create descriptor pool");
		return ENGINE_DESCRIPTOR_POOL_CREATE_FAIL;
	}

	VkDescriptorSetLayout layout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_OneSampler);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, &_descriptorSet) != VK_SUCCESS)
	{
		Logger::Log(BOX_MODULE, LOG_CRITICAL, "Failed to allocate descriptor set");
		return ENGINE_DESCRIPTOR_SET_CREATE_FAIL;
	}

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = _tex->GetImageView();
	imageInfo.sampler = GUIManager::GetSampler();

	VkWriteDescriptorSet textureDescriptorWrite{};
	textureDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	textureDescriptorWrite.dstSet = _descriptorSet;
	textureDescriptorWrite.dstBinding = 0;
	textureDescriptorWrite.dstArrayElement = 0;
	textureDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureDescriptorWrite.descriptorCount = 1;
	textureDescriptorWrite.pBufferInfo = nullptr;
	textureDescriptorWrite.pImageInfo = &imageInfo;
	textureDescriptorWrite.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(VKUtil::GetDevice(), 1, &textureDescriptorWrite, 0, nullptr);

	return ENGINE_OK;
}

int Box::_BuildCommandBuffer()
{
	if (_commandBuffer != VK_NULL_HANDLE)
		VKUtil::FreeCommandBuffer(_commandBuffer);

	_commandBuffer = VKUtil::CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

	VkCommandBufferInheritanceInfo inheritanceInfo{};
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;
	inheritanceInfo.renderPass = RenderPassManager::GetRenderPass(RP_GUI);
	inheritanceInfo.subpass = 0;
	inheritanceInfo.framebuffer = Renderer::GetInstance()->GetGUIFramebuffer();

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = &inheritanceInfo;

	if (vkBeginCommandBuffer(_commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		Logger::Log(BOX_MODULE, LOG_CRITICAL, "vkBeginCommandBuffer call failed");
		return ENGINE_FAIL;
	}

	VK_DBG_MARKER_INSERT(_commandBuffer, "Box", vec4(1.0, 0.5, 0.0, 1.0));

	vkCmdBindVertexBuffers(_commandBuffer, 0, 1, &_buffer->GetHandle(), &_vertexOffset);
	vkCmdBindIndexBuffer(_commandBuffer, GUIManager::GetGUIIndexBuffer()->GetHandle(), 0, VK_INDEX_TYPE_UINT16);

	vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipeline(PIPE_GUI));
	GUIManager::BindDescriptorSet(_commandBuffer);
	vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PipelineLayoutId::PIPE_LYT_GUI), 1, 1, &_descriptorSet, 0, nullptr);

	vkCmdDrawIndexed(_commandBuffer, 6, 1, 0, 0, 0);

	if (vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS)
	{
		Logger::Log(BOX_MODULE, LOG_CRITICAL, "vkEndCommandBuffer call failed");
		return ENGINE_FAIL;
	}

	return ENGINE_OK;
}

void Box::_Update(double deltaTime)
{
	(void)deltaTime;
}

void Box::_UpdateData(void *commandBuffer)
{
	VkCommandBuffer updateBuffer{ (VkCommandBuffer)*(VkCommandBuffer *)commandBuffer };
	_buffer->UpdateData((uint8_t *)_vertices, 0, sizeof(GUIVertex) * 4, updateBuffer);
}

Box::~Box()
{
	GUIManager::FreeVertexBuffer(_buffer);
}