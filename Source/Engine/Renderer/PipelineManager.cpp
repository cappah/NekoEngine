/* NekoEngine
 *
 * PipelineManager.cpp
 * Author: Alexandru Naiman
 *
 * PipelineManager
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

#include <Engine/Vertex.h>
#include <Engine/Engine.h>
#include <Engine/ResourceManager.h>
#include <Renderer/GUI.h>
#include <Renderer/Debug.h>
#include <Renderer/VKUtil.h>
#include <Renderer/Material.h>
#include <Renderer/PipelineManager.h>
#include <Renderer/RenderPassManager.h>

#include <array>

#define PLMGR_MODULE				"PipelineManager"

#define SH_VTX						0
#define SH_VTX_NM					1

#define SH_VTX_STATIC				"main"
#define SH_VTX_ANIM					"anim_main"

#define SH_FRAG_UNLIT				0
#define SH_FRAG_PHONG				1
#define SH_FRAG_PHONG_SPEC			2
#define SH_FRAG_PHONG_SPEC_EM		3
#define SH_FRAG_PHONG_NM			4
#define SH_FRAG_PHONG_SPEC_NM		5
#define SH_FRAG_PHONG_SPEC_EM_NM	6

using namespace std;

enum ShaderId : uint8_t
{
	SH_Vertex,
	SH_Vertex_Anim,
	SH_Vertex_Depth,
	SH_Vertex_Depth_Anim,
	SH_Vertex_Skysphere,
	SH_Vertex_GUI,
	SH_Vertex_Terrain,
	SH_Vertex_Terrain_Depth,
	SH_Fragment_Phong,
	SH_Fragment_Depth,
	SH_Fragment_GUI,
	SH_Fragment_Skysphere,
	SH_Fragment_Postprocess,
	SH_Compute_Culling
};

unordered_map<uint8_t, VkPipeline> PipelineManager::_pipelines;
unordered_map<uint8_t, VkPipelineLayout> PipelineManager::_pipelineLayouts;
unordered_map<uint8_t, VkDescriptorSetLayout> PipelineManager::_descriptorSetLayouts;
static unordered_map<uint8_t, ShaderModule *> _shaderModules;
static unordered_map<uint8_t, VkPipeline> _shaderInfo;

static const int _guiFragShader = 0;
static const int _fontFragShader = 1;

int PipelineManager::Initialize()
{
	int ret = ENGINE_FAIL;

	if ((ret = _LoadShaders()) != ENGINE_OK)
		return ret;

	if ((ret = _CreateDescriptorSetLayouts()) != ENGINE_OK)
		return ret;
	if ((ret = _CreatePipelineLayouts()) != ENGINE_OK)
		return ret;

	if ((ret = _CreatePipelines()) != ENGINE_OK)
		return ret;

	if ((ret = _LoadComputeShaders()) != ENGINE_OK)
		return ret;

	if ((ret = _CreateComputeDescriptorSetLayouts()) != ENGINE_OK)
		return ret;

	if ((ret = _CreateComputePipelineLayouts()) != ENGINE_OK)
		return ret;

	if ((ret = _CreateComputePipelines()) != ENGINE_OK)
		return ret;

	return ENGINE_OK;
}

int PipelineManager::_LoadShaders()
{
	ShaderModule *module = nullptr;

	// Vertex shaders

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_vertex", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Vertex, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_anim_vertex", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Vertex_Anim, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_depth_vertex", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Vertex_Depth, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_depth_anim_vertex", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Vertex_Depth_Anim, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_skysphere_vertex", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Vertex_Skysphere, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_gui_vertex", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Vertex_GUI, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_terrain_vertex", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Vertex_Terrain, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_depth_terrain_vertex", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Vertex_Terrain_Depth, module));

	// Fragment shaders

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_phong", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Fragment_Phong, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_depth_fragment", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Fragment_Depth, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_skysphere_fragment", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Fragment_Skysphere, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_gui_fragment", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Fragment_GUI, module));

	return ENGINE_OK;
}

int PipelineManager::_CreatePipelines()
{
	VkPipeline pipeline = VK_NULL_HANDLE;

	VkSpecializationMapEntry int0MapEntry{};
	int0MapEntry.constantID = 0;
	int0MapEntry.offset = 0;
	int0MapEntry.size = sizeof(int32_t);

	struct ShaderSpecData
	{
		struct
		{
			int32_t type;
		} vtx;
		struct
		{
			int32_t type;
			int32_t numTextures;
			int32_t enableAO;
		} frag;
	} shaderSpecData;

	shaderSpecData.vtx.type = 0;
	shaderSpecData.frag.type = 0;
	shaderSpecData.frag.numTextures = 1;
	shaderSpecData.frag.enableAO = Engine::GetConfiguration().Renderer.SSAO.Enable;

	VkSpecializationMapEntry vtxShaderSpecMap[1];
	vtxShaderSpecMap[0].constantID = 0;
	vtxShaderSpecMap[0].offset = 0;
	vtxShaderSpecMap[0].size = sizeof(int32_t);

	VkSpecializationMapEntry fragShaderSpecMap[2];
	fragShaderSpecMap[0].constantID = 10;
	fragShaderSpecMap[0].offset = 0;
	fragShaderSpecMap[0].size = sizeof(int32_t);
	fragShaderSpecMap[1].constantID = 11;
	fragShaderSpecMap[1].offset = sizeof(int32_t);
	fragShaderSpecMap[1].size = sizeof(int32_t);
	fragShaderSpecMap[1].constantID = 12;
	fragShaderSpecMap[1].offset = sizeof(int32_t) * 2;
	fragShaderSpecMap[1].size = sizeof(int32_t);

	//****************
	//* Shader stages
	//****************

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	VkPipelineShaderStageCreateInfo animVertShaderStageInfo{};
	VkPipelineShaderStageCreateInfo terrainVertShaderStageInfo{};
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	VkPipelineShaderStageCreateInfo skyVertShaderStageInfo{};
	VkPipelineShaderStageCreateInfo skyFragShaderStageInfo{};
	VkPipelineShaderStageCreateInfo depthVertShaderStageInfo{};
	VkPipelineShaderStageCreateInfo depthAnimVertShaderStageInfo{};
	VkPipelineShaderStageCreateInfo depthTerrainVertShaderStageInfo{};
	VkPipelineShaderStageCreateInfo depthFragShaderStageInfo{};
	VkPipelineShaderStageCreateInfo guiVertShaderStageInfo{};
	VkPipelineShaderStageCreateInfo guiFragShaderStageInfo{};
	VkPipelineShaderStageCreateInfo fontFragShaderStageInfo{};
	VkSpecializationInfo vtxShaderSpecInfo{};
	VkSpecializationInfo fragShaderSpecInfo{};
	VkSpecializationInfo guiSpecInfo{};
	VkSpecializationInfo fontSpecInfo{};
	{
		// Specialization info
		vtxShaderSpecInfo.mapEntryCount = sizeof(vtxShaderSpecMap) / sizeof(VkSpecializationMapEntry);
		vtxShaderSpecInfo.pMapEntries = vtxShaderSpecMap;
		vtxShaderSpecInfo.dataSize = sizeof(shaderSpecData.vtx);
		vtxShaderSpecInfo.pData = &shaderSpecData.vtx;

		fragShaderSpecInfo.mapEntryCount = sizeof(fragShaderSpecMap) / sizeof(VkSpecializationMapEntry);
		fragShaderSpecInfo.pMapEntries = fragShaderSpecMap;
		fragShaderSpecInfo.dataSize = sizeof(shaderSpecData.frag);
		fragShaderSpecInfo.pData = &shaderSpecData.frag;

		guiSpecInfo.mapEntryCount = 1;
		guiSpecInfo.pMapEntries = &int0MapEntry;
		guiSpecInfo.dataSize = sizeof(_guiFragShader);
		guiSpecInfo.pData = &_guiFragShader;

		fontSpecInfo.mapEntryCount = 1;
		fontSpecInfo.pMapEntries = &int0MapEntry;
		fontSpecInfo.dataSize = sizeof(_fontFragShader);
		fontSpecInfo.pData = &_fontFragShader;

		// Stage 
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = _shaderModules[SH_Vertex]->GetHandle();
		vertShaderStageInfo.pSpecializationInfo = &vtxShaderSpecInfo;
		vertShaderStageInfo.pName = "main";

		animVertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		animVertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		animVertShaderStageInfo.module = _shaderModules[SH_Vertex_Anim]->GetHandle();
		animVertShaderStageInfo.pSpecializationInfo = &vtxShaderSpecInfo;
		animVertShaderStageInfo.pName = "main";

		terrainVertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		terrainVertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		terrainVertShaderStageInfo.module = _shaderModules[SH_Vertex_Terrain]->GetHandle();
		terrainVertShaderStageInfo.pSpecializationInfo = &vtxShaderSpecInfo;
		terrainVertShaderStageInfo.pName = "main";

		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = _shaderModules[SH_Fragment_Phong]->GetHandle();
		fragShaderStageInfo.pSpecializationInfo = &fragShaderSpecInfo;
		fragShaderStageInfo.pName = "main";

		skyVertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		skyVertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		skyVertShaderStageInfo.module = _shaderModules[SH_Vertex_Skysphere]->GetHandle();
		skyVertShaderStageInfo.pName = "main";

		skyFragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		skyFragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		skyFragShaderStageInfo.module = _shaderModules[SH_Fragment_Skysphere]->GetHandle();
		skyFragShaderStageInfo.pName = "main";

		depthVertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		depthVertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		depthVertShaderStageInfo.module = _shaderModules[SH_Vertex_Depth]->GetHandle();
		depthVertShaderStageInfo.pName = "main";

		depthAnimVertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		depthAnimVertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		depthAnimVertShaderStageInfo.module = _shaderModules[SH_Vertex_Depth_Anim]->GetHandle();
		depthAnimVertShaderStageInfo.pName = "main";

		depthTerrainVertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		depthTerrainVertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		depthTerrainVertShaderStageInfo.module = _shaderModules[SH_Vertex_Terrain_Depth]->GetHandle();
		depthTerrainVertShaderStageInfo.pName = "main";

		depthFragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		depthFragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		depthFragShaderStageInfo.module = _shaderModules[SH_Fragment_Depth]->GetHandle();
		depthFragShaderStageInfo.pName = "main";

		guiVertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		guiVertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		guiVertShaderStageInfo.module = _shaderModules[SH_Vertex_GUI]->GetHandle();
		guiVertShaderStageInfo.pName = "main";

		guiFragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		guiFragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		guiFragShaderStageInfo.module = _shaderModules[SH_Fragment_GUI]->GetHandle();
		guiFragShaderStageInfo.pName = "main";
		guiFragShaderStageInfo.pSpecializationInfo = &guiSpecInfo;

		fontFragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fontFragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fontFragShaderStageInfo.module = _shaderModules[SH_Fragment_GUI]->GetHandle();
		fontFragShaderStageInfo.pName = "main";
		fontFragShaderStageInfo.pSpecializationInfo = &fontSpecInfo;
	}

	//********************
	//* Shared structures
	//********************

	VkVertexInputBindingDescription bindingDesc = Vertex::GetBindingDescription();
	NArray<VkVertexInputAttributeDescription> attribDesc = Vertex::GetAttributeDescriptions();
	VkVertexInputBindingDescription animBindingDesc = SkeletalVertex::GetBindingDescription();
	NArray<VkVertexInputAttributeDescription> animAttribDesc = SkeletalVertex::GetAttributeDescriptions();
	VkVertexInputBindingDescription terrainBindingDesc = TerrainVertex::GetBindingDescription();
	NArray<VkVertexInputAttributeDescription> terrainAttribDesc = TerrainVertex::GetAttributeDescriptions();
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	VkPipelineVertexInputStateCreateInfo animVertexInputInfo{};
	VkPipelineVertexInputStateCreateInfo terrainVertexInputInfo{};
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	VkViewport viewport{};
	VkRect2D scissor{};
	VkPipelineViewportStateCreateInfo viewportState{};
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	VkPipelineMultisampleStateCreateInfo multisampling{};
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	VkPipelineColorBlendAttachmentState colorBlendAttachments[2]{};
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	VkPipelineColorBlendAttachmentState transparentColorBlendAttachments[2]{};
	VkPipelineColorBlendStateCreateInfo transparentColorBlending{};
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	{
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attribDesc.Count();
		vertexInputInfo.pVertexAttributeDescriptions = *attribDesc;

		animVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		animVertexInputInfo.vertexBindingDescriptionCount = 1;
		animVertexInputInfo.pVertexBindingDescriptions = &animBindingDesc;
		animVertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)animAttribDesc.Count();
		animVertexInputInfo.pVertexAttributeDescriptions = *animAttribDesc;

		terrainVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		terrainVertexInputInfo.vertexBindingDescriptionCount = 1;
		terrainVertexInputInfo.pVertexBindingDescriptions = &terrainBindingDesc;
		terrainVertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)terrainAttribDesc.Count();
		terrainVertexInputInfo.pVertexAttributeDescriptions = *terrainAttribDesc;

		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		viewport.x = 0.f;
		viewport.y = 0.f;
		viewport.width = (float)Engine::GetScreenWidth();
		viewport.height = (float)Engine::GetScreenHeight();
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		scissor.offset = { 0, 0 };
		scissor.extent.width = Engine::GetScreenWidth();
		scissor.extent.height = Engine::GetScreenHeight();
		
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_TRUE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.f;
		rasterizer.depthBiasClamp = 0.f;
		rasterizer.depthBiasSlopeFactor = 0.f;

		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		if (Engine::GetConfiguration().Renderer.Multisampling)
		{
			switch (Engine::GetConfiguration().Renderer.Samples)
			{
				case 2: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT; break;
				case 4: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT; break;
				case 8: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT; break;
				case 16: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_16_BIT; break;
				case 32: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_32_BIT; break;
				case 64: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_64_BIT; break;
			}
		}

		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f;
		depthStencil.maxDepthBounds = 1.0f;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {};
		depthStencil.back = {};
		
		colorBlendAttachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachments[0].blendEnable = VK_FALSE;
		colorBlendAttachments[1].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachments[1].blendEnable = VK_FALSE;
		
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 2;
		colorBlending.pAttachments = colorBlendAttachments;

		transparentColorBlendAttachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		transparentColorBlendAttachments[0].blendEnable = VK_TRUE;
		transparentColorBlendAttachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		transparentColorBlendAttachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		transparentColorBlendAttachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		transparentColorBlendAttachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		transparentColorBlendAttachments[0].colorBlendOp = VK_BLEND_OP_ADD;
		transparentColorBlendAttachments[0].alphaBlendOp = VK_BLEND_OP_ADD;
		transparentColorBlendAttachments[1].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		transparentColorBlendAttachments[1].blendEnable = VK_FALSE;

		transparentColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		transparentColorBlending.logicOpEnable = VK_FALSE;
		transparentColorBlending.attachmentCount = 2;
		transparentColorBlending.pAttachments = transparentColorBlendAttachments;
		
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_OneSampler];
		pipelineInfo.renderPass = RenderPassManager::GetRenderPass(RP_Graphics);
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;
	}

	//*******************
	//* Static pipelines
	//*******************

	// Opaque
	{
		// Unlit
		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_UNLIT;
		shaderSpecData.frag.numTextures = 1;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create unlit pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "unlit");
		_pipelines.insert(make_pair(PIPE_Unlit, pipeline));

		// All the other pipelines are derivatives
		pipelineInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
		pipelineInfo.basePipelineHandle = pipeline;

		// Phong
		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_PHONG;
		shaderSpecData.frag.numTextures = 1;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create phong pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "phong");
		_pipelines.insert(make_pair(PIPE_Phong, pipeline));

		// Phong with specular
		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC;
		shaderSpecData.frag.numTextures = 2;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_TwoSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "phong specular");
		_pipelines.insert(make_pair(PIPE_PhongSpecular, pipeline));

		// Phong with specular and emission
		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_EM;
		shaderSpecData.frag.numTextures = 3;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_ThreeSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "phong specular emissive");
		_pipelines.insert(make_pair(PIPE_PhongSpecularEmissive, pipeline));

		// Phong with normal map
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_NM;
		shaderSpecData.frag.numTextures = 2;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_TwoSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "phong normal");
		_pipelines.insert(make_pair(PIPE_PhongNormal, pipeline));

		// Phong with specular and normal
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_NM;
		shaderSpecData.frag.numTextures = 3;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_ThreeSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "phong normal specular");
		_pipelines.insert(make_pair(PIPE_PhongNormalSpecular, pipeline));

		// Phong with specular, normal and emission
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_EM_NM;
		shaderSpecData.frag.numTextures = 4;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_FourSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "phong normal specular emissive");
		_pipelines.insert(make_pair(PIPE_PhongNormalSpecularEmissive, pipeline));
	}

	// Transparent
	{
		pipelineInfo.pColorBlendState = &transparentColorBlending;

		// Unlit
		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_UNLIT;
		shaderSpecData.frag.numTextures = 1;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create unlit pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent unlit");
		_pipelines.insert(make_pair(PIPE_Transparent_Unlit, pipeline));

		// Phong
		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_PHONG;
		shaderSpecData.frag.numTextures = 1;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create phong pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent phong");
		_pipelines.insert(make_pair(PIPE_Transparent_Phong, pipeline));

		// Phong with specular
		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC;
		shaderSpecData.frag.numTextures = 2;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_TwoSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent phong specular");
		_pipelines.insert(make_pair(PIPE_Transparent_PhongSpecular, pipeline));

		// Phong with specular and emission
		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_EM;
		shaderSpecData.frag.numTextures = 3;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_ThreeSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent phong specular emissive");
		_pipelines.insert(make_pair(PIPE_Transparent_PhongSpecularEmissive, pipeline));

		// Phong with normal map
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_NM;
		shaderSpecData.frag.numTextures = 2;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_TwoSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent phong normal");
		_pipelines.insert(make_pair(PIPE_Transparent_PhongNormal, pipeline));

		// Phong with specular and normal
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_NM;
		shaderSpecData.frag.numTextures = 3;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_ThreeSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent phong normal specular");
		_pipelines.insert(make_pair(PIPE_Transparent_PhongNormalSpecular, pipeline));

		// Phong with specular, normal and emission
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_EM_NM;
		shaderSpecData.frag.numTextures = 4;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_FourSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent phong normal specular emissive");
		_pipelines.insert(make_pair(PIPE_Transparent_PhongNormalSpecularEmissive, pipeline));
	}

	//*********************
	//* Animated pipelines
	//*********************

	// Opaque
	{
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pVertexInputState = &animVertexInputInfo;

		// Unlit
		VkPipelineShaderStageCreateInfo shaderStages[] = { animVertShaderStageInfo, fragShaderStageInfo };
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_UNLIT;
		shaderSpecData.frag.numTextures = 1;

		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_OneSampler];
		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated unlit pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "animated unlit");
		_pipelines.insert(make_pair(PIPE_Anim_Unlit, pipeline));

		// Phong
		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_PHONG;
		shaderSpecData.frag.numTextures = 1;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated phong pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "animated phong");
		_pipelines.insert(make_pair(PIPE_Anim_Phong, pipeline));

		// Phong with specular
		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC;
		shaderSpecData.frag.numTextures = 2;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_TwoSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated phong specular pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "animated phong specular");
		_pipelines.insert(make_pair(PIPE_Anim_PhongSpecular, pipeline));

		// Phong with specular and emission
		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_EM;
		shaderSpecData.frag.numTextures = 3;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_ThreeSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated phong specular emissive pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "animated phong specular emissive");
		_pipelines.insert(make_pair(PIPE_Anim_PhongSpecularEmissive, pipeline));

		// Phong with normal map
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_NM;
		shaderSpecData.frag.numTextures = 2;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_TwoSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "animated phong normal");
		_pipelines.insert(make_pair(PIPE_Anim_PhongNormal, pipeline));

		// Phong with specular and normal
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_NM;
		shaderSpecData.frag.numTextures = 3;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_ThreeSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "animated phong normal specular");
		_pipelines.insert(make_pair(PIPE_Anim_PhongNormalSpecular, pipeline));

		// Phong with specular, normal and emission
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_EM_NM;
		shaderSpecData.frag.numTextures = 4;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_FourSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "animated phong normal specular emissive");
		_pipelines.insert(make_pair(PIPE_Anim_PhongNormalSpecularEmissive, pipeline));
	}

	// Transparent
	{
		pipelineInfo.pColorBlendState = &transparentColorBlending;

		// Unlit
		VkPipelineShaderStageCreateInfo shaderStages[] = { animVertShaderStageInfo, fragShaderStageInfo };
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_UNLIT;
		shaderSpecData.frag.numTextures = 1;

		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_OneSampler];
		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated unlit pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent animated unlit");
		_pipelines.insert(make_pair(PIPE_Transparent_Anim_Unlit, pipeline));

		// Phong
		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_PHONG;
		shaderSpecData.frag.numTextures = 1;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated phong pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent animated phong");
		_pipelines.insert(make_pair(PIPE_Transparent_Anim_Phong, pipeline));

		// Phong with specular
		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC;
		shaderSpecData.frag.numTextures = 2;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_TwoSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated phong specular pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent animated phong specular");
		_pipelines.insert(make_pair(PIPE_Transparent_Anim_PhongSpecular, pipeline));

		// Phong with specular and emission
		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_EM;
		shaderSpecData.frag.numTextures = 3;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_ThreeSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated phong specular emissive pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent animated phong specular emissive");
		_pipelines.insert(make_pair(PIPE_Transparent_Anim_PhongSpecularEmissive, pipeline));

		// Phong with normal map
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_NM;
		shaderSpecData.frag.numTextures = 2;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_TwoSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent animated phong normal");
		_pipelines.insert(make_pair(PIPE_Transparent_Anim_PhongNormal, pipeline));

		// Phong with specular and normal
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_NM;
		shaderSpecData.frag.numTextures = 3;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_ThreeSamplers];
	
		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent animated phong normal specular");
		_pipelines.insert(make_pair(PIPE_Transparent_Anim_PhongNormalSpecular, pipeline));

		// Phong with specular, normal and emission
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_EM_NM;
		shaderSpecData.frag.numTextures = 4;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_FourSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent animated phong normal specular emissive");
		_pipelines.insert(make_pair(PIPE_Transparent_Anim_PhongNormalSpecularEmissive, pipeline));
	}

	//*********************
	//* Skysphere pipeline
	//*********************

	pipelineInfo.pColorBlendState = &colorBlending;

	{
		pipelineInfo.pVertexInputState = &vertexInputInfo;

		VkPipelineShaderStageCreateInfo skyShaderStages[] = { skyVertShaderStageInfo, skyFragShaderStageInfo };
		pipelineInfo.pStages = skyShaderStages;

		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_OneSampler];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "skysphere");
		_pipelines.insert(make_pair(PIPE_Skysphere, pipeline));
	}

	//*******************
	//* Terrain pipeline
	//*******************

	{
		VkPipelineShaderStageCreateInfo shaderStages[] = { terrainVertShaderStageInfo, fragShaderStageInfo };
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		pipelineInfo.pVertexInputState = &terrainVertexInputInfo;

		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_PHONG;
		shaderSpecData.frag.numTextures = 1;

		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_OneSampler];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create terrain pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "terrain");
		_pipelines.insert(make_pair(PIPE_Terrain, pipeline));
	}

	//***************************
	//* Depth & normal pipelines
	//***************************

	{
		pipelineInfo.pVertexInputState = &vertexInputInfo;

		VkPipelineColorBlendStateCreateInfo depthPrepass_colorBlending = {};
		depthPrepass_colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		depthPrepass_colorBlending.logicOpEnable = VK_FALSE;
		depthPrepass_colorBlending.attachmentCount = 1;
		depthPrepass_colorBlending.pAttachments = &colorBlendAttachments[0];

		VkPipelineDepthStencilStateCreateInfo depthPrepass_DepthStencil = {};
		depthPrepass_DepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthPrepass_DepthStencil.depthTestEnable = VK_TRUE;
		depthPrepass_DepthStencil.depthWriteEnable = VK_TRUE;
		depthPrepass_DepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthPrepass_DepthStencil.depthBoundsTestEnable = VK_FALSE;
		depthPrepass_DepthStencil.stencilTestEnable = VK_FALSE;

		pipelineInfo.pDepthStencilState = &depthPrepass_DepthStencil;
		pipelineInfo.pColorBlendState = &depthPrepass_colorBlending;
		pipelineInfo.renderPass = RenderPassManager::GetRenderPass(RP_Depth);

		VkPipelineShaderStageCreateInfo depthShaderStages[] = { depthVertShaderStageInfo, depthFragShaderStageInfo };
		pipelineInfo.pStages = depthShaderStages;

		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Depth];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "depth");
		_pipelines.insert(make_pair(PIPE_Depth, pipeline));

		VkPipelineShaderStageCreateInfo depthAnimShaderStages[] = { depthAnimVertShaderStageInfo, depthFragShaderStageInfo };
		pipelineInfo.pStages = depthAnimShaderStages;

		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_Depth];
		pipelineInfo.pVertexInputState = &animVertexInputInfo;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "animated depth");
		_pipelines.insert(make_pair(PIPE_Anim_Depth, pipeline));

		VkPipelineShaderStageCreateInfo depthTerrainShaderStages[] = { depthTerrainVertShaderStageInfo, depthFragShaderStageInfo };
		pipelineInfo.pStages = depthTerrainShaderStages;

		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Depth];
		pipelineInfo.pVertexInputState = &terrainVertexInputInfo;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "terrain depth");
		_pipelines.insert(make_pair(PIPE_Terrain_Depth, pipeline));
	}
	
	//***************
	//* GUI pipeline
	//***************

	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	{
		VkPipelineShaderStageCreateInfo guiShaderStages[] = { guiVertShaderStageInfo, guiFragShaderStageInfo };

		VkVertexInputBindingDescription guiBindingDesc = GUIVertex::GetBindingDescription();
		NArray<VkVertexInputAttributeDescription> guiAttribDesc = GUIVertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo guiVertexInputInfo{};
		guiVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		guiVertexInputInfo.vertexBindingDescriptionCount = 1;
		guiVertexInputInfo.pVertexBindingDescriptions = &guiBindingDesc;
		guiVertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)guiAttribDesc.Count();
		guiVertexInputInfo.pVertexAttributeDescriptions = *guiAttribDesc;

		VkPipelineDepthStencilStateCreateInfo guiDepthStencil{};
		guiDepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		guiDepthStencil.depthTestEnable = VK_FALSE;
		guiDepthStencil.depthWriteEnable = VK_FALSE;
		guiDepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		guiDepthStencil.depthBoundsTestEnable = VK_FALSE;
		guiDepthStencil.minDepthBounds = 0.0f; // Optional
		guiDepthStencil.maxDepthBounds = 1.0f; // Optional
		guiDepthStencil.stencilTestEnable = VK_FALSE;
		guiDepthStencil.front = {}; // Optional
		guiDepthStencil.back = {}; // Optional

		VkPipelineColorBlendAttachmentState guiColorBlendAttachment{};
		guiColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		guiColorBlendAttachment.blendEnable = VK_TRUE;
		guiColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		guiColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		guiColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		guiColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		guiColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		guiColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

		VkPipelineColorBlendStateCreateInfo guiColorBlending{};
		guiColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		guiColorBlending.logicOpEnable = VK_FALSE;
		guiColorBlending.attachmentCount = 1;
		guiColorBlending.pAttachments = &guiColorBlendAttachment;

		VkPipelineRasterizationStateCreateInfo guiRasterizer{};
		guiRasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		guiRasterizer.depthClampEnable = VK_TRUE;
		guiRasterizer.rasterizerDiscardEnable = VK_FALSE;
		guiRasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		guiRasterizer.lineWidth = 1.f;
		guiRasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		guiRasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		guiRasterizer.depthBiasEnable = VK_FALSE;
		guiRasterizer.depthBiasConstantFactor = 0.f;
		guiRasterizer.depthBiasClamp = 0.f;
		guiRasterizer.depthBiasSlopeFactor = 0.f;

		pipelineInfo.pRasterizationState = &guiRasterizer;
		pipelineInfo.pVertexInputState = &guiVertexInputInfo;
		pipelineInfo.pColorBlendState = &guiColorBlending;
		pipelineInfo.pDepthStencilState = &guiDepthStencil;
		pipelineInfo.renderPass = RenderPassManager::GetRenderPass(RP_GUI);
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_GUI];
		pipelineInfo.pStages = guiShaderStages;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "gui");
		_pipelines.insert(make_pair(PIPE_GUI, pipeline));

		// Font

		VkPipelineShaderStageCreateInfo fontShaderStages[] = { guiVertShaderStageInfo, fontFragShaderStageInfo };

		pipelineInfo.pStages = fontShaderStages;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "font");
		_pipelines.insert(make_pair(PIPE_Font, pipeline));
	}

	//************************
	//* PostProcess pipelines
	//************************

	{
		/*VkPipelineShaderStageCreateInfo hdrShaderStages[] = { fsVertShaderStageInfo, hdrFragShaderStageInfo };
		pipelineInfo.pStages = hdrShaderStages;

		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_PostProcess];
		pipelineInfo.pVertexInputState = &emptyVertexInputInfo;
		rasterizer.cullMode = VK_CULL_MODE_NONE;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline (HDR)");
			return ENGINE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "postprocess");
		_pipelines.insert(make_pair(PIPE_HDR, pipeline));*/
	}

	return ENGINE_OK;
}

