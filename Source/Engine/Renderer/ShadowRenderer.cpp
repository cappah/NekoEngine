/* NekoEngine
 *
 * ShadowRenderer.cpp
 * Author: Alexandru Naiman
 *
 * Shadow renderer
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

#include <stack>

#include <Renderer/VKUtil.h>
#include <Renderer/DebugMarker.h>
#include <Renderer/ShadowRenderer.h>
#include <Renderer/PipelineManager.h>
#include <Renderer/RenderPassManager.h>
#include <Scene/CameraManager.h>
#include <Scene/SceneManager.h>

#define SHADOW_RENDERER_MODULE	"ShadowRenderer"

using namespace std;
using namespace glm;

struct ShadowCaster
{
	int32_t lightId;
	uint8_t mapCount;
	uint8_t mapIds[6];
};

static mat4 *_matrices{ nullptr };
static Texture *_shadowMap{ nullptr };
static Texture *_depthBuffer{ nullptr };
static Texture *_shadowTarget{ nullptr };
static VkImageView *_shadowImageViews{ nullptr };
static VkFramebuffer *_shadowFramebuffers{ nullptr }, _shadowTargetFramebuffer{ VK_NULL_HANDLE },
					 _shadowTargetFilterFramebuffer{ VK_NULL_HANDLE };
static Buffer *_matricesBuffer{ nullptr };
static NArray<ShadowCaster> _shadowCasters{};
static stack<int32_t> _freeShadowMaps{};
static VkDeviceSize _matricesBufferSize{ 0 };
static VkDescriptorPool _descriptorPool{ VK_NULL_HANDLE };
static VkDescriptorSet _matricesDescriptorSet{ VK_NULL_HANDLE }, _biasedMatricesDescriptorSet{ VK_NULL_HANDLE },
					   _shadowTargetDescriptorSet{ VK_NULL_HANDLE }, *_shadowDescriptorSets{ nullptr };

int ShadowRenderer::Initialize()
{
	VkSampleCountFlagBits samples{ VK_SAMPLE_COUNT_1_BIT };

	if (Engine::GetConfiguration().Renderer.ShadowMultisampling)
	{
		switch (Engine::GetConfiguration().Renderer.ShadowSamples)
		{
			case 2: samples = VK_SAMPLE_COUNT_2_BIT; break;
			case 4: samples = VK_SAMPLE_COUNT_4_BIT; break;
			case 8: samples = VK_SAMPLE_COUNT_8_BIT; break;
			case 16: samples = VK_SAMPLE_COUNT_16_BIT; break;
			case 32: samples = VK_SAMPLE_COUNT_32_BIT; break;
			case 64: samples = VK_SAMPLE_COUNT_64_BIT; break;
		}
	}

	_matricesBufferSize = Engine::GetConfiguration().Renderer.MaxShadowMaps * sizeof(mat4);
	
	if ((_matrices = (mat4 *)calloc(1, _matricesBufferSize * 2)) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;

	if ((_shadowImageViews = (VkImageView *)calloc(Engine::GetConfiguration().Renderer.MaxShadowMaps, sizeof(VkImageView))) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;

	if ((_shadowFramebuffers = (VkFramebuffer *)calloc(Engine::GetConfiguration().Renderer.MaxShadowMaps, sizeof(VkFramebuffer))) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;
	
	if ((_shadowDescriptorSets = (VkDescriptorSet *)calloc(Engine::GetConfiguration().Renderer.MaxShadowMaps, sizeof(VkDescriptorSet))) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;

	if ((_shadowMap = new Texture(VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
								  VK_IMAGE_TILING_OPTIMAL, Engine::GetConfiguration().Renderer.ShadowMapSize, Engine::GetConfiguration().Renderer.ShadowMapSize, 1, true, VK_NULL_HANDLE, samples, 1, Engine::GetConfiguration().Renderer.MaxShadowMaps, true)) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;
	VK_DBG_SET_OBJECT_NAME((uint64_t)_shadowMap->GetImage(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "Shadow map image");
	
	if ((_depthBuffer = new Texture(VK_FORMAT_D32_SFLOAT, VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
								  VK_IMAGE_TILING_OPTIMAL, Engine::GetConfiguration().Renderer.ShadowMapSize, Engine::GetConfiguration().Renderer.ShadowMapSize, 1, true, VK_NULL_HANDLE, samples, 1, 1, true)) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;
	VK_DBG_SET_OBJECT_NAME((uint64_t)_depthBuffer->GetImage(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "Shadow map depth buffer image");

	if ((_shadowTarget = new Texture(VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
								  VK_IMAGE_TILING_OPTIMAL, Engine::GetConfiguration().Renderer.ShadowMapSize, Engine::GetConfiguration().Renderer.ShadowMapSize, 1, true, VK_NULL_HANDLE, VK_SAMPLE_COUNT_1_BIT, 1, 1, true)) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;
	VK_DBG_SET_OBJECT_NAME((uint64_t)_shadowTarget->GetImage(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "Shadow map render target image");

	if (!_shadowMap->CreateView(VK_IMAGE_ASPECT_COLOR_BIT, true))
		return ENGINE_FAIL;
	VK_DBG_SET_OBJECT_NAME((uint64_t)_shadowMap->GetImageView(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, "Shadow map image view");

	if (!_depthBuffer->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT))
		return ENGINE_FAIL;
	VK_DBG_SET_OBJECT_NAME((uint64_t)_depthBuffer->GetImageView(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, "Shadow map depth buffer image view");

	if (!_shadowTarget->CreateView(VK_IMAGE_ASPECT_COLOR_BIT))
		return ENGINE_FAIL;
	VK_DBG_SET_OBJECT_NAME((uint64_t)_shadowTarget->GetImageView(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, "Shadow map render target image view");

	if ((_matricesBuffer = new Buffer(_matricesBufferSize * 2, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;

	{ // Framebuffers
		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.attachmentCount = 2;
		createInfo.width = createInfo.height = Engine::GetConfiguration().Renderer.ShadowMapSize;
		createInfo.layers = 1;

		VkImageView attachments[]{ _shadowTarget->GetImageView(), _depthBuffer->GetImageView() };
		createInfo.pAttachments = attachments;
		createInfo.renderPass = RenderPassManager::GetRenderPass(RP_ShadowMap);

		if (vkCreateFramebuffer(VKUtil::GetDevice(), &createInfo, VKUtil::GetAllocator(), &_shadowTargetFramebuffer) != VK_SUCCESS)
		{
			Logger::Log(SHADOW_RENDERER_MODULE, LOG_CRITICAL, "Failed to create framebuffer");
			return ENGINE_FRAMEUBFFER_CREATE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)_shadowTargetFramebuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, "Shadow render target framebuffer");

		createInfo.attachmentCount = 1;
		createInfo.renderPass = RenderPassManager::GetRenderPass(RP_ShadowFilter);

		if (vkCreateFramebuffer(VKUtil::GetDevice(), &createInfo, VKUtil::GetAllocator(), &_shadowTargetFilterFramebuffer) != VK_SUCCESS)
		{
			Logger::Log(SHADOW_RENDERER_MODULE, LOG_CRITICAL, "Failed to create framebuffer");
			return ENGINE_FRAMEUBFFER_CREATE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)_shadowTargetFilterFramebuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, "Shadow render target filter framebuffer");

		for (int32_t i = 0; i < Engine::GetConfiguration().Renderer.MaxShadowMaps; ++i)
		{
			VKUtil::CreateImageView(_shadowImageViews[i], _shadowMap->GetImage(), VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32G32_SFLOAT,
									VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, i, 1);
			VK_DBG_SET_OBJECT_NAME((uint64_t)_shadowImageViews[i], VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, "Shadow map layer image view");
			
			createInfo.pAttachments = &_shadowImageViews[i];

			if (vkCreateFramebuffer(VKUtil::GetDevice(), &createInfo, VKUtil::GetAllocator(), &_shadowFramebuffers[i]) != VK_SUCCESS)
			{
				Logger::Log(SHADOW_RENDERER_MODULE, LOG_CRITICAL, "Failed to create framebuffer");
				return ENGINE_FRAMEUBFFER_CREATE_FAIL;
			}
			VK_DBG_SET_OBJECT_NAME((uint64_t)_shadowFramebuffers[i], VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, "Shadow framebuffer");
			
			_freeShadowMaps.push(Engine::GetConfiguration().Renderer.MaxShadowMaps - i - 1);
		}
	}
	
	VkImageSubresourceRange range{};
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseArrayLayer = 0;
	range.layerCount = Engine::GetConfiguration().Renderer.MaxShadowMaps;
	range.baseMipLevel = 0;
	range.levelCount = 1;

	VKUtil::TransitionImageLayout(_shadowMap->GetImage(), VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, range);
	
	range.layerCount = 1;
	VKUtil::TransitionImageLayout(_shadowTarget->GetImage(), VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, range);

	range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;	
	VKUtil::TransitionImageLayout(_depthBuffer->GetImage(), VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, range);

	SamplerParams params{};
	params.addressU = params.addressV = params.addressW = SamplerAddressMode::ClampToBorder;
	params.magFilter = params.minFilter = SamplerFilter::Linear;
	params.mipmapMode = SamplerMipmapMode::MipmapLinear;
	params.borderColor = SamplerBorderColor::OpaqueWhite;
	_shadowMap->SetParameters(params);
	_shadowTarget->SetParameters(params);

	{ // Descriptor set
		VkDescriptorPoolSize poolSizes[2]{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[0].descriptorCount = 2;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 1 + Engine::GetConfiguration().Renderer.MaxShadowMaps;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 2;
		poolInfo.pPoolSizes = poolSizes;
		poolInfo.maxSets = 3 + Engine::GetConfiguration().Renderer.MaxShadowMaps;

		if (vkCreateDescriptorPool(VKUtil::GetDevice(), &poolInfo, VKUtil::GetAllocator(), &_descriptorPool) != VK_SUCCESS)
		{
			Logger::Log(SHADOW_RENDERER_MODULE, LOG_CRITICAL, "Failed to create descriptor pool");
			return ENGINE_DESCRIPTOR_POOL_CREATE_FAIL;
		}

		VkDescriptorSetLayout layout{ PipelineManager::GetDescriptorSetLayout(DESC_LYT_ShadowMap) };

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = _descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, &_matricesDescriptorSet) != VK_SUCCESS)
		{
			Logger::Log(SHADOW_RENDERER_MODULE, LOG_CRITICAL, "Failed to allocate shadow matrices descriptor set");
			return ENGINE_DESCRIPTOR_SET_CREATE_FAIL;
		}

		if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, &_biasedMatricesDescriptorSet) != VK_SUCCESS)
		{
			Logger::Log(SHADOW_RENDERER_MODULE, LOG_CRITICAL, "Failed to allocate shadow matrices descriptor set");
			return ENGINE_DESCRIPTOR_SET_CREATE_FAIL;
		}

		VkDescriptorBufferInfo ssboInfo{};
		ssboInfo.buffer = _matricesBuffer->GetHandle();
		ssboInfo.offset = 0;
		ssboInfo.range = _matricesBufferSize;

		VkWriteDescriptorSet descriptorWrite{};
		VKUtil::WriteDS(&descriptorWrite, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &ssboInfo, _matricesDescriptorSet, 0);
		vkUpdateDescriptorSets(VKUtil::GetDevice(), 1, &descriptorWrite, 0, nullptr);

		descriptorWrite.dstSet = _biasedMatricesDescriptorSet;

		ssboInfo.buffer = _matricesBuffer->GetHandle();
		ssboInfo.offset = _matricesBufferSize;
		ssboInfo.range = _matricesBufferSize;
		vkUpdateDescriptorSets(VKUtil::GetDevice(), 1, &descriptorWrite, 0, nullptr);

		layout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_ShadowFilter);

		if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, &_shadowTargetDescriptorSet) != VK_SUCCESS)
		{
			Logger::Log(SHADOW_RENDERER_MODULE, LOG_CRITICAL, "Failed to allocate shadow filter descriptor set");
			return ENGINE_DESCRIPTOR_SET_CREATE_FAIL;
		}

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = _shadowTarget->GetImageView();
		imageInfo.sampler = _shadowTarget->GetSampler();

		descriptorWrite.dstSet = _shadowTargetDescriptorSet;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.pBufferInfo = nullptr;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(VKUtil::GetDevice(), 1, &descriptorWrite, 0, nullptr);

		for (int i = 0; i < Engine::GetConfiguration().Renderer.MaxShadowMaps; ++i)
		{
			if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, &_shadowDescriptorSets[i]) != VK_SUCCESS)
			{
				Logger::Log(SHADOW_RENDERER_MODULE, LOG_CRITICAL, "Failed to allocate shadow filter descriptor set");
				return ENGINE_DESCRIPTOR_SET_CREATE_FAIL;
			}

			imageInfo.imageView = _shadowImageViews[i];
			descriptorWrite.dstSet = _shadowDescriptorSets[i];
			vkUpdateDescriptorSets(VKUtil::GetDevice(), 1, &descriptorWrite, 0, nullptr);
		}
	}

	return ENGINE_OK;
}

int32_t ShadowRenderer::RegisterShadowCaster(int32_t lightId, uint8_t count, uint32_t *shadowIds, uint8_t &casterId)
{
	ShadowCaster caster{};
	caster.lightId = lightId;
	caster.mapCount = count;

	for (uint8_t i = 0; i < count; ++i)
	{
		shadowIds[i] = _freeShadowMaps.top();
		caster.mapIds[i] = shadowIds[i];
		_freeShadowMaps.pop();
	}

	casterId = (uint8_t)_shadowCasters.Count();
	_shadowCasters.Add(caster);
	
	return ENGINE_OK;
}

void ShadowRenderer::UnregisterShadowCaster(uint8_t id)
{
	ShadowCaster &caster{ _shadowCasters[id] };

	for (uint8_t i = 0; i < caster.mapCount; ++i)
		_freeShadowMaps.push(caster.mapIds[i]);

	_shadowCasters.Remove(id);
}

void ShadowRenderer::GetMatrices(uint8_t id, glm::mat4 **matrices, glm::mat4 **biasedMatrices)
{
	ShadowCaster &caster{ _shadowCasters[id] };

	for (uint8_t i = 0; i < caster.mapCount; ++i)
	{
		matrices[i] = &_matrices[caster.mapIds[i]];
		biasedMatrices[i] = &_matrices[Engine::GetConfiguration().Renderer.MaxShadowMaps + caster.mapIds[i]];
	}
}

Texture *ShadowRenderer::GetShadowMap()
{
	return _shadowMap;
}

Buffer *ShadowRenderer::GetMatricesBuffer()
{
	return _matricesBuffer;
}

VkDeviceSize ShadowRenderer::GetMatricesBufferSize()
{
	return _matricesBufferSize;
}

VkDescriptorSet ShadowRenderer::GetMatricesDescriptorSet()
{
	return _matricesDescriptorSet;
}

VkDescriptorSet ShadowRenderer::GetBiasedMatricesDescriptorSet()
{
	return _biasedMatricesDescriptorSet;
}

void ShadowRenderer::BuildCommandBuffer(VkCommandBuffer commandBuffer)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	
	VkClearValue clearValues[2]{};
	clearValues[1].depthStencil = { 1.f, 0 };
	
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { (uint32_t)Engine::GetConfiguration().Renderer.ShadowMapSize, (uint32_t)Engine::GetConfiguration().Renderer.ShadowMapSize };
	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearValues;
	
	VK_DBG_MARKER_BEGIN(commandBuffer, "Shadow Pass", vec4(0.85, 0.85, 0.85, 1.0));

	VkViewport viewport{};
	VKUtil::InitViewport(&viewport, (float)Engine::GetConfiguration().Renderer.ShadowMapSize, (float)Engine::GetConfiguration().Renderer.ShadowMapSize);

	VkRect2D scissor{};
	VKUtil::InitScissor(&scissor, Engine::GetConfiguration().Renderer.ShadowMapSize, Engine::GetConfiguration().Renderer.ShadowMapSize);

	for (ShadowCaster caster : _shadowCasters)
	{
		//Light *l{ Renderer::GetInstance()->GetLight(caster.lightId) };

		for (uint8_t i = 0; i < caster.mapCount; ++i)
		{
			// 1. Render shadow map
			renderPassInfo.renderPass = RenderPassManager::GetRenderPass(RP_ShadowMap);
			renderPassInfo.framebuffer = _shadowTargetFramebuffer;

			VkImageSubresourceRange targetRange{};
			targetRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			targetRange.baseArrayLayer = 0;
			targetRange.layerCount = 1;
			targetRange.baseMipLevel = 0;
			targetRange.levelCount = 1;

			VKUtil::TransitionImageLayout(_shadowTarget->GetImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, targetRange, commandBuffer);
			VkClearColorValue clearColor{ { 1.f, 1.f, 1.f, 1.f } };
			vkCmdClearColorImage(commandBuffer, _shadowTarget->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &targetRange);
			VKUtil::TransitionImageLayout(_shadowTarget->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, targetRange, commandBuffer);
			
			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			{
				vkCmdSetDepthBias(commandBuffer, 0.f, 0.f, 0.f);
				vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
				vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

				SceneManager::GetActiveScene()->DrawShadow(commandBuffer, caster.mapIds[i]);
			}
			vkCmdEndRenderPass(commandBuffer);

			// 2. Filter shadow map and write destination
			renderPassInfo.renderPass = RenderPassManager::GetRenderPass(RP_ShadowFilter);
			renderPassInfo.framebuffer = _shadowFramebuffers[caster.mapIds[i]];

			VkImageSubresourceRange range{};
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseArrayLayer = caster.mapIds[i];
			range.layerCount = 1;
			range.baseMipLevel = 0;
			range.levelCount = 1;

			VKUtil::TransitionImageLayout(_shadowMap->GetImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, range, commandBuffer);
			vkCmdClearColorImage(commandBuffer, _shadowMap->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);
			VKUtil::TransitionImageLayout(_shadowMap->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, range, commandBuffer);

			vec4 data{};
			data[0] = 1.f / (float)Engine::GetConfiguration().Renderer.ShadowMapSize;
			data[1] = data[0];
			data[2] = 0.f;

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			{
				//data[2] = 1.f;
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipeline(PIPE_ShadowFilter));
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_ShadowFilter), 0, 1, &_shadowTargetDescriptorSet, 0, nullptr);
				vkCmdPushConstants(commandBuffer, PipelineManager::GetPipelineLayout(PIPE_LYT_ShadowFilter), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(vec4), &data);
				vkCmdDraw(commandBuffer, 3, 1, 0, 0);
			}
			vkCmdEndRenderPass(commandBuffer);

			VKUtil::TransitionImageLayout(_shadowTarget->GetImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, targetRange, commandBuffer);

			renderPassInfo.framebuffer = _shadowTargetFilterFramebuffer;
			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			{
				//data[2] = 1.f;
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipeline(PIPE_ShadowFilter));
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_ShadowFilter), 0, 1, &_shadowDescriptorSets[caster.mapIds[i]], 0, nullptr);
				vkCmdPushConstants(commandBuffer, PipelineManager::GetPipelineLayout(PIPE_LYT_ShadowFilter), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(vec4), &data);
				vkCmdDraw(commandBuffer, 3, 1, 0, 0);
			}
			vkCmdEndRenderPass(commandBuffer);

			VKUtil::TransitionImageLayout(_shadowMap->GetImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, range, commandBuffer);

			renderPassInfo.framebuffer = _shadowFramebuffers[caster.mapIds[i]];
			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			{
				//data[2] = 2.f;
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipeline(PIPE_ShadowFilter));
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_ShadowFilter), 0, 1, &_shadowTargetDescriptorSet, 0, nullptr);
				vkCmdPushConstants(commandBuffer, PipelineManager::GetPipelineLayout(PIPE_LYT_ShadowFilter), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(vec4), &data);
				vkCmdDraw(commandBuffer, 3, 1, 0, 0);
			}
			vkCmdEndRenderPass(commandBuffer);
		}
	}
	
	VK_DBG_MARKER_END(commandBuffer);
	vkEndCommandBuffer(commandBuffer);
}

void ShadowRenderer::UpdateData(VkCommandBuffer cmdBuffer)
{
	VK_DBG_MARKER_INSERT(cmdBuffer, "ShadowRenderer Update", vec4(0.83, 0.63, 0.56, 1.0));
	vkCmdUpdateBuffer(cmdBuffer, _matricesBuffer->GetHandle(), 0, _matricesBufferSize * 2, _matrices);
}

void ShadowRenderer::Release()
{
	free(_matrices);

	for (int32_t i = 0; i < Engine::GetConfiguration().Renderer.MaxShadowMaps; ++i)
	{
		if (_shadowImageViews && _shadowImageViews[i] != VK_NULL_HANDLE)
			vkDestroyImageView(VKUtil::GetDevice(), _shadowImageViews[i], VKUtil::GetAllocator());

		if (_shadowFramebuffers && _shadowFramebuffers[i] != VK_NULL_HANDLE)
			vkDestroyFramebuffer(VKUtil::GetDevice(), _shadowFramebuffers[i], VKUtil::GetAllocator());
	}

	if (_shadowTargetFramebuffer != VK_NULL_HANDLE)
		vkDestroyFramebuffer(VKUtil::GetDevice(), _shadowTargetFramebuffer, VKUtil::GetAllocator());

	if (_shadowTargetFilterFramebuffer != VK_NULL_HANDLE)
		vkDestroyFramebuffer(VKUtil::GetDevice(), _shadowTargetFilterFramebuffer, VKUtil::GetAllocator());

	if (_descriptorPool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(VKUtil::GetDevice(), _descriptorPool, VKUtil::GetAllocator());

	delete _matricesBuffer;
	delete _depthBuffer;
	delete _shadowTarget;
	delete _shadowMap;
}
