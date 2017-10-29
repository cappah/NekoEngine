/* NekoEngine
 *
 * PipelineManager.cpp
 * Author: Alexandru Naiman
 *
 * PipelineManager
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

#include <GUI/GUI.h>
#include <Engine/Defs.h>
#include <Engine/Vertex.h>
#include <Engine/Engine.h>
#include <Engine/ResourceManager.h>
#include <Renderer/VKUtil.h>
#include <Renderer/Material.h>
#include <Renderer/DebugMarker.h>
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
#define SH_FRAG_WF					0

using namespace std;
using namespace glm;

enum ShaderId : uint8_t
{
	SH_Vertex,
	SH_Vertex_Anim,
	SH_Vertex_Depth,
	SH_Vertex_Depth_Anim,
	SH_Vertex_Shadow,
	SH_Vertex_Shadow_Anim,
	SH_Vertex_Skysphere,
	SH_Vertex_GUI,
	SH_Vertex_Terrain,
	SH_Vertex_Terrain_Depth,
	SH_Vertex_Terrain_Shadow,
	SH_Vertex_Billboard,
	SH_Vertex_Bounds,
	SH_Vertex_Fullscreen,
	SH_Geometry_Billboard,
	SH_Fragment_Phong,
	SH_Fragment_Depth,
	SH_Fragment_GUI,
	SH_Fragment_Skysphere,
	SH_Fragment_Postprocess,
	SH_Fragment_Shadow,
	SH_Fragment_ShadowFilter,
	SH_Fragment_Billboard,
	SH_Fragment_Bounds,
	SH_Compute_Culling,
	SH_Compute_ParticleUpdate,
	SH_Compute_ParticleSort,
	SH_Compute_ParticleEmit
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

int PipelineManager::RecreatePipelines()
{
	int ret = -1;

	_DestroyPipelines();

	if ((ret = _CreatePipelines()) != ENGINE_OK)
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

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_shadow_vertex", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Vertex_Shadow, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_shadow_anim_vertex", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Vertex_Shadow_Anim, module));

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

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_shadow_terrain_vertex", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Vertex_Terrain_Shadow, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_billboard_vertex", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Vertex_Billboard, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_bounds_vertex", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Vertex_Bounds, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_fullscreen_vertex", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Vertex_Fullscreen, module));

	// Geometry shaders

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_billboard_geometry", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Geometry_Billboard, module));

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

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_shadow_fragment", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Fragment_Shadow, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_shadow_filter", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Fragment_ShadowFilter, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_billboard_fragment", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Fragment_Billboard, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_bounds_fragment", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Fragment_Bounds, module));

	return ENGINE_OK;
}

int PipelineManager::_CreatePipelines()
{
	VkPipeline pipeline{ VK_NULL_HANDLE };

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
		} frag;
	} shaderSpecData;

	shaderSpecData.vtx.type = 0;
	shaderSpecData.frag.type = 0;
	shaderSpecData.frag.numTextures = 1;

	VkSpecializationMapEntry vtxShaderSpecMap[1]{};
	vtxShaderSpecMap[0].constantID = 0;
	vtxShaderSpecMap[0].offset = 0;
	vtxShaderSpecMap[0].size = sizeof(int32_t);

	VkSpecializationMapEntry fragShaderSpecMap[2]{};
	fragShaderSpecMap[0].constantID = 10;
	fragShaderSpecMap[0].offset = 0;
	fragShaderSpecMap[0].size = sizeof(int32_t);
	fragShaderSpecMap[1].constantID = 11;
	fragShaderSpecMap[1].offset = sizeof(int32_t);
	fragShaderSpecMap[1].size = sizeof(int32_t);

	//****************
	//* Shader stages
	//****************

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	VkPipelineShaderStageCreateInfo animVertShaderStageInfo{};
	VkPipelineShaderStageCreateInfo terrainVertShaderStageInfo{};
	VkPipelineShaderStageCreateInfo fsVertShaderStageInfo{};
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
	VkPipelineShaderStageCreateInfo shadowVertShaderStageInfo{};
	VkPipelineShaderStageCreateInfo shadowAnimVertShaderStageInfo{};
	VkPipelineShaderStageCreateInfo shadowTerrainVertShaderStageInfo{};
	VkPipelineShaderStageCreateInfo shadowFragShaderStageInfo{};
	VkPipelineShaderStageCreateInfo shadowFilterShaderStageInfo{};
	VkPipelineShaderStageCreateInfo billboardVertShaderStageInfo{};
	VkPipelineShaderStageCreateInfo billboardGeomShaderStageInfo{};
	VkPipelineShaderStageCreateInfo billboardFragShaderStageInfo{};
	VkPipelineShaderStageCreateInfo boundsVertShaderStageInfo{};
	VkPipelineShaderStageCreateInfo boundsFragShaderStageInfo{};
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

		// Vertex
		VKUtil::InitShaderStage(&vertShaderStageInfo, VK_SHADER_STAGE_VERTEX_BIT, _shaderModules[SH_Vertex]->GetHandle(), &vtxShaderSpecInfo);
		VKUtil::InitShaderStage(&animVertShaderStageInfo, VK_SHADER_STAGE_VERTEX_BIT, _shaderModules[SH_Vertex_Anim]->GetHandle(), &vtxShaderSpecInfo);
		VKUtil::InitShaderStage(&terrainVertShaderStageInfo, VK_SHADER_STAGE_VERTEX_BIT, _shaderModules[SH_Vertex_Terrain]->GetHandle(), &vtxShaderSpecInfo);
		VKUtil::InitShaderStage(&fsVertShaderStageInfo, VK_SHADER_STAGE_VERTEX_BIT, _shaderModules[SH_Vertex_Fullscreen]->GetHandle());
		VKUtil::InitShaderStage(&skyVertShaderStageInfo, VK_SHADER_STAGE_VERTEX_BIT, _shaderModules[SH_Vertex_Skysphere]->GetHandle());
		VKUtil::InitShaderStage(&depthVertShaderStageInfo, VK_SHADER_STAGE_VERTEX_BIT, _shaderModules[SH_Vertex_Depth]->GetHandle(), &vtxShaderSpecInfo);
		VKUtil::InitShaderStage(&depthAnimVertShaderStageInfo, VK_SHADER_STAGE_VERTEX_BIT, _shaderModules[SH_Vertex_Depth_Anim]->GetHandle(), &vtxShaderSpecInfo);
		VKUtil::InitShaderStage(&depthTerrainVertShaderStageInfo, VK_SHADER_STAGE_VERTEX_BIT, _shaderModules[SH_Vertex_Terrain_Depth]->GetHandle(), &vtxShaderSpecInfo);
		VKUtil::InitShaderStage(&guiVertShaderStageInfo, VK_SHADER_STAGE_VERTEX_BIT, _shaderModules[SH_Vertex_GUI]->GetHandle());
		VKUtil::InitShaderStage(&shadowVertShaderStageInfo, VK_SHADER_STAGE_VERTEX_BIT, _shaderModules[SH_Vertex_Shadow]->GetHandle());
		VKUtil::InitShaderStage(&shadowAnimVertShaderStageInfo, VK_SHADER_STAGE_VERTEX_BIT, _shaderModules[SH_Vertex_Shadow_Anim]->GetHandle());
		VKUtil::InitShaderStage(&shadowTerrainVertShaderStageInfo, VK_SHADER_STAGE_VERTEX_BIT, _shaderModules[SH_Vertex_Terrain_Shadow]->GetHandle());
		VKUtil::InitShaderStage(&billboardVertShaderStageInfo, VK_SHADER_STAGE_VERTEX_BIT, _shaderModules[SH_Vertex_Billboard]->GetHandle());
		VKUtil::InitShaderStage(&boundsVertShaderStageInfo, VK_SHADER_STAGE_VERTEX_BIT, _shaderModules[SH_Vertex_Bounds]->GetHandle());

		// Geometry
		VKUtil::InitShaderStage(&billboardGeomShaderStageInfo, VK_SHADER_STAGE_GEOMETRY_BIT, _shaderModules[SH_Geometry_Billboard]->GetHandle());

		// Fragment
		VKUtil::InitShaderStage(&fragShaderStageInfo, VK_SHADER_STAGE_FRAGMENT_BIT, _shaderModules[SH_Fragment_Phong]->GetHandle(), &fragShaderSpecInfo);
		VKUtil::InitShaderStage(&skyFragShaderStageInfo, VK_SHADER_STAGE_FRAGMENT_BIT, _shaderModules[SH_Fragment_Skysphere]->GetHandle());
		VKUtil::InitShaderStage(&depthFragShaderStageInfo, VK_SHADER_STAGE_FRAGMENT_BIT, _shaderModules[SH_Fragment_Depth]->GetHandle(), &fragShaderSpecInfo);
		VKUtil::InitShaderStage(&guiFragShaderStageInfo, VK_SHADER_STAGE_FRAGMENT_BIT, _shaderModules[SH_Fragment_GUI]->GetHandle(), &guiSpecInfo);
		VKUtil::InitShaderStage(&fontFragShaderStageInfo, VK_SHADER_STAGE_FRAGMENT_BIT, _shaderModules[SH_Fragment_GUI]->GetHandle(), &fontSpecInfo);
		VKUtil::InitShaderStage(&shadowFragShaderStageInfo, VK_SHADER_STAGE_FRAGMENT_BIT, _shaderModules[SH_Fragment_Shadow]->GetHandle());		
		VKUtil::InitShaderStage(&shadowFilterShaderStageInfo, VK_SHADER_STAGE_FRAGMENT_BIT, _shaderModules[SH_Fragment_ShadowFilter]->GetHandle());
		VKUtil::InitShaderStage(&billboardFragShaderStageInfo, VK_SHADER_STAGE_FRAGMENT_BIT, _shaderModules[SH_Fragment_Billboard]->GetHandle());
		VKUtil::InitShaderStage(&boundsFragShaderStageInfo, VK_SHADER_STAGE_FRAGMENT_BIT, _shaderModules[SH_Fragment_Bounds]->GetHandle());
	}

	//********************
	//* Shared structures
	//********************

	VkVertexInputBindingDescription bindingDesc{ Vertex::GetBindingDescription() };
	NArray<VkVertexInputAttributeDescription> attribDesc{ Vertex::GetAttributeDescriptions() };
	VkVertexInputBindingDescription animBindingDesc{ SkeletalVertex::GetBindingDescription() };
	NArray<VkVertexInputAttributeDescription> animAttribDesc{ SkeletalVertex::GetAttributeDescriptions() };
	VkVertexInputBindingDescription terrainBindingDesc{ TerrainVertex::GetBindingDescription() };
	NArray<VkVertexInputAttributeDescription> terrainAttribDesc{ TerrainVertex::GetAttributeDescriptions() };
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	VkPipelineVertexInputStateCreateInfo animVertexInputInfo{};
	VkPipelineVertexInputStateCreateInfo terrainVertexInputInfo{};
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	VkViewport viewport{};
	VkRect2D scissor{};
	VkPipelineViewportStateCreateInfo viewportState{};
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	VkPipelineMultisampleStateCreateInfo multisampling{};
	VkPipelineMultisampleStateCreateInfo noMultisampling{};
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	VkPipelineDepthStencilStateCreateInfo transparentDepthStencil{};
	VkPipelineColorBlendAttachmentState colorBlendAttachments[2]{};
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	VkPipelineColorBlendAttachmentState transparentColorBlendAttachments[2]{};
	VkPipelineColorBlendStateCreateInfo transparentColorBlending{};
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	{
		VKUtil::InitVertexInput(&vertexInputInfo, 1, &bindingDesc, (uint32_t)attribDesc.Count(), *attribDesc);
		VKUtil::InitVertexInput(&animVertexInputInfo, 1, &animBindingDesc, (uint32_t)animAttribDesc.Count(), *animAttribDesc);
		VKUtil::InitVertexInput(&terrainVertexInputInfo, 1, &terrainBindingDesc, (uint32_t)terrainAttribDesc.Count(), *terrainAttribDesc);

		VKUtil::InitInputAssembly(&inputAssembly);

		VKUtil::InitViewport(&viewport, (float)Engine::GetScreenWidth(), (float)Engine::GetScreenHeight());
		VKUtil::InitScissor(&scissor, Engine::GetScreenWidth(), Engine::GetScreenHeight());
		VKUtil::InitViewportState(&viewportState, 1, &viewport, 1, &scissor);		

		VKUtil::InitRasterizationState(&rasterizer);

		if (Engine::GetConfiguration().Renderer.Multisampling)
		{
			switch (Engine::GetConfiguration().Renderer.Samples)
			{
				case 2: VKUtil::InitMultisampleState(&multisampling, VK_SAMPLE_COUNT_2_BIT); break;
				case 4: VKUtil::InitMultisampleState(&multisampling, VK_SAMPLE_COUNT_4_BIT); break;
				case 8: VKUtil::InitMultisampleState(&multisampling, VK_SAMPLE_COUNT_8_BIT); break;
				case 16: VKUtil::InitMultisampleState(&multisampling, VK_SAMPLE_COUNT_16_BIT); break;
				case 32: VKUtil::InitMultisampleState(&multisampling, VK_SAMPLE_COUNT_32_BIT); break;
				case 64: VKUtil::InitMultisampleState(&multisampling, VK_SAMPLE_COUNT_64_BIT); break;
			}
		}
		else
			VKUtil::InitMultisampleState(&multisampling, VK_SAMPLE_COUNT_1_BIT);

		VKUtil::InitMultisampleState(&noMultisampling, VK_SAMPLE_COUNT_1_BIT);

		VKUtil::InitDepthState(&depthStencil, VK_TRUE, VK_FALSE, VK_COMPARE_OP_EQUAL);
		VKUtil::InitStencilState(&depthStencil, VK_FALSE);
		
		VKUtil::InitDepthState(&transparentDepthStencil, VK_TRUE, VK_FALSE, VK_COMPARE_OP_EQUAL);
		VKUtil::InitStencilState(&transparentDepthStencil, VK_FALSE);

		VKUtil::InitColorBlendAttachmentState(&colorBlendAttachments[0], VK_FALSE);
		VKUtil::InitColorBlendAttachmentState(&colorBlendAttachments[1], VK_FALSE);		
		VKUtil::InitColorBlendState(&colorBlending, 2, colorBlendAttachments);

		VKUtil::InitColorBlendAttachmentState(&transparentColorBlendAttachments[0], VK_TRUE);
		VKUtil::InitColorBlendAttachmentState(&transparentColorBlendAttachments[1], VK_FALSE);
		VKUtil::InitColorBlendState(&transparentColorBlending, 2, transparentColorBlendAttachments);
		
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
		VkPipelineShaderStageCreateInfo shaderStages[]{ vertShaderStageInfo, fragShaderStageInfo };
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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "unlit");
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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "phong");
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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "phong specular");
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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "phong specular emissive");
		_pipelines.insert(make_pair(PIPE_PhongSpecularEmissive, pipeline));

		// Phong with normal map
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_NM;
		shaderSpecData.frag.numTextures = 1;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_OneSampler];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "phong normal");
		_pipelines.insert(make_pair(PIPE_PhongNormal, pipeline));

		// Phong with specular and normal
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_NM;
		shaderSpecData.frag.numTextures = 2;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_TwoSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "phong normal specular");
		_pipelines.insert(make_pair(PIPE_PhongNormalSpecular, pipeline));

		// Phong with specular, normal and emission
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_EM_NM;
		shaderSpecData.frag.numTextures = 3;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_ThreeSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "phong normal specular emissive");
		_pipelines.insert(make_pair(PIPE_PhongNormalSpecularEmissive, pipeline));
	}

	// Transparent
	{
		pipelineInfo.pColorBlendState = &transparentColorBlending;
		pipelineInfo.pDepthStencilState = &transparentDepthStencil;

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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent unlit");
		_pipelines.insert(make_pair(PIPE_Transparent_Unlit, pipeline));

		// Phong
		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_PHONG;
		shaderSpecData.frag.numTextures = 1;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_OneSampler];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create phong pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent phong");
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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent phong specular");
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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent phong specular emissive");
		_pipelines.insert(make_pair(PIPE_Transparent_PhongSpecularEmissive, pipeline));

		// Phong with normal map
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_NM;
		shaderSpecData.frag.numTextures = 1;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_OneSampler];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent phong normal");
		_pipelines.insert(make_pair(PIPE_Transparent_PhongNormal, pipeline));

		// Phong with specular and normal
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_NM;
		shaderSpecData.frag.numTextures = 2;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_TwoSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent phong normal specular");
		_pipelines.insert(make_pair(PIPE_Transparent_PhongNormalSpecular, pipeline));

		// Phong with specular, normal and emission
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_EM_NM;
		shaderSpecData.frag.numTextures = 3;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_ThreeSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent phong normal specular emissive");
		_pipelines.insert(make_pair(PIPE_Transparent_PhongNormalSpecularEmissive, pipeline));

		pipelineInfo.pDepthStencilState = &depthStencil;
	}

	// Debug
	{
		const VkPipelineMultisampleStateCreateInfo *oldMSCI{ pipelineInfo.pMultisampleState };
		pipelineInfo.pMultisampleState = &noMultisampling;

		const VkPipelineColorBlendStateCreateInfo *oldCBSCI{ pipelineInfo.pColorBlendState };
		const VkPipelineDepthStencilStateCreateInfo *oldDSCI{ pipelineInfo.pDepthStencilState };

		VkPipelineShaderStageCreateInfo shaderStages[]{ boundsVertShaderStageInfo, boundsFragShaderStageInfo };
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_FRAG_WF;
		shaderSpecData.frag.numTextures = 1;

		VkPipelineDepthStencilStateCreateInfo wfDepthStencil{};
		VKUtil::InitDepthState(&wfDepthStencil, VK_FALSE, VK_FALSE);
		VKUtil::InitStencilState(&wfDepthStencil, VK_FALSE);

		VkPipelineColorBlendStateCreateInfo wfColorBlending{};
		VKUtil::InitColorBlendState(&wfColorBlending, 1, &transparentColorBlendAttachments[0]);

		VkPipelineRasterizationStateCreateInfo wfRasterizationState{};
		VKUtil::InitRasterizationState(&wfRasterizationState, VK_POLYGON_MODE_LINE, VK_CULL_MODE_NONE);

		pipelineInfo.pColorBlendState = &wfColorBlending;
		pipelineInfo.pDepthStencilState = &wfDepthStencil;
		pipelineInfo.pRasterizationState = &wfRasterizationState;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Debug];		
		pipelineInfo.renderPass = RenderPassManager::GetRenderPass(RP_GUI);

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create bounds pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "Bounds (DEBUG)");
		_pipelines.insert(make_pair(PIPE_Bounds, pipeline));

		/*VkPipelineVertexInputStateCreateInfo empty{};
		VKUtil::InitVertexInput(&empty);

		VkPipelineInputAssemblyStateCreateInfo debugIA{};
		VKUtil::InitInputAssembly(&debugIA, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);

		pipelineInfo.pVertexInputState = &empty;
		pipelineInfo.pInputAssemblyState = &debugIA;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create bounds pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "Bounds (DEBUG)");
		_pipelines.insert(make_pair(PIPE_DebugLine, pipeline));*/

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.renderPass = RenderPassManager::GetRenderPass(RP_Graphics);
		pipelineInfo.pMultisampleState = oldMSCI;
		pipelineInfo.pColorBlendState = oldCBSCI;
		pipelineInfo.pDepthStencilState = oldDSCI;
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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "animated unlit");
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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "animated phong");
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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "animated phong specular");
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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "animated phong specular emissive");
		_pipelines.insert(make_pair(PIPE_Anim_PhongSpecularEmissive, pipeline));

		// Phong with normal map
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_NM;
		shaderSpecData.frag.numTextures = 1;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_OneSampler];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "animated phong normal");
		_pipelines.insert(make_pair(PIPE_Anim_PhongNormal, pipeline));

		// Phong with specular and normal
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_NM;
		shaderSpecData.frag.numTextures = 2;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_TwoSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "animated phong normal specular");
		_pipelines.insert(make_pair(PIPE_Anim_PhongNormalSpecular, pipeline));

		// Phong with specular, normal and emission
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_EM_NM;
		shaderSpecData.frag.numTextures = 3;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_ThreeSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "animated phong normal specular emissive");
		_pipelines.insert(make_pair(PIPE_Anim_PhongNormalSpecularEmissive, pipeline));
	}

	// Transparent
	{
		pipelineInfo.pColorBlendState = &transparentColorBlending;
		pipelineInfo.pDepthStencilState = &transparentDepthStencil;

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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent animated unlit");
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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent animated phong");
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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent animated phong specular");
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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent animated phong specular emissive");
		_pipelines.insert(make_pair(PIPE_Transparent_Anim_PhongSpecularEmissive, pipeline));

		// Phong with normal map
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_NM;
		shaderSpecData.frag.numTextures = 1;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_OneSampler];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent animated phong normal");
		_pipelines.insert(make_pair(PIPE_Transparent_Anim_PhongNormal, pipeline));

		// Phong with specular and normal
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_NM;
		shaderSpecData.frag.numTextures = 2;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_TwoSamplers];
	
		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent animated phong normal specular");
		_pipelines.insert(make_pair(PIPE_Transparent_Anim_PhongNormalSpecular, pipeline));

		// Phong with specular, normal and emission
		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_FRAG_PHONG_SPEC_EM_NM;
		shaderSpecData.frag.numTextures = 3;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_ThreeSamplers];

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create animated pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "transparent animated phong normal specular emissive");
		_pipelines.insert(make_pair(PIPE_Transparent_Anim_PhongNormalSpecularEmissive, pipeline));

		pipelineInfo.pDepthStencilState = &depthStencil;
	}

	//*********************
	//* Skysphere pipeline
	//*********************

	pipelineInfo.pColorBlendState = &colorBlending;

	{
		depthStencil.depthTestEnable = VK_FALSE;
		pipelineInfo.pVertexInputState = &vertexInputInfo;

		VkPipelineShaderStageCreateInfo skyShaderStages[] = { skyVertShaderStageInfo, skyFragShaderStageInfo };
		pipelineInfo.pStages = skyShaderStages;

		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_OneSampler];
		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "skysphere");
		_pipelines.insert(make_pair(PIPE_Skysphere, pipeline));

		depthStencil.depthTestEnable = VK_TRUE;
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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "terrain");
		_pipelines.insert(make_pair(PIPE_Terrain, pipeline));
	}

	//***************************
	//* Depth & normal pipelines
	//***************************

	{
		pipelineInfo.pVertexInputState = &vertexInputInfo;

		VkPipelineColorBlendStateCreateInfo dpColorBlending{};
		VKUtil::InitColorBlendState(&dpColorBlending, 1, &colorBlendAttachments[0]);

		VkPipelineDepthStencilStateCreateInfo dpDepthStencil{};
		VKUtil::InitDepthState(&dpDepthStencil, VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);

		pipelineInfo.pDepthStencilState = &dpDepthStencil;
		pipelineInfo.pColorBlendState = &dpColorBlending;
		pipelineInfo.renderPass = RenderPassManager::GetRenderPass(RP_Depth);

		VkPipelineShaderStageCreateInfo depthShaderStages[]{ depthVertShaderStageInfo, depthFragShaderStageInfo };
		pipelineInfo.pStages = depthShaderStages;
		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Depth];

		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_VTX;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "depth");
		_pipelines.insert(make_pair(PIPE_Depth, pipeline));

		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_VTX_NM;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "normal mapped depth");
		_pipelines.insert(make_pair(PIPE_DepthNormal, pipeline));

		VkPipelineShaderStageCreateInfo depthAnimShaderStages[]{ depthAnimVertShaderStageInfo, depthFragShaderStageInfo };
		pipelineInfo.pStages = depthAnimShaderStages;

		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_Depth];
		pipelineInfo.pVertexInputState = &animVertexInputInfo;

		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_VTX;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "animated depth");
		_pipelines.insert(make_pair(PIPE_Anim_Depth, pipeline));

		shaderSpecData.vtx.type = SH_VTX_NM;
		shaderSpecData.frag.type = SH_VTX_NM;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "normal mapped animated depth");
		_pipelines.insert(make_pair(PIPE_Anim_DepthNormal, pipeline));

		VkPipelineShaderStageCreateInfo depthTerrainShaderStages[]{ depthTerrainVertShaderStageInfo, depthFragShaderStageInfo };
		pipelineInfo.pStages = depthTerrainShaderStages;

		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Depth];
		pipelineInfo.pVertexInputState = &terrainVertexInputInfo;

		shaderSpecData.vtx.type = SH_VTX;
		shaderSpecData.frag.type = SH_VTX;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "terrain depth");
		_pipelines.insert(make_pair(PIPE_Terrain_Depth, pipeline));
	}
	
	//*******************
	//* Shadow pipelines
	//*******************

	{
		viewportState.pViewports = nullptr;
		viewportState.pScissors = nullptr;

		VkDynamicState shadowDynamicState[]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS };

		VkPipelineDynamicStateCreateInfo shadowDynamicStateCI{};
		shadowDynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		shadowDynamicStateCI.dynamicStateCount = 3;
		shadowDynamicStateCI.pDynamicStates = shadowDynamicState;

		VkPipelineColorBlendAttachmentState smAttachment{};
		smAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT;
		smAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo smColorBlending{};
		VKUtil::InitColorBlendState(&smColorBlending, 1, &smAttachment);

		VkPipelineDepthStencilStateCreateInfo smDepthStencil{};
		VKUtil::InitDepthState(&smDepthStencil);

		VkPipelineMultisampleStateCreateInfo smMultisample{};
		VKUtil::InitMultisampleState(&smMultisample, VK_SAMPLE_COUNT_1_BIT);

		VkPipelineVertexInputStateCreateInfo smFilterVertexInputInfo{};
		VKUtil::InitVertexInput(&smFilterVertexInputInfo);

		if (Engine::GetConfiguration().Renderer.ShadowMultisampling)
		{
			switch (Engine::GetConfiguration().Renderer.ShadowSamples)
			{
				case 2: VKUtil::InitMultisampleState(&smMultisample, VK_SAMPLE_COUNT_2_BIT); break;
				case 4: VKUtil::InitMultisampleState(&smMultisample, VK_SAMPLE_COUNT_4_BIT); break;
				case 8: VKUtil::InitMultisampleState(&smMultisample, VK_SAMPLE_COUNT_8_BIT); break;
				case 16: VKUtil::InitMultisampleState(&smMultisample, VK_SAMPLE_COUNT_16_BIT); break;
				case 32: VKUtil::InitMultisampleState(&smMultisample, VK_SAMPLE_COUNT_32_BIT); break;
				case 64: VKUtil::InitMultisampleState(&smMultisample, VK_SAMPLE_COUNT_64_BIT); break;
			}
		}

		pipelineInfo.pDynamicState = &shadowDynamicStateCI;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pMultisampleState = &smMultisample;
		pipelineInfo.pDepthStencilState = &smDepthStencil;
		pipelineInfo.pColorBlendState = &smColorBlending;
		pipelineInfo.renderPass = RenderPassManager::GetRenderPass(RP_ShadowMap);

		VkPipelineShaderStageCreateInfo shadowShaderStages[]{ shadowVertShaderStageInfo, shadowFragShaderStageInfo };
		pipelineInfo.pStages = shadowShaderStages;

		rasterizer.depthBiasEnable = VK_TRUE;
		rasterizer.depthBiasConstantFactor = 1.25f;
		rasterizer.depthBiasClamp = 0.f;
		rasterizer.depthBiasSlopeFactor = 1.75f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;

		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Shadow];
		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "shadow");
		_pipelines.insert(make_pair(PIPE_Shadow, pipeline));

		shadowShaderStages[0] = shadowAnimVertShaderStageInfo;
		pipelineInfo.pVertexInputState = &animVertexInputInfo;

		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Anim_Shadow];
		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "animated shadow");
		_pipelines.insert(make_pair(PIPE_Anim_Shadow, pipeline));

		shadowShaderStages[0] = shadowTerrainVertShaderStageInfo;

		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_Shadow];
		pipelineInfo.pVertexInputState = &terrainVertexInputInfo;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "terrain shadow");
		_pipelines.insert(make_pair(PIPE_Terrain_Shadow, pipeline));

		rasterizer.depthBiasEnable = VK_FALSE;
		smDepthStencil.depthTestEnable = VK_FALSE;
		shadowShaderStages[0] = fsVertShaderStageInfo;
		shadowShaderStages[1] = shadowFilterShaderStageInfo;
		shadowDynamicStateCI.dynamicStateCount = 2;

		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_ShadowFilter];
		pipelineInfo.pVertexInputState = &smFilterVertexInputInfo;
		pipelineInfo.renderPass = RenderPassManager::GetRenderPass(RP_ShadowFilter);
		
		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "shadow filter");
		_pipelines.insert(make_pair(PIPE_ShadowFilter, pipeline));

		viewportState.pViewports = &viewport;
		viewportState.pScissors = &scissor;

		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;

		pipelineInfo.pDynamicState = nullptr;
	}

	//***************
	//* GUI pipeline
	//***************

	pipelineInfo.pMultisampleState = &noMultisampling;

	{
		VkPipelineShaderStageCreateInfo guiShaderStages[] = { guiVertShaderStageInfo, guiFragShaderStageInfo };

		VkVertexInputBindingDescription guiBindingDesc = GUIVertex::GetBindingDescription();
		NArray<VkVertexInputAttributeDescription> guiAttribDesc = GUIVertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo guiVertexInputInfo{};
		VKUtil::InitVertexInput(&guiVertexInputInfo, 1, &guiBindingDesc, (uint32_t)guiAttribDesc.Count(), *guiAttribDesc);

		VkPipelineDepthStencilStateCreateInfo guiDepthStencil{};
		VKUtil::InitDepthState(&guiDepthStencil, VK_FALSE, VK_FALSE);

		VkPipelineColorBlendAttachmentState guiColorBlendAttachment{};
		VKUtil::InitColorBlendAttachmentState(&guiColorBlendAttachment, VK_TRUE,
											  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
											  VK_BLEND_OP_ADD, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
											  VK_BLEND_OP_ADD, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);

		VkPipelineColorBlendStateCreateInfo guiColorBlending{};
		VKUtil::InitColorBlendState(&guiColorBlending, 1, &guiColorBlendAttachment);

		VkPipelineRasterizationStateCreateInfo guiRasterizer{};
		VKUtil::InitRasterizationState(&guiRasterizer, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);

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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "gui");
		_pipelines.insert(make_pair(PIPE_GUI, pipeline));

		// Font

		VkPipelineShaderStageCreateInfo fontShaderStages[] = { guiVertShaderStageInfo, fontFragShaderStageInfo };

		pipelineInfo.pStages = fontShaderStages;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "font");
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
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "postprocess");
		_pipelines.insert(make_pair(PIPE_HDR, pipeline));*/
	}
	
	//***************************
	//* Particle System pipeline
	//***************************

	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &transparentColorBlending;
	pipelineInfo.renderPass = RenderPassManager::GetRenderPass(RP_Graphics);

	{
		VkPipelineShaderStageCreateInfo psShaderStages[]{ billboardVertShaderStageInfo, billboardGeomShaderStageInfo, billboardFragShaderStageInfo };
		pipelineInfo.pStages = psShaderStages;
		pipelineInfo.stageCount = 3;

		VkVertexInputBindingDescription psBindingDesc{ ParticleVertex::GetBindingDescription() };
		NArray<VkVertexInputAttributeDescription> psAttribDesc{ ParticleVertex::GetAttributeDescriptions() };

		VkPipelineVertexInputStateCreateInfo psVertexInputInfo{};
		VKUtil::InitVertexInput(&psVertexInputInfo, 1, &psBindingDesc, (uint32_t)psAttribDesc.Count(), *psAttribDesc);

		VkPipelineInputAssemblyStateCreateInfo psInputAssemblyInfo{};
		VKUtil::InitInputAssembly(&psInputAssemblyInfo, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);

		pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_ParticleDraw];
		pipelineInfo.pVertexInputState = &psVertexInputInfo;
		pipelineInfo.pInputAssemblyState = &psInputAssemblyInfo;
		pipelineInfo.pDepthStencilState = &depthStencil;

		rasterizer.cullMode = VK_CULL_MODE_NONE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthWriteEnable = VK_TRUE;

		if (vkCreateGraphicsPipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline (particle system draw)");
			return ENGINE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "particle system draw");
		_pipelines.insert(make_pair(PIPE_ParticleDraw, pipeline));

		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_EQUAL;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	}

	return ENGINE_OK;
}