int PipelineManager::_CreatePipelineLayouts()
{
	VkPipelineLayout layout;

	VkPushConstantRange range{};
	range.offset = 0;
	range.size = sizeof(MaterialData);
	range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &range;

	// Static
	{
		pipelineLayoutInfo.setLayoutCount = 3;

		VkDescriptorSetLayout oneSamplerLayouts[] = { _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Object], _descriptorSetLayouts[DESC_LYT_OneSampler] };
		pipelineLayoutInfo.pSetLayouts = oneSamplerLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (one sampler)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "one sampler");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_OneSampler, layout));

		VkDescriptorSetLayout twoSamplersLayouts[] = { _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Object], _descriptorSetLayouts[DESC_LYT_TwoSamplers] };
		pipelineLayoutInfo.pSetLayouts = twoSamplersLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (two samplers)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "two samplers");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_TwoSamplers, layout));

		VkDescriptorSetLayout threeSamplersLayouts[] = { _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Object], _descriptorSetLayouts[DESC_LYT_ThreeSamplers] };
		pipelineLayoutInfo.pSetLayouts = threeSamplersLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (three samplers)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "three samplers");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_ThreeSamplers, layout));

		VkDescriptorSetLayout fourSamplersLayouts[] = { _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Object], _descriptorSetLayouts[DESC_LYT_FourSamplers] };
		pipelineLayoutInfo.pSetLayouts = fourSamplersLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (four samplers)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "four samplers");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_FourSamplers, layout));

		VkDescriptorSetLayout depthOnlyLayouts[] = { _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Object] };
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = depthOnlyLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (depth)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "depth");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_Depth, layout));
	}

	// Animated
	{
		pipelineLayoutInfo.setLayoutCount = 3;
		
		VkDescriptorSetLayout oneSamplerLayouts[] = { _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Anim_Object], _descriptorSetLayouts[DESC_LYT_OneSampler] };
		pipelineLayoutInfo.pSetLayouts = oneSamplerLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (animated one sampler)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "animated one sampler");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_Anim_OneSampler, layout));

		VkDescriptorSetLayout twoSamplersLayouts[] = { _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Anim_Object], _descriptorSetLayouts[DESC_LYT_TwoSamplers] };
		pipelineLayoutInfo.pSetLayouts = twoSamplersLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (animated two samplers)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "animated two samplers");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_Anim_TwoSamplers, layout));

		VkDescriptorSetLayout threeSamplersLayouts[] = { _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Anim_Object], _descriptorSetLayouts[DESC_LYT_ThreeSamplers] };
		pipelineLayoutInfo.pSetLayouts = threeSamplersLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (animated three samplers)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "animated three samplers");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_Anim_ThreeSamplers, layout));

		VkDescriptorSetLayout fourSamplersLayouts[] = { _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Anim_Object], _descriptorSetLayouts[DESC_LYT_FourSamplers] };
		pipelineLayoutInfo.pSetLayouts = fourSamplersLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (animated four samplers)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "animated four samplers");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_Anim_FourSamplers, layout));

		VkDescriptorSetLayout depthOnlyLayouts[] = { _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Anim_Object] };
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = depthOnlyLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (animated depth)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "animated depth");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_Anim_Depth, layout));
	}

	//***************
	//* GUI pipeline
	//***************

	VkDescriptorSetLayout guiLayouts[] = { _descriptorSetLayouts[DESC_LYT_Object], _descriptorSetLayouts[DESC_LYT_OneSampler] };
	pipelineLayoutInfo.pSetLayouts = guiLayouts;
	pipelineLayoutInfo.setLayoutCount = 2;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (gui)");
		return ENGINE_PIPELINE_LYT_CREATE_FAIL;
	}
	DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "gui");
	_pipelineLayouts.insert(make_pair(PIPE_LYT_GUI, layout));

	//*************************
	//* Post process pipelines
	//*************************

	VkDescriptorSetLayout ppLayouts[] = { _descriptorSetLayouts[DESC_LYT_PostProcess] };
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = ppLayouts;

	VkPushConstantRange blurRange{};
	blurRange.offset = 0;
	blurRange.size = sizeof(float) * 4;
	blurRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &blurRange;
	
	if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (post process)");
		return ENGINE_PIPELINE_LYT_CREATE_FAIL;
	}
	DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "postprocess");
	_pipelineLayouts.insert(make_pair(PIPE_LYT_PostProcess, layout));

	VkDescriptorSetLayout blurLayouts[] = { _descriptorSetLayouts[DESC_LYT_PostProcess], _descriptorSetLayouts[DESC_LYT_OneSampler] };
	pipelineLayoutInfo.setLayoutCount = 2;
	pipelineLayoutInfo.pSetLayouts = blurLayouts;

	if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (blur)");
		return ENGINE_PIPELINE_LYT_CREATE_FAIL;
	}
	DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "blur");
	_pipelineLayouts.insert(make_pair(PIPE_LYT_Blur, layout));

	VkDescriptorSetLayout dofLayouts[] = { _descriptorSetLayouts[DESC_LYT_PostProcess], _descriptorSetLayouts[DESC_LYT_OneSampler], _descriptorSetLayouts[DESC_LYT_OneSampler] };
	pipelineLayoutInfo.setLayoutCount = 3;
	pipelineLayoutInfo.pSetLayouts = dofLayouts;

	if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (dof)");
		return ENGINE_PIPELINE_LYT_CREATE_FAIL;
	}
	DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "dof");
	_pipelineLayouts.insert(make_pair(PIPE_LYT_DoF, layout));

	return ENGINE_OK;
}

