/* NekoEngine
 *
 * SSAO.cpp
 * Author: Alexandru Naiman
 *
 * Screen-Space Ambient Occlusion
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

#include <random>

#include <System/Logger.h>
#include <Renderer/SSAO.h>
#include <Renderer/VKUtil.h>
#include <Renderer/DebugMarker.h>
#include <Renderer/PostProcessor.h>
#include <Engine/ResourceManager.h>
#include <Scene/CameraManager.h>

#define SSAO_MODULE		"SSAO"

using namespace glm;
using namespace std;

Texture *_aoTexture = nullptr, *_blurTexture = nullptr, *_noiseTexture = nullptr;
int32_t _noiseSize = 16;
vec2 _noise[SSAO_MAX_NOISE];

typedef struct SSAO_DATA
{
	mat4 inverseView;
	mat4 inverseProjection;
	vec4 frameAndNoise;
	float kernelSize;
	float radius;
	float powerExponent;
	float threshold;
	float zNear;
	float zFar;
	int32_t numSamples;
	float bias;
	vec4 kernel[SSAO_MAX_SAMPLES];
} SSAODataBlock;

SSAODataBlock _dataBlock;
int _blurRadius;

Buffer *_dataBuffer, *_blurUbo;

VkPipeline _aoPipeline = VK_NULL_HANDLE, _blurPipeline = VK_NULL_HANDLE;
VkPipelineLayout _aoPipelineLayout = VK_NULL_HANDLE, _blurPipelineLayout = VK_NULL_HANDLE;
VkDescriptorSetLayout _aoDescriptorSetLayout = VK_NULL_HANDLE, _blurDescriptorSetLayout = VK_NULL_HANDLE;
VkDescriptorSet _aoDescriptorSet = VK_NULL_HANDLE, _blurDescriptorSet = VK_NULL_HANDLE;
VkDescriptorPool _pool = VK_NULL_HANDLE;
VkRenderPass _renderPass = VK_NULL_HANDLE;
static VkCommandBuffer _commandBuffer = VK_NULL_HANDLE;
static VkFramebuffer _framebuffer = VK_NULL_HANDLE;

int SSAO::Initialize()
{
	int ret = ENGINE_FAIL;

	Logger::Log(SSAO_MODULE, LOG_INFORMATION, "Initializing...");

	memset(&_dataBlock, 0x0, sizeof(SSAODataBlock));

	NE_SRANDOM((unsigned int)time(NULL));

	_dataBlock.kernelSize = (float)Engine::GetConfiguration().Renderer.SSAO.KernelSize;
	_dataBlock.radius = Engine::GetConfiguration().Renderer.SSAO.Radius;
	_dataBlock.powerExponent = Engine::GetConfiguration().Renderer.SSAO.PowerExponent;
	_dataBlock.threshold = Engine::GetConfiguration().Renderer.SSAO.Threshold;
	_dataBlock.bias = Engine::GetConfiguration().Renderer.SSAO.Bias;

	if (Engine::GetConfiguration().Renderer.SSAO.Multisampling)
		_dataBlock.numSamples = Engine::GetConfiguration().Renderer.Multisampling ? Engine::GetConfiguration().Renderer.Samples : 1;
	else
		_dataBlock.numSamples = 1;

	uniform_real_distribution<float> rd(0.0, 1.0);
	default_random_engine generator;
	for (uint32_t i = 0; i < _dataBlock.kernelSize; ++i)
	{
		vec3 sample = vec3(rd(generator) * 2.0 - 1.0, rd(generator) * 2.0 - 1.0, rd(generator));
		sample = normalize(sample);
		sample *= rd(generator);
		float scale = float(i) / _dataBlock.kernelSize;
		scale = .1f + (scale * scale) * .9f;
		sample *= scale;
		_dataBlock.kernel[i] = vec4(sample, 0.f);
	}
	
	for (uint32_t i = 0; i < 16; i++)
		_noise[i] = vec2(rd(generator) * 2.0 - 1.0, rd(generator) * 2.0 - 1.0);

	_CreateTextures();

	_dataBlock.frameAndNoise = vec4(Engine::GetScreenWidth(), Engine::GetScreenHeight(), Engine::GetScreenWidth() / (_noiseSize / 4), Engine::GetScreenHeight() / (_noiseSize / 4));

	if ((_dataBuffer = new Buffer(sizeof(SSAODataBlock), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, (uint8_t *)&_dataBlock, VK_NULL_HANDLE, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) == nullptr)
	{ DIE("Out of resources"); }

	if (!_CreateRenderPass())
		return ENGINE_RENDERPASS_CREATE_FAIL;

	if ((ret = _CreateDescriptorSets()) != ENGINE_OK)
		return ret;

	if ((ret = _CreatePipelineLayout()) != ENGINE_OK)
		return ret;

	if ((ret = _CreatePipeline()) != ENGINE_OK)
		return ret;

	if (!_CreateFramebuffers())
		return ENGINE_FRAMEUBFFER_CREATE_FAIL;
	
	if ((ret = BuildCommandBuffer()) != ENGINE_OK)
		return ret;

	Logger::Log(SSAO_MODULE, LOG_INFORMATION, "Initialized");

	return ENGINE_OK;
}

VkImageView SSAO::GetAOImageView()
{
	return _aoTexture->GetImageView();
}

void SSAO::UpdateData(VkCommandBuffer cmdBuffer) noexcept
{
	Camera *cam = CameraManager::GetActiveCamera();
	_dataBlock.inverseView = inverse(cam->GetView());
	_dataBlock.inverseProjection = inverse(cam->GetProjectionMatrix());
	_dataBlock.frameAndNoise = vec4(Engine::GetScreenWidth(), Engine::GetScreenHeight(), Engine::GetScreenWidth() / (_noiseSize / 4), Engine::GetScreenHeight() / (_noiseSize / 4));
	_dataBlock.zNear = cam->GetNear();
	_dataBlock.zFar = cam->GetFar();
	_dataBuffer->UpdateData((uint8_t *)&_dataBlock, 0, sizeof(SSAODataBlock), cmdBuffer);
}

int SSAO::BuildCommandBuffer()
{
	VkResult result{};

	if (_commandBuffer != VK_NULL_HANDLE)
		VKUtil::FreeCommandBuffer(_commandBuffer);

	if ((_commandBuffer = VKUtil::CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY)) == VK_NULL_HANDLE)
		return ENGINE_CMDBUFFER_CREATE_FAIL;

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	if ((result = vkBeginCommandBuffer(_commandBuffer, &beginInfo)) != VK_SUCCESS)
	{
		Logger::Log(SSAO_MODULE, LOG_CRITICAL, "vkBeginCommandBuffer call failed");
		return ENGINE_CMDBUFFER_BEGIN_FAIL;
	}

	VK_DBG_MARKER_BEGIN(_commandBuffer, "SSAO", vec4(0.56, 0.76, 0.83, 1.0));

	VkClearValue clearValues[2]{};
	clearValues[0].color = { { 0.f, 0.f, 0.f, 0.f } };
	clearValues[1].color = { { 0.f, 0.f, 0.f, 0.f } };

	VkRenderPassBeginInfo rpInfo{};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.renderPass = _renderPass;
	rpInfo.framebuffer = _framebuffer;
	rpInfo.renderArea.offset = { 0, 0 };
	rpInfo.renderArea.extent = { Engine::GetScreenWidth(), Engine::GetScreenHeight() };
	rpInfo.clearValueCount = 2;
	rpInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass(_commandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
	{
		// AO pass
		vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _aoPipeline);
		
		vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _aoPipelineLayout, 0, 1, &_aoDescriptorSet, 0, nullptr);
		vkCmdDraw(_commandBuffer, 3, 1, 0, 0);

		vkCmdNextSubpass(_commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

		// Blur pass
		vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _blurPipeline);

		vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _blurPipelineLayout, 0, 1, &_blurDescriptorSet, 0, nullptr);
		vkCmdDraw(_commandBuffer, 3, 1, 0, 0);
	}
	vkCmdEndRenderPass(_commandBuffer);
	
	VK_DBG_MARKER_END(_commandBuffer);

	if ((result = vkEndCommandBuffer(_commandBuffer)) != VK_SUCCESS)
	{
		Logger::Log(SSAO_MODULE, LOG_CRITICAL, "vkEndCommandBuffer call failed");
		return ENGINE_CMDBUFFER_RECORD_FAIL;
	}
	
	return ENGINE_OK;
}

VkCommandBuffer SSAO::GetCommandBuffer() noexcept { return _commandBuffer; }

void SSAO::ScreenResized() noexcept
{
	if (_commandBuffer != VK_NULL_HANDLE)
		VKUtil::FreeCommandBuffer(_commandBuffer);
	
	if (_framebuffer != VK_NULL_HANDLE)
		vkDestroyFramebuffer(VKUtil::GetDevice(), _framebuffer, VKUtil::GetAllocator());

	if (_renderPass != VK_NULL_HANDLE)
		vkDestroyRenderPass(VKUtil::GetDevice(), _renderPass, VKUtil::GetAllocator());

	if (_aoPipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(VKUtil::GetDevice(), _aoPipeline, VKUtil::GetAllocator());

	if (_blurPipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(VKUtil::GetDevice(), _blurPipeline, VKUtil::GetAllocator());

	if (!_CreateRenderPass())
	{ DIE("Failed to create render pass"); }

	_UpdateDescriptorSets();

	if (_CreatePipeline() != ENGINE_OK)
	{ DIE("Failed to create pipeline"); }

	if (!_CreateFramebuffers())
	{ DIE("Failed to create framebuffer"); }

	if (BuildCommandBuffer() != ENGINE_OK)
	{ DIE("Failed to create command buffer"); }
}

void SSAO::_CreateTextures()
{
	if((_aoTexture = new Texture(VK_FORMAT_R16_SFLOAT, VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_TILING_OPTIMAL, Engine::GetScreenWidth(), Engine::GetScreenHeight(), 1, true)) == nullptr)
	{ DIE("Out of resources"); }
	_aoTexture->CreateView(VK_IMAGE_ASPECT_COLOR_BIT);

	if((_blurTexture = new Texture(VK_FORMAT_R16_SFLOAT, VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_TILING_OPTIMAL, Engine::GetScreenWidth(), Engine::GetScreenHeight(), 1, true)) == nullptr)
	{ DIE("Out of resources"); }
	_blurTexture->CreateView(VK_IMAGE_ASPECT_COLOR_BIT);

	VKUtil::TransitionImageLayout(_aoTexture->GetImage(), VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
	VKUtil::TransitionImageLayout(_blurTexture->GetImage(), VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_DBG_SET_OBJECT_NAME((uint64_t)_aoTexture->GetImage(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "AO texture");
	VK_DBG_SET_OBJECT_NAME((uint64_t)_blurTexture->GetImage(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "Blurred AO texture");

	if((_noiseTexture = new Texture(_noiseSize / 4, _noiseSize / 4, 1, VK_FORMAT_R16G16_SFLOAT, sizeof(vec2) * _noiseSize, (uint8_t *)_noise)) == nullptr)
	{ DIE("Out of resources"); }
	VK_DBG_SET_OBJECT_NAME((uint64_t)_noiseTexture->GetImage(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "AO noise texture");

	SamplerParams noiseParams{};
	noiseParams.addressU = noiseParams.addressV = noiseParams.addressW = SamplerAddressMode::Repeat;
	noiseParams.magFilter = noiseParams.minFilter = SamplerFilter::Nearest;
	noiseParams.mipmapMode = SamplerMipmapMode::MipmapNearest;

	_noiseTexture->SetParameters(noiseParams);
}

int SSAO::_CreatePipeline()
{
	VkGraphicsPipelineCreateInfo pipelineInfo{};

	ShaderModule *vs = (ShaderModule *)ResourceManager::GetResourceByName("sh_fullscreen_vertex", ResourceType::RES_SHADERMODULE);
	ShaderModule *fs = (ShaderModule *)ResourceManager::GetResourceByName("sh_ssao", ResourceType::RES_SHADERMODULE);

	if (!vs || !fs)
	{
		Logger::Log(SSAO_MODULE, LOG_CRITICAL, "Failed to load SSAO shaders");
		return ENGINE_LOAD_SHADER_FAIL;
	}

	VkPipelineShaderStageCreateInfo fsVertShaderStageInfo{};
	VKUtil::InitShaderStage(&fsVertShaderStageInfo, VK_SHADER_STAGE_VERTEX_BIT, vs->GetHandle());

	VkPipelineShaderStageCreateInfo ssaoFragShaderStageInfo{};
	VKUtil::InitShaderStage(&ssaoFragShaderStageInfo, VK_SHADER_STAGE_FRAGMENT_BIT, fs->GetHandle());

	VkPipelineShaderStageCreateInfo ssaoShaderStages[]{ fsVertShaderStageInfo, ssaoFragShaderStageInfo };

	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = ssaoShaderStages;

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
	pipelineInfo.layout = _aoPipelineLayout;
	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &_aoPipeline) != VK_SUCCESS)
	{
		Logger::Log(SSAO_MODULE, LOG_CRITICAL, "Failed to create AO pipeline");
		return ENGINE_PIPELINE_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)_aoPipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "AO");

	ShaderModule *blur = (ShaderModule *)ResourceManager::GetResourceByName("sh_ssao_blur", ResourceType::RES_SHADERMODULE);
	
	if (!blur)
	{
		Logger::Log(SSAO_MODULE, LOG_CRITICAL, "Failed to load SSAO shaders");
		return ENGINE_LOAD_SHADER_FAIL;
	}

	VkPipelineShaderStageCreateInfo blurFragShaderStageInfo{};
	VKUtil::InitShaderStage(&blurFragShaderStageInfo, VK_SHADER_STAGE_FRAGMENT_BIT, blur->GetHandle());

	VkPipelineShaderStageCreateInfo blurShaderStages[]{ fsVertShaderStageInfo, blurFragShaderStageInfo };

	pipelineInfo.pStages = blurShaderStages;

	pipelineInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
	pipelineInfo.basePipelineHandle = _aoPipeline;
	pipelineInfo.layout = _blurPipelineLayout;
	pipelineInfo.subpass = 1;

	if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &_blurPipeline) != VK_SUCCESS)
	{
		Logger::Log(SSAO_MODULE, LOG_CRITICAL, "Failed to create AO blur pipeline");
		return ENGINE_PIPELINE_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)_blurPipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "AO blur");

	return ENGINE_OK;
}

int SSAO::_CreatePipelineLayout()
{
	VkPipelineLayoutCreateInfo layoutCI{};
	layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutCI.setLayoutCount = 1;
	layoutCI.pSetLayouts = &_aoDescriptorSetLayout;

	if (vkCreatePipelineLayout(VKUtil::GetDevice(), &layoutCI, VKUtil::GetAllocator(), &_aoPipelineLayout) != VK_SUCCESS)
	{
		Logger::Log(SSAO_MODULE, LOG_CRITICAL, "Failed to create AO pipeline layout");
		return ENGINE_PIPELINE_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)_aoPipelineLayout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "AO pipeline layout");

	layoutCI.pSetLayouts = &_blurDescriptorSetLayout;

	if (vkCreatePipelineLayout(VKUtil::GetDevice(), &layoutCI, VKUtil::GetAllocator(), &_blurPipelineLayout) != VK_SUCCESS)
	{
		Logger::Log(SSAO_MODULE, LOG_CRITICAL, "Failed to create AO blur pipeline layout");
		return ENGINE_PIPELINE_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)_blurPipelineLayout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "AO blur pipeline layout");

	return ENGINE_OK;
}

bool SSAO::_CreateRenderPass()
{
	VkAttachmentDescription aoAttachment{};
	aoAttachment.format = VK_FORMAT_R16_SFLOAT;
	aoAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	aoAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	aoAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	aoAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	aoAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	aoAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	aoAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference aoAttachmentRef{};
	aoAttachmentRef.attachment = 0;
	aoAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference blurAttachmentRef{};
	blurAttachmentRef.attachment = 0;
	blurAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDependency dependencies[3]{};
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = 1;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	dependencies[2].srcSubpass = 1;
	dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[2].srcAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkSubpassDescription subpasses[2]{};
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &aoAttachmentRef;
	subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[1].colorAttachmentCount = 1;
	subpasses[1].pColorAttachments = &blurAttachmentRef;

	VkAttachmentDescription attachments[] = { aoAttachment, aoAttachment };
	
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 2;
	renderPassInfo.pSubpasses = subpasses;
	renderPassInfo.dependencyCount = 3;
	renderPassInfo.pDependencies = dependencies;

	if (vkCreateRenderPass(VKUtil::GetDevice(), &renderPassInfo, VKUtil::GetAllocator(), &_renderPass) != VK_SUCCESS)
	{
		Logger::Log(SSAO_MODULE, LOG_CRITICAL, "Failed to create render pass");
		return false;
	}

	return true;
}

int SSAO::_CreateDescriptorSets()
{
	VkDescriptorSetLayoutBinding sceneDataBinding{};
	sceneDataBinding.binding = 0;
	sceneDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	sceneDataBinding.descriptorCount = 1;
	sceneDataBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	sceneDataBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding ssaoDataBinding{};
	ssaoDataBinding.binding = 1;
	ssaoDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ssaoDataBinding.descriptorCount = 1;
	ssaoDataBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	ssaoDataBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding depthBinding{};
	depthBinding.binding = 2;
	depthBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	depthBinding.descriptorCount = 1;
	depthBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	depthBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding normalBinding{};
	normalBinding.binding = 3;
	normalBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	normalBinding.descriptorCount = 1;
	normalBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	normalBinding.pImmutableSamplers = nullptr;
	
	VkDescriptorSetLayoutBinding noiseTexture{};
	noiseTexture.binding = 4;
	noiseTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	noiseTexture.descriptorCount = 1;
	noiseTexture.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	noiseTexture.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding aoBindings[5]{ sceneDataBinding, ssaoDataBinding, depthBinding, normalBinding, noiseTexture };

	VkDescriptorSetLayoutCreateInfo dsCI{};
	dsCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dsCI.bindingCount = 5;
	dsCI.pBindings = aoBindings;

	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &dsCI, VKUtil::GetAllocator(), &_aoDescriptorSetLayout) != VK_SUCCESS)
	{
		Logger::Log(SSAO_MODULE, LOG_CRITICAL, "Failed to create AO descriptor set layout");
		return false;
	}

	VkDescriptorSetLayoutBinding aoTextureBinding{};
	aoTextureBinding.binding = 0;
	aoTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	aoTextureBinding.descriptorCount = 1;
	aoTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	aoTextureBinding.pImmutableSamplers = nullptr;

	dsCI.bindingCount = 1;
	dsCI.pBindings = &aoTextureBinding;

	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &dsCI, VKUtil::GetAllocator(), &_blurDescriptorSetLayout) != VK_SUCCESS)
	{
		Logger::Log(SSAO_MODULE, LOG_CRITICAL, "Failed to create AO descriptor set layout");
		return false;
	}

	VkDescriptorPoolSize poolSizes[2]{};
	poolSizes[0].descriptorCount = 2;
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = 4;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	VkDescriptorPoolCreateInfo poolCI{};
	poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCI.poolSizeCount = 2;
	poolCI.pPoolSizes = poolSizes;
	poolCI.maxSets = 2;

	if (vkCreateDescriptorPool(VKUtil::GetDevice(), &poolCI, VKUtil::GetAllocator(), &_pool) != VK_SUCCESS)
	{
		Logger::Log(SSAO_MODULE, LOG_CRITICAL, "Failed to create descriptor pool");
		return ENGINE_DESCRIPTOR_POOL_CREATE_FAIL;
	}

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &_aoDescriptorSetLayout;

	if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, &_aoDescriptorSet) != VK_SUCCESS)
	{
		Logger::Log(SSAO_MODULE, LOG_CRITICAL, "Failed to allocate descriptor AO set");
		return ENGINE_DESCRIPTOR_SET_CREATE_FAIL;
	}

	allocInfo.pSetLayouts = &_blurDescriptorSetLayout;

	if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, &_blurDescriptorSet) != VK_SUCCESS)
	{
		Logger::Log(SSAO_MODULE, LOG_CRITICAL, "Failed to allocate descriptor AO blur set");
		return ENGINE_DESCRIPTOR_SET_CREATE_FAIL;
	}

	_UpdateDescriptorSets();

	return ENGINE_OK;
}

void SSAO::_UpdateDescriptorSets()
{
	VkDescriptorBufferInfo sceneDataInfo{};
	sceneDataInfo.buffer = Renderer::GetInstance()->GetSceneDataBuffer()->GetHandle();
	sceneDataInfo.offset = Renderer::GetInstance()->GetSceneDataBuffer()->GetParentOffset();
	sceneDataInfo.range = sizeof(mat4) * 2;

	VkDescriptorBufferInfo ssaoDataInfo{};
	ssaoDataInfo.buffer = _dataBuffer->GetHandle();
	ssaoDataInfo.offset = _dataBuffer->GetParentOffset();
	ssaoDataInfo.range = sizeof(SSAODataBlock);

	VkDescriptorImageInfo depthInfo{};
	depthInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	depthInfo.imageView = Renderer::GetInstance()->GetDepthImageView();
	depthInfo.sampler = Renderer::GetInstance()->GetDepthSampler();

	VkDescriptorImageInfo normalInfo{};
	normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	normalInfo.imageView = Engine::GetConfiguration().Renderer.Multisampling ? Renderer::GetInstance()->GetMSAANormalImageView() : Renderer::GetInstance()->GetNormalImageView();
	normalInfo.sampler = Renderer::GetInstance()->GetNearestSampler();

	VkDescriptorImageInfo noiseInfo{};
	noiseInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	noiseInfo.imageView = _noiseTexture->GetImageView();
	noiseInfo.sampler = _noiseTexture->GetSampler();

	VkDescriptorImageInfo aoTextureInfo{};
	aoTextureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	aoTextureInfo.imageView = _aoTexture->GetImageView();
	aoTextureInfo.sampler = Renderer::GetInstance()->GetNearestSampler();

	VkWriteDescriptorSet writeDS[6]{};
	VKUtil::WriteDS(&writeDS[0], 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &sceneDataInfo, _aoDescriptorSet, 0);
	VKUtil::WriteDS(&writeDS[1], 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &ssaoDataInfo, _aoDescriptorSet, 1);
	VKUtil::WriteDS(&writeDS[2], 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &depthInfo, _aoDescriptorSet, 2);
	VKUtil::WriteDS(&writeDS[3], 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &normalInfo, _aoDescriptorSet, 3);
	VKUtil::WriteDS(&writeDS[4], 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &noiseInfo, _aoDescriptorSet, 4);
	VKUtil::WriteDS(&writeDS[5], 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &aoTextureInfo, _blurDescriptorSet, 0);

	vkUpdateDescriptorSets(VKUtil::GetDevice(), 6, writeDS, 0, nullptr);
}

bool SSAO::_CreateFramebuffers()
{
	VkImageView attachments[]{ _aoTexture->GetImageView(), _blurTexture->GetImageView() };

	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = _renderPass;
	createInfo.attachmentCount = 2;
	createInfo.pAttachments = attachments;
	createInfo.width = Engine::GetScreenWidth();
	createInfo.height = Engine::GetScreenHeight();
	createInfo.layers = 1;

	if (vkCreateFramebuffer(VKUtil::GetDevice(), &createInfo, VKUtil::GetAllocator(), &_framebuffer) != VK_SUCCESS)
	{
		Logger::Log(SSAO_MODULE, LOG_CRITICAL, "Failed to create framebuffer");
		return false;
	}

	return true;
}

void SSAO::Release()
{
	delete _dataBuffer;
	delete _aoTexture;
	delete _blurTexture;
	delete _noiseTexture;

	if (_commandBuffer != VK_NULL_HANDLE)
		VKUtil::FreeCommandBuffer(_commandBuffer);

	if (_framebuffer != VK_NULL_HANDLE)
		vkDestroyFramebuffer(VKUtil::GetDevice(), _framebuffer, VKUtil::GetAllocator());

	if (_renderPass != VK_NULL_HANDLE)
		vkDestroyRenderPass(VKUtil::GetDevice(), _renderPass, VKUtil::GetAllocator());

	if (_pool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(VKUtil::GetDevice(), _pool, VKUtil::GetAllocator());

	if (_aoDescriptorSetLayout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(VKUtil::GetDevice(), _aoDescriptorSetLayout, VKUtil::GetAllocator());

	if (_blurDescriptorSetLayout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(VKUtil::GetDevice(), _blurDescriptorSetLayout, VKUtil::GetAllocator());

	if (_aoPipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(VKUtil::GetDevice(), _aoPipeline, VKUtil::GetAllocator());

	if (_blurPipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(VKUtil::GetDevice(), _blurPipeline, VKUtil::GetAllocator());

	if (_aoPipelineLayout != VK_NULL_HANDLE)
		vkDestroyPipelineLayout(VKUtil::GetDevice(), _aoPipelineLayout, VKUtil::GetAllocator());

	if (_blurPipelineLayout != VK_NULL_HANDLE)
		vkDestroyPipelineLayout(VKUtil::GetDevice(), _blurPipelineLayout, VKUtil::GetAllocator());

	Logger::Log(SSAO_MODULE, LOG_INFORMATION, "Released");
}