int PipelineManager::_CreatePipelineLayouts()
{
	VkPipelineLayout layout{};

	VkPushConstantRange range{};
	range.offset = 0;
	range.size = sizeof(MaterialData);
	range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &range;

	// Static
	{
		pipelineLayoutInfo.setLayoutCount = 3;

		VkDescriptorSetLayout oneSamplerLayouts[]{ _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Object], _descriptorSetLayouts[DESC_LYT_OneSampler] };
		pipelineLayoutInfo.pSetLayouts = oneSamplerLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (one sampler)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "one sampler");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_OneSampler, layout));

		VkDescriptorSetLayout twoSamplersLayouts[]{ _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Object], _descriptorSetLayouts[DESC_LYT_TwoSamplers] };
		pipelineLayoutInfo.pSetLayouts = twoSamplersLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (two samplers)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "two samplers");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_TwoSamplers, layout));

		VkDescriptorSetLayout threeSamplersLayouts[]{ _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Object], _descriptorSetLayouts[DESC_LYT_ThreeSamplers] };
		pipelineLayoutInfo.pSetLayouts = threeSamplersLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (three samplers)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "three samplers");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_ThreeSamplers, layout));

		VkDescriptorSetLayout fourSamplersLayouts[]{ _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Object], _descriptorSetLayouts[DESC_LYT_FourSamplers] };
		pipelineLayoutInfo.pSetLayouts = fourSamplersLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (four samplers)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "four samplers");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_FourSamplers, layout));

		VkDescriptorSetLayout depthOnlyLayouts[]{ _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Object], _descriptorSetLayouts[DESC_LYT_OneSampler] };
		pipelineLayoutInfo.setLayoutCount = 3;
		pipelineLayoutInfo.pSetLayouts = depthOnlyLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (depth)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "depth");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_Depth, layout));
	}

	// Animated
	{
		pipelineLayoutInfo.setLayoutCount = 3;
		
		VkDescriptorSetLayout oneSamplerLayouts[]{ _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Anim_Object], _descriptorSetLayouts[DESC_LYT_OneSampler] };
		pipelineLayoutInfo.pSetLayouts = oneSamplerLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (animated one sampler)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "animated one sampler");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_Anim_OneSampler, layout));

		VkDescriptorSetLayout twoSamplersLayouts[]{ _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Anim_Object], _descriptorSetLayouts[DESC_LYT_TwoSamplers] };
		pipelineLayoutInfo.pSetLayouts = twoSamplersLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (animated two samplers)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "animated two samplers");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_Anim_TwoSamplers, layout));

		VkDescriptorSetLayout threeSamplersLayouts[]{ _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Anim_Object], _descriptorSetLayouts[DESC_LYT_ThreeSamplers] };
		pipelineLayoutInfo.pSetLayouts = threeSamplersLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (animated three samplers)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "animated three samplers");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_Anim_ThreeSamplers, layout));

		VkDescriptorSetLayout fourSamplersLayouts[]{ _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Anim_Object], _descriptorSetLayouts[DESC_LYT_FourSamplers] };
		pipelineLayoutInfo.pSetLayouts = fourSamplersLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (animated four samplers)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "animated four samplers");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_Anim_FourSamplers, layout));

		VkDescriptorSetLayout depthOnlyLayouts[]{ _descriptorSetLayouts[DESC_LYT_Scene], _descriptorSetLayouts[DESC_LYT_Anim_Object], _descriptorSetLayouts[DESC_LYT_OneSampler] };
		pipelineLayoutInfo.setLayoutCount = 3;
		pipelineLayoutInfo.pSetLayouts = depthOnlyLayouts;
		if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
		{
			Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (animated depth)");
			return ENGINE_PIPELINE_LYT_CREATE_FAIL;
		}
		VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "animated depth");
		_pipelineLayouts.insert(make_pair(PIPE_LYT_Anim_Depth, layout));
	}

	//*****************
	//* Debug pipeline
	//*****************

	VkPushConstantRange debugMVPRange{};
	debugMVPRange.offset = 0;
	debugMVPRange.size = sizeof(mat4);
	debugMVPRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &debugMVPRange;
	if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (bounds)");
		return ENGINE_PIPELINE_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "bounds");
	_pipelineLayouts.insert(make_pair(PIPE_LYT_Debug, layout));

	//***************
	//* GUI pipeline
	//***************

	VkDescriptorSetLayout guiLayouts[]{ _descriptorSetLayouts[DESC_LYT_Object], _descriptorSetLayouts[DESC_LYT_OneSampler] };
	pipelineLayoutInfo.pSetLayouts = guiLayouts;
	pipelineLayoutInfo.setLayoutCount = 2;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (gui)");
		return ENGINE_PIPELINE_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "gui");
	_pipelineLayouts.insert(make_pair(PIPE_LYT_GUI, layout));

	//*************************
	//* Post process pipelines
	//*************************

	VkDescriptorSetLayout ppLayouts[]{ _descriptorSetLayouts[DESC_LYT_PostProcess] };
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
	VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "postprocess");
	_pipelineLayouts.insert(make_pair(PIPE_LYT_PostProcess, layout));

	VkDescriptorSetLayout blurLayouts[]{ _descriptorSetLayouts[DESC_LYT_PostProcess], _descriptorSetLayouts[DESC_LYT_OneSampler] };
	pipelineLayoutInfo.setLayoutCount = 2;
	pipelineLayoutInfo.pSetLayouts = blurLayouts;

	if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (blur)");
		return ENGINE_PIPELINE_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "blur");
	_pipelineLayouts.insert(make_pair(PIPE_LYT_Blur, layout));

	VkDescriptorSetLayout dofLayouts[]{ _descriptorSetLayouts[DESC_LYT_PostProcess], _descriptorSetLayouts[DESC_LYT_OneSampler], _descriptorSetLayouts[DESC_LYT_OneSampler] };
	pipelineLayoutInfo.setLayoutCount = 3;
	pipelineLayoutInfo.pSetLayouts = dofLayouts;

	if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (dof)");
		return ENGINE_PIPELINE_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "dof");
	_pipelineLayouts.insert(make_pair(PIPE_LYT_DoF, layout));

	//***********************
	//* Shadow map pipelines
	//***********************

	VkDescriptorSetLayout shadowMapLayouts[]{ _descriptorSetLayouts[DESC_LYT_ShadowMap], _descriptorSetLayouts[DESC_LYT_Object] };
	pipelineLayoutInfo.setLayoutCount = 2;
	pipelineLayoutInfo.pSetLayouts = shadowMapLayouts;

	VkPushConstantRange shadowMapRange{};
	shadowMapRange.offset = 0;
	shadowMapRange.size = sizeof(uint32_t);
	shadowMapRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &shadowMapRange;

	if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (shadow map)");
		return ENGINE_PIPELINE_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "shadow map");
	_pipelineLayouts.insert(make_pair(PIPE_LYT_Shadow, layout));

	shadowMapLayouts[1] = _descriptorSetLayouts[DESC_LYT_Anim_Object];

	if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (animated shadow map)");
		return ENGINE_PIPELINE_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "animated shadow map");
	_pipelineLayouts.insert(make_pair(PIPE_LYT_Anim_Shadow, layout));

	shadowMapLayouts[0] = _descriptorSetLayouts[DESC_LYT_ShadowFilter];
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &blurRange;

	if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (shadow map filter)");
		return ENGINE_PIPELINE_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "shadow map filter");
	_pipelineLayouts.insert(make_pair(PIPE_LYT_ShadowFilter, layout));

	//***************************
	//* Particle system pipeline
	//***************************

	VkDescriptorSetLayout psLayouts[]{ _descriptorSetLayouts[DESC_LYT_ParticleDraw] };
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = psLayouts;

	VkPushConstantRange psRange{};
	psRange.offset = 0;
	psRange.size = sizeof(int32_t);
	psRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &psRange;

	if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (particle system draw)");
		return ENGINE_PIPELINE_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "particle system draw");
	_pipelineLayouts.insert(make_pair(PIPE_LYT_ParticleDraw, layout));

	return ENGINE_OK;
}