int PipelineManager::_CreateDescriptorSetLayouts()
{
	//***********************
	//* 1. Scene descriptors
	//***********************
	VkDescriptorSetLayout dsl;

	VkDescriptorSetLayoutBinding sceneDataBlockBinding = {};
	sceneDataBlockBinding.binding = 0;
	sceneDataBlockBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	sceneDataBlockBinding.descriptorCount = 1;
	sceneDataBlockBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	sceneDataBlockBinding.pImmutableSamplers = nullptr;
	
	VkDescriptorSetLayoutBinding lightBlockBinding = {};
	lightBlockBinding.binding = 1;
	lightBlockBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	lightBlockBinding.descriptorCount = 1;
	lightBlockBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	lightBlockBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding visibleIndicesBlockBinding = {};
	visibleIndicesBlockBinding.binding = 2;
	visibleIndicesBlockBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	visibleIndicesBlockBinding.descriptorCount = 1;
	visibleIndicesBlockBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	visibleIndicesBlockBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding aoBinding{};
	aoBinding.binding = 3;
	aoBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	aoBinding.descriptorCount = 1;
	aoBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	aoBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding sceneLayouts[4] = { sceneDataBlockBinding, lightBlockBinding, visibleIndicesBlockBinding, aoBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 4;
	layoutInfo.pBindings = sceneLayouts;

	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (scene)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "scene");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_Scene, dsl));

	//************************
	//* 2. Object descriptors
	//************************

	VkDescriptorSetLayoutBinding matrixBlockBinding{};
	matrixBlockBinding.binding = 0;
	matrixBlockBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	matrixBlockBinding.descriptorCount = 1;
	matrixBlockBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	matrixBlockBinding.pImmutableSamplers = nullptr;

	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &matrixBlockBinding;

	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (object)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "object");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_Object, dsl));

	VkDescriptorSetLayoutBinding boneBlockBinding{};
	boneBlockBinding.binding = 1;
	boneBlockBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	boneBlockBinding.descriptorCount = 1;
	boneBlockBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	boneBlockBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding animBindings[]{ matrixBlockBinding,boneBlockBinding };
	layoutInfo.bindingCount = 2;
	layoutInfo.pBindings = animBindings;

	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (bones)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "bones");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_Anim_Object, dsl));

	//*************************
	//* 3. Sampler descriptors
	//*************************

	VkDescriptorSetLayoutBinding samplerBinding{};
	samplerBinding.binding = 0;
	samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerBinding.descriptorCount = 1;
	samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerBinding.pImmutableSamplers = nullptr;

	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &samplerBinding;

	// One sampler
	samplerBinding.descriptorCount = 1;
	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (one sampler)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "one sampler");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_OneSampler, dsl));

	// Two samplers
	++samplerBinding.descriptorCount;
	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (two samplers)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "two samplers");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_TwoSamplers, dsl));

	// Three samplers
	++samplerBinding.descriptorCount;
	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (three samplers)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "three samplers");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_ThreeSamplers, dsl));

	// Four samplers
	++samplerBinding.descriptorCount;
	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (four samplers)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "four samplers");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_FourSamplers, dsl));

	//*****************************
	//* 3. PostProcess descriptors
	//*****************************

	VkDescriptorSetLayoutBinding iaBindings[3]{};
	iaBindings[0].binding = 0;
	iaBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	iaBindings[0].descriptorCount = 1;
	iaBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	iaBindings[0].pImmutableSamplers = nullptr;
	iaBindings[1].binding = 1;
	iaBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	iaBindings[1].descriptorCount = 1;
	iaBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	iaBindings[1].pImmutableSamplers = nullptr;

	layoutInfo.bindingCount = 2;
	layoutInfo.pBindings = iaBindings;

	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (post process)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "post process");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_PostProcess, dsl));

	return ENGINE_OK;
}

