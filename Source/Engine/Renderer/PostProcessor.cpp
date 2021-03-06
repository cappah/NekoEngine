/* NekoEngine
 *
 * PostProcessor.cpp
 * Author: Alexandru Naiman
 *
 * Vulkan post processor
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

#include <Engine/Engine.h>
#include <Engine/ResourceManager.h>
#include <Renderer/VKUtil.h>
#include <Renderer/Texture.h>
#include <Renderer/DebugMarker.h>
#include <Renderer/PostProcessor.h>
#include <Renderer/PipelineManager.h>
#include <Renderer/RenderPassManager.h>

#define PP_MODULE	"PostProcessor"

#define BLOOM_LOW	3
#define BLOOM_MED	5
#define	BLOOM_HIGH	11

using namespace glm;

struct FilmGrainData
{
	float time;
};

VkCommandBuffer PostProcessor::_commandBuffer = VK_NULL_HANDLE;

static int _bloomIntensity[3]{ BLOOM_LOW, BLOOM_MED, BLOOM_HIGH };

static VkRenderPass _renderPass = VK_NULL_HANDLE, _dofRenderPass = VK_NULL_HANDLE, _fgRenderPass = VK_NULL_HANDLE;;
static VkFramebuffer _framebuffer = VK_NULL_HANDLE;
static VkDescriptorSet _ds[2]{ VK_NULL_HANDLE, VK_NULL_HANDLE };
static VkDescriptorSet _ppDS[2]{ VK_NULL_HANDLE, VK_NULL_HANDLE };
static VkDescriptorSet _brightDS{ VK_NULL_HANDLE };
static VkDescriptorSet _dofDS{ VK_NULL_HANDLE };
//static VkDescriptorSet _fgDS{ VK_NULL_HANDLE };
static VkDescriptorPool _pool = VK_NULL_HANDLE;
static VkPipeline _pipeline = VK_NULL_HANDLE, _blurPipeline = VK_NULL_HANDLE, _dofPipeline = VK_NULL_HANDLE, _fgPipeline = VK_NULL_HANDLE;
static int _bloomBlurPasses = BLOOM_HIGH;
static FilmGrainData _filmGrainData{ 0.f };

static Texture *_ppTexture0, *_ppTexture1;

int PostProcessor::Initialize()
{
	int ret = ENGINE_FAIL;

	Logger::Log(PP_MODULE, LOG_INFORMATION, "Initializing...");

	_bloomBlurPasses = _bloomIntensity[Engine::GetConfiguration().PostProcessor.BloomIntensity];

	if (!_CreateRenderPass())
		return ENGINE_FAIL;

	if ((ret = _CreatePipeline()) != ENGINE_OK)
		return ret;

	if (!_CreateFramebuffers())
		return ENGINE_FRAMEUBFFER_CREATE_FAIL;

	if (!_CreateDescriptorSets())
		return ENGINE_DESCRIPTOR_SET_CREATE_FAIL;
	
	if (!BuildCommandBuffer())
		return ENGINE_CMDBUFFER_CREATE_FAIL;

	Logger::Log(PP_MODULE, LOG_INFORMATION, "Initialized");

	return ENGINE_OK;
}

bool PostProcessor::BuildCommandBuffer()
{
	VkResult result{};
	bool odd = false;
	
	if (_commandBuffer != VK_NULL_HANDLE)
		VKUtil::FreeCommandBuffer(_commandBuffer);

	_commandBuffer = VKUtil::CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	
	if ((result = vkBeginCommandBuffer(_commandBuffer, &beginInfo)) != VK_SUCCESS)
	{
		Logger::Log(PP_MODULE, LOG_CRITICAL, "vkBeginCommandBuffer call failed");
		return false;
	}

	VK_DBG_MARKER_BEGIN(_commandBuffer, "Post processing", vec4(0.56, 0.76, 0.83, 1.0));

	VkClearValue clearValues[2]{};
	clearValues[0].color = { { 0.f, 0.f, 0.f, 0.f } };
	clearValues[1].color = { { 0.f, 0.f, 0.f, 0.f } };

	VkRenderPassBeginInfo rpInfo{};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.renderPass = _renderPass;
	rpInfo.framebuffer = _framebuffer;
	rpInfo.renderArea.offset = { 0, 0 };
	rpInfo.renderArea.extent = { Engine::GetScreenWidth(), Engine::GetScreenHeight() };
	//rpInfo.clearValueCount = 2;
	//rpInfo.pClearValues = clearValues;

	// per effect
	//vkCmdBeginRenderPass(_commandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
	// per pass
	//vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
	//vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_PostProcess), 0, 1, odd ? &_ds[1] : &_ds[0], 0, nullptr);
	//vkCmdDraw(_commandBuffer, 3, 1, 0, 0);
	//odd = !odd;
	
	VkClearColorValue clearColor{ { 0.f, 0.f, 0.f, 0.f } };
	VkImageSubresourceRange range{};
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.levelCount = 1;
	range.layerCount = 1;

	VKUtil::TransitionImageLayout(_ppTexture0->GetImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, _commandBuffer);
	VKUtil::TransitionImageLayout(_ppTexture1->GetImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, _commandBuffer);
	vkCmdClearColorImage(_commandBuffer, _ppTexture0->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);
	vkCmdClearColorImage(_commandBuffer, _ppTexture1->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);
	VKUtil::TransitionImageLayout(_ppTexture0->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, _commandBuffer);
	VKUtil::TransitionImageLayout(_ppTexture1->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, _commandBuffer);

	{ // per effect
		vkCmdBeginRenderPass(_commandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

		// per pass
		{
			if (Engine::GetConfiguration().PostProcessor.Bloom)
			{
				vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _blurPipeline);
				float data[4]{ 1.f, 0.f, 0.f, 0.f };
				bool firstRun = true;

				data[0] = 1.f / (float)_ppTexture0->GetWidth();
				data[1] = 1.f / (float)_ppTexture0->GetHeight();

				for (int i = 0; i < _bloomBlurPasses; ++i)
				{
					vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_Blur), 0, 1, odd ? &_ds[1] : &_ds[0], 0, nullptr);
					vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_Blur), 1, 1, firstRun ? &_brightDS : (odd ? &_ppDS[1] : &_ppDS[0]), 0, nullptr);
					vkCmdPushConstants(_commandBuffer, PipelineManager::GetPipelineLayout(PIPE_LYT_Blur), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float) * 4, data);
					vkCmdDraw(_commandBuffer, 3, 1, 0, 0);

					data[2] += 1.f;
					odd = !odd;
					firstRun = false;

					vkCmdNextSubpass(_commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
				}
			}
			
			vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
			vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_PostProcess), 0, 1, odd ? &_ds[1] : &_ds[0], 0, nullptr);
			vkCmdDraw(_commandBuffer, 3, 1, 0, 0);
			odd = !odd;
		}

		vkCmdEndRenderPass(_commandBuffer);
	
		// DoF
		if (Engine::GetConfiguration().PostProcessor.DepthOfField)
		{
			VKUtil::TransitionImageLayout(odd ? _ppTexture0->GetImage() : _ppTexture1->GetImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, _commandBuffer);
			vkCmdClearColorImage(_commandBuffer, odd ? _ppTexture0->GetImage() : _ppTexture1->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);
			VKUtil::TransitionImageLayout(odd ? _ppTexture0->GetImage() : _ppTexture1->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, _commandBuffer);

			rpInfo.renderPass = _dofRenderPass;
			vkCmdBeginRenderPass(_commandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _dofPipeline);
			float data[4]{ 100.f, 10.f, 1.f / 22.f, 0.f };
			vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_DoF), 0, 1, odd ? &_ds[1] : &_ds[0], 0, nullptr);
			vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_DoF), 1, 1, odd ? &_ppDS[1] : &_ppDS[0], 0, nullptr);
			vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_DoF), 2, 1, &_dofDS, 0, nullptr);
			vkCmdPushConstants(_commandBuffer, PipelineManager::GetPipelineLayout(PIPE_LYT_DoF), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float) * 4, data);
			vkCmdDraw(_commandBuffer, 3, 1, 0, 0);

			vkCmdEndRenderPass(_commandBuffer);

			odd = !odd;
		}

		// Film Grain
		if (Engine::GetConfiguration().PostProcessor.FilmGrain)
		{
			VKUtil::TransitionImageLayout(odd ? _ppTexture0->GetImage() : _ppTexture1->GetImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, _commandBuffer);
			vkCmdClearColorImage(_commandBuffer, odd ? _ppTexture0->GetImage() : _ppTexture1->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);
			VKUtil::TransitionImageLayout(odd ? _ppTexture0->GetImage() : _ppTexture1->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, _commandBuffer);

			rpInfo.renderPass = _fgRenderPass;
			vkCmdBeginRenderPass(_commandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _fgPipeline);
			vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_FilmGrain), 0, 1, odd ? &_ds[1] : &_ds[0], 0, nullptr);
			vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_FilmGrain), 1, 1, odd ? &_ppDS[1] : &_ppDS[0], 0, nullptr);
			vkCmdPushConstants(_commandBuffer, PipelineManager::GetPipelineLayout(PIPE_LYT_FilmGrain), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(FilmGrainData), &_filmGrainData);
			vkCmdDraw(_commandBuffer, 3, 1, 0, 0);

			vkCmdEndRenderPass(_commandBuffer);

			odd = !odd;
		}

		VKUtil::BlitImage(odd ? _ppTexture1->GetImage() : _ppTexture0->GetImage(), Renderer::GetInstance()->GetRenderTargetImage(), Engine::GetScreenWidth(), Engine::GetScreenHeight(), Engine::GetScreenWidth(), Engine::GetScreenHeight(), VK_FILTER_NEAREST, _commandBuffer);
	}

	VK_DBG_MARKER_END(_commandBuffer);

	if ((result = vkEndCommandBuffer(_commandBuffer)) != VK_SUCCESS)
	{
		Logger::Log(PP_MODULE, LOG_CRITICAL, "vkEndCommandBuffer call failed");
		return false;
	}

	return true;
}

void PostProcessor::ScreenResized() noexcept
{
	if (_renderPass != VK_NULL_HANDLE)
		vkDestroyRenderPass(VKUtil::GetDevice(), _renderPass, VKUtil::GetAllocator());

	if (_dofRenderPass != VK_NULL_HANDLE)
		vkDestroyRenderPass(VKUtil::GetDevice(), _dofRenderPass, VKUtil::GetAllocator());

	if (_pipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(VKUtil::GetDevice(), _pipeline, VKUtil::GetAllocator());

	if (_blurPipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(VKUtil::GetDevice(), _blurPipeline, VKUtil::GetAllocator());

	if (_dofPipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(VKUtil::GetDevice(), _dofPipeline, VKUtil::GetAllocator());

	if (_framebuffer != VK_NULL_HANDLE)
		vkDestroyFramebuffer(VKUtil::GetDevice(), _framebuffer, VKUtil::GetAllocator());

	if (!_CreateRenderPass())
	{ DIE("Failed to create render pass"); }

	if (_CreatePipeline() != ENGINE_OK)
	{ DIE("Failed to create pipeline"); }

	if (!_CreateFramebuffers())
	{ DIE("Failed to create framebuffers"); }

	_UpdateDescriptorSets();

	if (!BuildCommandBuffer())
	{ DIE("Failed to create command buffer"); }
}

bool PostProcessor::_CreateRenderPass()
{
	VkAttachmentDescription ppAttachment0{};
	ppAttachment0.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	ppAttachment0.samples = VK_SAMPLE_COUNT_1_BIT;
	ppAttachment0.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	ppAttachment0.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	ppAttachment0.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ppAttachment0.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ppAttachment0.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	ppAttachment0.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	VkAttachmentDescription ppAttachment1{};
	ppAttachment1.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	ppAttachment1.samples = VK_SAMPLE_COUNT_1_BIT;
	ppAttachment1.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	ppAttachment1.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	ppAttachment1.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ppAttachment1.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ppAttachment1.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	ppAttachment1.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference ppAttachmentRef0In{};
	ppAttachmentRef0In.attachment = 0;
	ppAttachmentRef0In.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference ppAttachmentRef1In{};
	ppAttachmentRef1In.attachment = 1;
	ppAttachmentRef1In.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference ppAttachmentRef0Out{};
	ppAttachmentRef0Out.attachment = 0;
	ppAttachmentRef0Out.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference ppAttachmentRef1Out{};
	ppAttachmentRef1Out.attachment = 1;
	ppAttachmentRef1Out.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 2;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	NArray<VkSubpassDescription> subpassDesc;
	NArray<VkSubpassDependency> subpassDepend;

	VkSubpassDependency startDependency{};
	startDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	startDependency.dstSubpass = 0;
	startDependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	startDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	startDependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	startDependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	startDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	subpassDepend.Add(startDependency);

	VkAttachmentReference ppInAttachments0[2]{ ppAttachmentRef0In, colorAttachmentRef };
	VkAttachmentReference ppInAttachments1[2]{ ppAttachmentRef1In, colorAttachmentRef };
	uint32_t preserve = 2;

	{ // For ea effect
		if (Engine::GetConfiguration().PostProcessor.Bloom)
		{
			for (int i = 0; i < _bloomBlurPasses; ++i)
			{
				VkSubpassDescription subpass{};
				subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass.colorAttachmentCount = 1;
				subpass.pColorAttachments = subpassDesc.Count() % 2 ? &ppAttachmentRef0Out : &ppAttachmentRef1Out;
				subpass.inputAttachmentCount = 2;
				subpass.pInputAttachments = subpassDesc.Count() % 2 ? ppInAttachments1 : ppInAttachments0;
				subpass.pDepthStencilAttachment = nullptr;
				subpass.preserveAttachmentCount = 1;
				subpass.pPreserveAttachments = &preserve;
				subpassDesc.Add(subpass);

				VkSubpassDependency dependency{};
				dependency.srcSubpass = (uint32_t)subpassDesc.Count() - 1;
				dependency.dstSubpass = (uint32_t)subpassDesc.Count();
				dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
				dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
				subpassDepend.Add(dependency);
			}
		}

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = subpassDesc.Count() % 2 ? &ppAttachmentRef0Out : &ppAttachmentRef1Out;
		subpass.inputAttachmentCount = 2;
		subpass.pInputAttachments = subpassDesc.Count() % 2 ? ppInAttachments1 : ppInAttachments0;
		subpass.pDepthStencilAttachment = nullptr;
		subpassDesc.Add(subpass);
	}

	VkSubpassDependency endDependency{};
	endDependency.srcSubpass = (uint32_t)subpassDesc.Count() - 1;
	endDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
	endDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	endDependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	endDependency.srcAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	endDependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	endDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	subpassDepend.Add(endDependency);

	VkAttachmentDescription attachments[] = { ppAttachment0, ppAttachment1, colorAttachment };

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 3;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = (uint32_t)subpassDesc.Count();
	renderPassInfo.pSubpasses = *subpassDesc;
	renderPassInfo.dependencyCount = (uint32_t)subpassDepend.Count();
	renderPassInfo.pDependencies = *subpassDepend;

	if (vkCreateRenderPass(VKUtil::GetDevice(), &renderPassInfo, VKUtil::GetAllocator(), &_renderPass) != VK_SUCCESS)
	{
		Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to create render pass");
		return false;
	}

	subpassDesc.Clear(false);
	subpassDepend.Clear(false);
	
	{
		if (Engine::GetConfiguration().PostProcessor.DepthOfField)
		{
			subpassDepend.Add(startDependency);

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = subpassDesc.Count() % 2 ? &ppAttachmentRef0Out : &ppAttachmentRef1Out;
			subpass.inputAttachmentCount = 2;
			subpass.pInputAttachments = subpassDesc.Count() % 2 ? ppInAttachments1 : ppInAttachments0;
			subpass.pDepthStencilAttachment = nullptr;
			subpass.preserveAttachmentCount = 1;
			subpass.pPreserveAttachments = &preserve;
			subpassDesc.Add(subpass);

			endDependency.srcSubpass = (uint32_t)subpassDesc.Count() - 1;
			subpassDepend.Add(endDependency);

			renderPassInfo.subpassCount = (uint32_t)subpassDesc.Count();
			renderPassInfo.pSubpasses = *subpassDesc;
			renderPassInfo.dependencyCount = (uint32_t)subpassDepend.Count();
			renderPassInfo.pDependencies = *subpassDepend;

			if (vkCreateRenderPass(VKUtil::GetDevice(), &renderPassInfo, VKUtil::GetAllocator(), &_dofRenderPass) != VK_SUCCESS)
			{
				Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to create render pass");
				return false;
			}
		}
	}
	subpassDesc.Clear(false);
	subpassDepend.Clear(false);

	{
		if (Engine::GetConfiguration().PostProcessor.FilmGrain)
		{
			subpassDepend.Add(startDependency);

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = subpassDesc.Count() % 2 ? &ppAttachmentRef0Out : &ppAttachmentRef1Out;
			subpass.inputAttachmentCount = 2;
			subpass.pInputAttachments = subpassDesc.Count() % 2 ? ppInAttachments1 : ppInAttachments0;
			subpass.pDepthStencilAttachment = nullptr;
			subpass.preserveAttachmentCount = 1;
			subpass.pPreserveAttachments = &preserve;
			subpassDesc.Add(subpass);

			endDependency.srcSubpass = (uint32_t)subpassDesc.Count() - 1;
			subpassDepend.Add(endDependency);

			renderPassInfo.subpassCount = (uint32_t)subpassDesc.Count();
			renderPassInfo.pSubpasses = *subpassDesc;
			renderPassInfo.dependencyCount = (uint32_t)subpassDepend.Count();
			renderPassInfo.pDependencies = *subpassDepend;

			if (vkCreateRenderPass(VKUtil::GetDevice(), &renderPassInfo, VKUtil::GetAllocator(), &_fgRenderPass) != VK_SUCCESS)
			{
				Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to create render pass");
				return false;
			}
		}
	}
	
	return true;
}

int PostProcessor::_CreatePipeline()
{
	VkGraphicsPipelineCreateInfo pipelineInfo{};

	ShaderModule *vs = (ShaderModule *)ResourceManager::GetResourceByName("sh_fullscreen_vertex", ResourceType::RES_SHADERMODULE);
	ShaderModule *fs = (ShaderModule *)ResourceManager::GetResourceByName("sh_pp_hdr", ResourceType::RES_SHADERMODULE);

	if (!vs || !fs)
	{
		Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to load post process shaders");
		return ENGINE_LOAD_SHADER_FAIL;
	}

	VkPipelineShaderStageCreateInfo fsVertShaderStageInfo{};
	VKUtil::InitShaderStage(&fsVertShaderStageInfo, VK_SHADER_STAGE_VERTEX_BIT, vs->GetHandle());

	VkPipelineShaderStageCreateInfo hdrFragShaderStageInfo{};
	VKUtil::InitShaderStage(&hdrFragShaderStageInfo, VK_SHADER_STAGE_FRAGMENT_BIT, fs->GetHandle());

	VkPipelineShaderStageCreateInfo hdrShaderStages[]{ fsVertShaderStageInfo, hdrFragShaderStageInfo };

	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = hdrShaderStages;

	VkPipelineVertexInputStateCreateInfo emptyVertexInputInfo{};
	VKUtil::InitVertexInput(&emptyVertexInputInfo);
	
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	VKUtil::InitInputAssembly(&inputAssembly, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	VkViewport viewport{};
	VKUtil::InitViewport(&viewport, (float)Engine::GetScreenWidth(), (float)Engine::GetScreenHeight());

	VkRect2D scissor{};
	VKUtil::InitScissor(&scissor, Engine::GetScreenWidth(), Engine::GetScreenHeight());

	VkPipelineViewportStateCreateInfo viewportState{};
	VKUtil::InitViewportState(&viewportState, 1, &viewport, 1, &scissor);

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	VKUtil::InitRasterizationState(&rasterizer, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE);

	VkPipelineMultisampleStateCreateInfo multisampling{};
	VKUtil::InitMultisampleState(&multisampling, VK_SAMPLE_COUNT_1_BIT);

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	VKUtil::InitDepthState(&depthStencil, VK_FALSE, VK_FALSE);

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	VKUtil::InitColorBlendAttachmentState(&colorBlendAttachment, VK_FALSE);

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	VKUtil::InitColorBlendState(&colorBlending, 1, &colorBlendAttachment);

	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
	pipelineInfo.pVertexInputState = &emptyVertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = PipelineManager::GetPipelineLayout(PIPE_LYT_PostProcess);
	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &_pipeline) != VK_SUCCESS)
	{
		Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to create pipeline (HDR)");
		return false;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)_pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "HDR");

	if (Engine::GetConfiguration().PostProcessor.Bloom)
	{
		ShaderModule *blur = (ShaderModule *)ResourceManager::GetResourceByName("sh_pp_blur", ResourceType::RES_SHADERMODULE);

		if (!blur)
		{
			Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to load post process shaders");
			return ENGINE_LOAD_SHADER_FAIL;
		}

		VkPipelineShaderStageCreateInfo blurFragShaderStageInfo{};
		VKUtil::InitShaderStage(&blurFragShaderStageInfo, VK_SHADER_STAGE_FRAGMENT_BIT, blur->GetHandle());

		pipelineInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
		pipelineInfo.basePipelineHandle = _pipeline;
		pipelineInfo.layout = PipelineManager::GetPipelineLayout(PIPE_LYT_Blur);

		VkPipelineShaderStageCreateInfo blurShaderStages[]{ fsVertShaderStageInfo, blurFragShaderStageInfo };
		pipelineInfo.pStages = blurShaderStages;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &_blurPipeline) != VK_SUCCESS)
		{
			Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to create pipeline (blur)");
			return false;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)_blurPipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "Blur");
	}

	if (Engine::GetConfiguration().PostProcessor.DepthOfField)
	{
		ShaderModule *dof = (ShaderModule *)ResourceManager::GetResourceByName("sh_pp_dof", ResourceType::RES_SHADERMODULE);

		if (!dof)
		{
			Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to load post process shaders");
			return ENGINE_LOAD_SHADER_FAIL;
		}

		VkPipelineShaderStageCreateInfo dofFragShaderStageInfo{};
		VKUtil::InitShaderStage(&dofFragShaderStageInfo, VK_SHADER_STAGE_FRAGMENT_BIT, dof->GetHandle());

		pipelineInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
		pipelineInfo.basePipelineHandle = _pipeline;
		pipelineInfo.renderPass = _dofRenderPass;
		pipelineInfo.layout = PipelineManager::GetPipelineLayout(PIPE_LYT_DoF);

		VkPipelineShaderStageCreateInfo dofShaderStages[]{ fsVertShaderStageInfo, dofFragShaderStageInfo };
		pipelineInfo.pStages = dofShaderStages;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &_dofPipeline) != VK_SUCCESS)
		{
			Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to create pipeline (DoF)");
			return false;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)_dofPipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "DoF");
	}

	if (Engine::GetConfiguration().PostProcessor.FilmGrain)
	{
		ShaderModule *filmgrain{ (ShaderModule *)ResourceManager::GetResourceByName("sh_pp_filmgrain", ResourceType::RES_SHADERMODULE) };

		if (!filmgrain)
		{
			Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to load post process shaders");
			return ENGINE_LOAD_SHADER_FAIL;
		}

		VkPipelineShaderStageCreateInfo fgFragShaderStageInfo{};
		VKUtil::InitShaderStage(&fgFragShaderStageInfo, VK_SHADER_STAGE_FRAGMENT_BIT, filmgrain->GetHandle());

		pipelineInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
		pipelineInfo.basePipelineHandle = _pipeline;
		pipelineInfo.renderPass = _fgRenderPass;
		pipelineInfo.layout = PipelineManager::GetPipelineLayout(PIPE_LYT_FilmGrain);

		VkPipelineShaderStageCreateInfo fgShaderStages[]{ fsVertShaderStageInfo, fgFragShaderStageInfo };
		pipelineInfo.pStages = fgShaderStages;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &_dofPipeline) != VK_SUCCESS)
		{
			Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to create pipeline (FilmGrain)");
			return false;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)_dofPipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "FilmGrain");
	}

	return ENGINE_OK;
}

bool PostProcessor::_CreateFramebuffers()
{
	_ppTexture0 = new Texture(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_IMAGE_TILING_OPTIMAL, Engine::GetScreenWidth(), Engine::GetScreenHeight(), 1, true);
	_ppTexture0->CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
	_ppTexture1 = new Texture(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_IMAGE_TILING_OPTIMAL, Engine::GetScreenWidth(), Engine::GetScreenHeight(), 1, true);
	_ppTexture1->CreateView(VK_IMAGE_ASPECT_COLOR_BIT);

	VKUtil::TransitionImageLayout(_ppTexture0->GetImage(), VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
	VKUtil::TransitionImageLayout(_ppTexture1->GetImage(), VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_DBG_SET_OBJECT_NAME((uint64_t)_ppTexture0->GetImage(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "Post process texture 0");
	VK_DBG_SET_OBJECT_NAME((uint64_t)_ppTexture1->GetImage(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "Post process texture 1");

	VkImageView attachments[]{ _ppTexture0->GetImageView(), _ppTexture1->GetImageView(), Renderer::GetInstance()->GetRenderTargetImageView() };

	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = _renderPass;
	createInfo.attachmentCount = 3;
	createInfo.pAttachments = attachments;
	createInfo.width = Engine::GetScreenWidth();
	createInfo.height = Engine::GetScreenHeight();
	createInfo.layers = 1;

	if (vkCreateFramebuffer(VKUtil::GetDevice(), &createInfo, VKUtil::GetAllocator(), &_framebuffer) != VK_SUCCESS)
	{
		Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to create framebuffer");
		return false;
	}

	VK_DBG_SET_OBJECT_NAME((uint64_t)_framebuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, "Post process framebuffer");

	return true;
}

bool PostProcessor::_CreateDescriptorSets()
{
	if (_pool == VK_NULL_HANDLE)
	{
		VkDescriptorPoolSize poolSizes[2]{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		poolSizes[0].descriptorCount = 4;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 4;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 2;
		poolInfo.pPoolSizes = poolSizes;
		poolInfo.maxSets = 7;

		if (vkCreateDescriptorPool(VKUtil::GetDevice(), &poolInfo, VKUtil::GetAllocator(), &_pool) != VK_SUCCESS)
		{
			Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to create descriptor pool");
			return false;
		}

		VkDescriptorSetLayout dsLayout[]{ PipelineManager::GetDescriptorSetLayout(DESC_LYT_PostProcess), PipelineManager::GetDescriptorSetLayout(DESC_LYT_PostProcess) };
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = _pool;
		allocInfo.descriptorSetCount = 2;
		allocInfo.pSetLayouts = dsLayout;

		if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, _ds) != VK_SUCCESS)
		{
			Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to allocate descriptor set");
			return false;
		}

		VkDescriptorSetLayout brightLayout[]{ PipelineManager::GetDescriptorSetLayout(DESC_LYT_OneSampler) };
		allocInfo.pSetLayouts = brightLayout;
		allocInfo.descriptorSetCount = 1;

		if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, &_brightDS) != VK_SUCCESS)
		{
			Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to allocate descriptor set");
			return false;
		}

		VkDescriptorSetLayout ppLayouts[]{ PipelineManager::GetDescriptorSetLayout(DESC_LYT_OneSampler), PipelineManager::GetDescriptorSetLayout(DESC_LYT_OneSampler) };
		allocInfo.pSetLayouts = ppLayouts;
		allocInfo.descriptorSetCount = 2;

		if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, _ppDS) != VK_SUCCESS)
		{
			Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to allocate descriptor set");
			return false;
		}

		VkDescriptorSetLayout dofLayout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_OneSampler);
		allocInfo.pSetLayouts = &dofLayout;
		allocInfo.descriptorSetCount = 1;

		if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, &_dofDS) != VK_SUCCESS)
		{
			Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to allocate descriptor set");
			return false;
		}
	}

	_UpdateDescriptorSets();

	return true;
}

void PostProcessor::_UpdateDescriptorSets()
{
	VkDescriptorImageInfo img0Info{};
	img0Info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	img0Info.imageView = _ppTexture0->GetImageView();
	img0Info.sampler = Renderer::GetInstance()->GetNearestSampler();

	VkDescriptorImageInfo img1Info{};
	img1Info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	img1Info.imageView = _ppTexture1->GetImageView();
	img1Info.sampler = Renderer::GetInstance()->GetNearestSampler();

	VkDescriptorImageInfo colorInfo{};
	colorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	colorInfo.imageView = Renderer::GetInstance()->GetRenderTargetImageView();
	colorInfo.sampler = Renderer::GetInstance()->GetNearestSampler();

	VkDescriptorImageInfo brightInfo{};
	brightInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	brightInfo.imageView = Renderer::GetInstance()->GetBrightnessImageView();
	brightInfo.sampler = Renderer::GetInstance()->GetNearestSampler();

	VkDescriptorImageInfo depthInfo{};
	depthInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	depthInfo.imageView = Renderer::GetInstance()->GetDepthImageView();
	depthInfo.sampler = Renderer::GetInstance()->GetDepthSampler();

	VkWriteDescriptorSet writeDS[8]{};
	VKUtil::WriteDS(&writeDS[0], 1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, &img0Info, _ds[0], 0);
	VKUtil::WriteDS(&writeDS[1], 1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, &colorInfo, _ds[0], 1);
	VKUtil::WriteDS(&writeDS[2], 1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, &img1Info, _ds[1], 0);
	VKUtil::WriteDS(&writeDS[3], 1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, &colorInfo, _ds[1], 1);
	VKUtil::WriteDS(&writeDS[4], 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &brightInfo, _brightDS, 0);
	VKUtil::WriteDS(&writeDS[5], 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &img0Info, _ppDS[0], 0);
	VKUtil::WriteDS(&writeDS[6], 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &img1Info, _ppDS[1], 0);
	VKUtil::WriteDS(&writeDS[7], 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &depthInfo, _dofDS, 0);

	vkUpdateDescriptorSets(VKUtil::GetDevice(), 8, writeDS, 0, nullptr);
}

Texture *PostProcessor::_GetPPTexture0() { return _ppTexture0; }
Texture *PostProcessor::_GetPPTexture1() { return _ppTexture1; }

void PostProcessor::Release()
{
	if (_commandBuffer != VK_NULL_HANDLE)
		VKUtil::FreeCommandBuffer(_commandBuffer);

	delete _ppTexture0;
	delete _ppTexture1;

	if (_renderPass != VK_NULL_HANDLE)
		vkDestroyRenderPass(VKUtil::GetDevice(), _renderPass, VKUtil::GetAllocator());

	if (_dofRenderPass != VK_NULL_HANDLE)
		vkDestroyRenderPass(VKUtil::GetDevice(), _dofRenderPass, VKUtil::GetAllocator());

	if (_pipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(VKUtil::GetDevice(), _pipeline, VKUtil::GetAllocator());

	if (_blurPipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(VKUtil::GetDevice(), _blurPipeline, VKUtil::GetAllocator());

	if (_dofPipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(VKUtil::GetDevice(), _dofPipeline, VKUtil::GetAllocator());

	if (_framebuffer != VK_NULL_HANDLE)
		vkDestroyFramebuffer(VKUtil::GetDevice(), _framebuffer, VKUtil::GetAllocator());

	if (_pool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(VKUtil::GetDevice(), _pool, VKUtil::GetAllocator());

	Logger::Log(PP_MODULE, LOG_INFORMATION, "Released");
}