int PipelineManager::_CreateDescriptorSetLayouts()
{
	//***********************
	//* 1. Scene descriptors
	//***********************
	VkDescriptorSetLayout dsl{};

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

	VkDescriptorSetLayoutBinding shadowMatricesBinding{};
	shadowMatricesBinding.binding = 4;
	shadowMatricesBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	shadowMatricesBinding.descriptorCount = 1;
	shadowMatricesBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	shadowMatricesBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding shadowMap{};
	shadowMap.binding = 5;
	shadowMap.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	shadowMap.descriptorCount = 1;
	shadowMap.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	shadowMap.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding wsNormalMap{};
	wsNormalMap.binding = 6;
	wsNormalMap.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	wsNormalMap.descriptorCount = 1;
	wsNormalMap.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	wsNormalMap.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding sceneLayouts[7]{ sceneDataBlockBinding, lightBlockBinding, visibleIndicesBlockBinding, aoBinding, shadowMatricesBinding, shadowMap, wsNormalMap };

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 7;
	layoutInfo.pBindings = sceneLayouts;

	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (scene)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "scene");
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
	VK_DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "object");
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
	VK_DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "bones");
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
	VK_DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "one sampler");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_OneSampler, dsl));

	// Two samplers
	++samplerBinding.descriptorCount;
	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (two samplers)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "two samplers");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_TwoSamplers, dsl));

	// Three samplers
	++samplerBinding.descriptorCount;
	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (three samplers)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "three samplers");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_ThreeSamplers, dsl));

	// Four samplers
	++samplerBinding.descriptorCount;
	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (four samplers)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "four samplers");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_FourSamplers, dsl));

	//*****************************
	//* 3. PostProcess descriptors
	//*****************************

	VkDescriptorSetLayoutBinding iaBindings[2]{};
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
	VK_DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "post process");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_PostProcess, dsl));

	//****************************
	//* 4. Shadow map descriptors
	//****************************

	VkDescriptorSetLayoutBinding smMatricesBinding{};
	smMatricesBinding.binding = 0;
	smMatricesBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	smMatricesBinding.descriptorCount = 1;
	smMatricesBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	smMatricesBinding.pImmutableSamplers = nullptr;

	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &smMatricesBinding;

	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (shadow map matrices)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "shadow map matrices");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_ShadowMap, dsl));

	VkDescriptorSetLayoutBinding smfTextureBinding{};
	smfTextureBinding.binding = 0;
	smfTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	smfTextureBinding.descriptorCount = 1;
	smfTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	smfTextureBinding.pImmutableSamplers = nullptr;

	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &smfTextureBinding;
	
	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (shadow map filter)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "shadow map filter");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_ShadowFilter, dsl));

	//*********************************
	//* 5. Particle system descriptors
	//*********************************

	VkDescriptorSetLayoutBinding psBindings[2]{};
	psBindings[0].binding = 0;
	psBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	psBindings[0].descriptorCount = 1;
	psBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;
	psBindings[0].pImmutableSamplers = nullptr;
	psBindings[1].binding = 1;
	psBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	psBindings[1].descriptorCount = 1;
	psBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	psBindings[1].pImmutableSamplers = nullptr;	

	layoutInfo.bindingCount = 2;
	layoutInfo.pBindings = psBindings;

	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (particle system draw)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "particle system draw");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_ParticleDraw, dsl));

	return ENGINE_OK;
}