int PipelineManager::_LoadComputeShaders()
{
	ShaderModule *module = nullptr;

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_light_culling", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Compute_Culling, module));

	return ENGINE_OK;
}

int PipelineManager::_CreateComputePipelines()
{
	VkPipeline pipeline;

	VkComputePipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	pipelineInfo.stage.module = _shaderModules[SH_Compute_Culling]->GetHandle();
	pipelineInfo.stage.pName = "main";
	pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Culling];
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateComputePipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create compute pipeline");
		return ENGINE_PIPELINE_CREATE_FAIL;
	}
	DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "culling");
	_pipelines.insert(make_pair(PIPE_Culling, pipeline));

	return ENGINE_OK;
}

int PipelineManager::_CreateComputePipelineLayouts()
{
	VkPipelineLayout layout = VK_NULL_HANDLE;

	VkDescriptorSetLayout lightCullingLayouts[] = { _descriptorSetLayouts[DESC_LYT_Culling] };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = lightCullingLayouts;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (light culling)");
		return ENGINE_PIPELINE_LYT_CREATE_FAIL;
	}
	DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "culling");
	_pipelineLayouts.insert(make_pair(PIPE_LYT_Culling, layout));

	return ENGINE_OK;
}

int PipelineManager::_CreateComputeDescriptorSetLayouts()
{
	VkDescriptorSetLayout dsl = VK_NULL_HANDLE;

	VkDescriptorSetLayoutBinding lightBufferBinding = {};
	lightBufferBinding.binding = 0;
	lightBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	lightBufferBinding.descriptorCount = 1;
	lightBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	lightBufferBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding visibleIndicesBlockBinding = {};
	visibleIndicesBlockBinding.binding = 1;
	visibleIndicesBlockBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	visibleIndicesBlockBinding.descriptorCount = 1;
	visibleIndicesBlockBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	visibleIndicesBlockBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding dataBlockBinding = {};
	dataBlockBinding.binding = 2;
	dataBlockBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	dataBlockBinding.descriptorCount = 1;
	dataBlockBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	dataBlockBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding depthMapBinding = {};
	depthMapBinding.binding = 3;
	depthMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	depthMapBinding.descriptorCount = 1;
	depthMapBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	depthMapBinding.pImmutableSamplers = nullptr;

	array<VkDescriptorSetLayoutBinding, 4> layouts = { { lightBufferBinding, visibleIndicesBlockBinding, dataBlockBinding, depthMapBinding } };

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = (uint32_t)layouts.size();
	layoutInfo.pBindings = layouts.data();

	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (light culling)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "culling");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_Culling, dsl));

	return ENGINE_OK;
}

void PipelineManager::Release()
{
	for (pair<uint8_t, VkDescriptorSetLayout> kvp : _descriptorSetLayouts)
		vkDestroyDescriptorSetLayout(VKUtil::GetDevice(), kvp.second, VKUtil::GetAllocator());

	for (pair<uint8_t, VkPipelineLayout> kvp : _pipelineLayouts)
		vkDestroyPipelineLayout(VKUtil::GetDevice(), kvp.second, VKUtil::GetAllocator());

	for (pair<uint8_t, VkPipeline> kvp : _pipelines)
		vkDestroyPipeline(VKUtil::GetDevice(), kvp.second, VKUtil::GetAllocator());
}