int PipelineManager::_LoadComputeShaders()
{
	ShaderModule *module = nullptr;

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_light_culling", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Compute_Culling, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_particle_update", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Compute_ParticleUpdate, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_particle_sort", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Compute_ParticleSort, module));

	if ((module = (ShaderModule *)ResourceManager::GetResourceByName("sh_particle_emit", ResourceType::RES_SHADERMODULE)) == nullptr)
		return ENGINE_FAIL;
	_shaderModules.insert(make_pair(SH_Compute_ParticleEmit, module));

	return ENGINE_OK;
}

int PipelineManager::_CreateComputePipelines()
{
	VkPipeline pipeline{};

	VkComputePipelineCreateInfo pipelineInfo{};
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
	VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "culling");
	_pipelines.insert(make_pair(PIPE_Culling, pipeline));

	pipelineInfo.stage.module = _shaderModules[SH_Compute_ParticleUpdate]->GetHandle();
	pipelineInfo.layout = _pipelineLayouts[PIPE_LYT_ParticleCompute];
	pipelineInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

	if (vkCreateComputePipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create compute pipeline (particle update)");
		return ENGINE_PIPELINE_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "particle update");
	_pipelines.insert(make_pair(PIPE_ParticleUpdate, pipeline));

	pipelineInfo.basePipelineHandle = pipeline;
	pipelineInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
	pipelineInfo.stage.module = _shaderModules[SH_Compute_ParticleSort]->GetHandle();

	if (vkCreateComputePipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create compute pipeline (particle sort)");
		return ENGINE_PIPELINE_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "particle sort");
	_pipelines.insert(make_pair(PIPE_ParticleSort, pipeline));

	pipelineInfo.stage.module = _shaderModules[SH_Compute_ParticleEmit]->GetHandle();

	if (vkCreateComputePipelines(VKUtil::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, VKUtil::GetAllocator(), &pipeline) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create compute pipeline (particle emit)");
		return ENGINE_PIPELINE_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "particle emit");
	_pipelines.insert(make_pair(PIPE_ParticleEmit, pipeline));

	return ENGINE_OK;
}

int PipelineManager::_CreateComputePipelineLayouts()
{
	VkPipelineLayout layout{ VK_NULL_HANDLE };

	VkDescriptorSetLayout lightCullingLayouts[]{ _descriptorSetLayouts[DESC_LYT_Culling] };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
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
	VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "culling");
	_pipelineLayouts.insert(make_pair(PIPE_LYT_Culling, layout));

	VkDescriptorSetLayout particleLayouts[]{ _descriptorSetLayouts[DESC_LYT_ParticleCompute] };

	pipelineLayoutInfo.pSetLayouts = particleLayouts;

	if (vkCreatePipelineLayout(VKUtil::GetDevice(), &pipelineLayoutInfo, VKUtil::GetAllocator(), &layout) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create pipeline layout (particles)");
		return ENGINE_PIPELINE_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, "particles");
	_pipelineLayouts.insert(make_pair(PIPE_LYT_ParticleCompute, layout));

	return ENGINE_OK;
}

int PipelineManager::_CreateComputeDescriptorSetLayouts()
{
	VkDescriptorSetLayout dsl{ VK_NULL_HANDLE };

	VkDescriptorSetLayoutBinding lightBufferBinding{};
	lightBufferBinding.binding = 0;
	lightBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	lightBufferBinding.descriptorCount = 1;
	lightBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	lightBufferBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding visibleIndicesBlockBinding{};
	visibleIndicesBlockBinding.binding = 1;
	visibleIndicesBlockBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	visibleIndicesBlockBinding.descriptorCount = 1;
	visibleIndicesBlockBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	visibleIndicesBlockBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding dataBlockBinding{};
	dataBlockBinding.binding = 2;
	dataBlockBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	dataBlockBinding.descriptorCount = 1;
	dataBlockBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	dataBlockBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding depthMapBinding{};
	depthMapBinding.binding = 3;
	depthMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	depthMapBinding.descriptorCount = 1;
	depthMapBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	depthMapBinding.pImmutableSamplers = nullptr;

	array<VkDescriptorSetLayoutBinding, 4> layouts{ { lightBufferBinding, visibleIndicesBlockBinding, dataBlockBinding, depthMapBinding } };

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = (uint32_t)layouts.size();
	layoutInfo.pBindings = layouts.data();

	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (light culling)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "culling");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_Culling, dsl));

	VkDescriptorSetLayoutBinding particleBufferBinding{};
	particleBufferBinding.binding = 0;
	particleBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	particleBufferBinding.descriptorCount = 1;
	particleBufferBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	particleBufferBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding drawCommandBinding{};
	drawCommandBinding.binding = 1;
	drawCommandBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	drawCommandBinding.descriptorCount = 1;
	drawCommandBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	drawCommandBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding emitterDataBinding{};
	emitterDataBinding.binding = 2;
	emitterDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	emitterDataBinding.descriptorCount = 1;
	emitterDataBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	emitterDataBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding noiseTextureBinding{};
	noiseTextureBinding.binding = 3;
	noiseTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	noiseTextureBinding.descriptorCount = 1;
	noiseTextureBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	noiseTextureBinding.pImmutableSamplers = nullptr;

	array<VkDescriptorSetLayoutBinding, 4> particleLayouts{ { particleBufferBinding, drawCommandBinding, emitterDataBinding, noiseTextureBinding } };
	layoutInfo.bindingCount = (uint32_t)particleLayouts.size();
	layoutInfo.pBindings = particleLayouts.data();

	if (vkCreateDescriptorSetLayout(VKUtil::GetDevice(), &layoutInfo, VKUtil::GetAllocator(), &dsl) != VK_SUCCESS)
	{
		Logger::Log(PLMGR_MODULE, LOG_CRITICAL, "Failed to create descriptor set layout (particles)");
		return ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL;
	}
	VK_DBG_SET_OBJECT_NAME((uint64_t)dsl, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, "particles");
	_descriptorSetLayouts.insert(make_pair(DESC_LYT_ParticleCompute, dsl));

	return ENGINE_OK;
}

void PipelineManager::_DestroyPipelines()
{
	for (pair<uint8_t, VkPipeline> kvp : _pipelines)
		vkDestroyPipeline(VKUtil::GetDevice(), kvp.second, VKUtil::GetAllocator());
	_pipelines.clear();
}

void PipelineManager::Release()
{
	_DestroyPipelines();

	for (pair<uint8_t, VkPipelineLayout> kvp : _pipelineLayouts)
		vkDestroyPipelineLayout(VKUtil::GetDevice(), kvp.second, VKUtil::GetAllocator());
	_pipelineLayouts.clear();

	for (pair<uint8_t, VkDescriptorSetLayout> kvp : _descriptorSetLayouts)
		vkDestroyDescriptorSetLayout(VKUtil::GetDevice(), kvp.second, VKUtil::GetAllocator());
	_descriptorSetLayouts.clear();
}
