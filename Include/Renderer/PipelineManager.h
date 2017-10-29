/* NekoEngine
 *
 * PipelineManager.h
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

#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <unordered_map>

#include <Renderer/ShaderModule.h>

enum DescriptorLayoutId : uint8_t
{
	DESC_LYT_Scene,
	DESC_LYT_Object,
	DESC_LYT_OneSampler,
	DESC_LYT_TwoSamplers,
	DESC_LYT_ThreeSamplers,
	DESC_LYT_FourSamplers,
	DESC_LYT_Culling,
	DESC_LYT_Anim_Object,
	DESC_LYT_PostProcess,
	DESC_LYT_ShadowMap,
	DESC_LYT_ShadowFilter,
	DESC_LYT_ParticleCompute,
	DESC_LYT_ParticleDraw,
};

enum PipelineLayoutId : uint8_t
{
	PIPE_LYT_OneSampler = 0,
	PIPE_LYT_TwoSamplers = 1,
	PIPE_LYT_ThreeSamplers = 2,
	PIPE_LYT_FourSamplers = 3,
	PIPE_LYT_Depth = 4,
	PIPE_LYT_Shadow = 5,
	PIPE_LYT_Anim_OneSampler = 10,
	PIPE_LYT_Anim_TwoSamplers = 11,
	PIPE_LYT_Anim_ThreeSamplers = 12,
	PIPE_LYT_Anim_FourSamplers = 13,
	PIPE_LYT_Anim_Depth = 14,
	PIPE_LYT_Anim_Shadow = 15,
	PIPE_LYT_GUI = 100,
	PIPE_LYT_PostProcess = 101,
	PIPE_LYT_Culling = 102,
	PIPE_LYT_Blur = 103,
	PIPE_LYT_DoF = 104,
	PIPE_LYT_FilmGrain = 105,
	PIPE_LYT_ShadowFilter = 190,
	PIPE_LYT_ParticleCompute = 200,
	PIPE_LYT_ParticleDraw = 201,
	PIPE_LYT_Debug = 220,
};

enum PipelineId : uint8_t
{
	PIPE_Unlit = 0,
	PIPE_Phong = 1,
	PIPE_PhongSpecular = 2,
	PIPE_PhongSpecularEmissive = 3,
	PIPE_PhongNormal = 4,
	PIPE_PhongNormalSpecular = 5,
	PIPE_PhongNormalSpecularEmissive = 6,
	PIPE_Depth = 7,
	PIPE_DepthNormal = 8,
	PIPE_Shadow = 9,
	PIPE_Anim_Unlit = 10,
	PIPE_Anim_Phong = 11,
	PIPE_Anim_PhongSpecular = 12,
	PIPE_Anim_PhongSpecularEmissive = 13,
	PIPE_Anim_PhongNormal = 14,
	PIPE_Anim_PhongNormalSpecular = 15,
	PIPE_Anim_PhongNormalSpecularEmissive = 16,
	PIPE_Anim_Depth = 17,
	PIPE_Anim_DepthNormal = 18,
	PIPE_Anim_Shadow = 19,
	PIPE_Transparent_Unlit = 20,
	PIPE_Transparent_Phong = 21,
	PIPE_Transparent_PhongSpecular = 22,
	PIPE_Transparent_PhongSpecularEmissive = 23,
	PIPE_Transparent_PhongNormal = 24,
	PIPE_Transparent_PhongNormalSpecular = 25,
	PIPE_Transparent_PhongNormalSpecularEmissive = 26,
	PIPE_Transparent_Anim_Unlit = 30,
	PIPE_Transparent_Anim_Phong = 31,
	PIPE_Transparent_Anim_PhongSpecular = 32,
	PIPE_Transparent_Anim_PhongSpecularEmissive = 33,
	PIPE_Transparent_Anim_PhongNormal = 34,
	PIPE_Transparent_Anim_PhongNormalSpecular = 35,
	PIPE_Transparent_Anim_PhongNormalSpecularEmissive = 36,
	PIPE_Skysphere = 100,
	PIPE_GUI = 101,
	PIPE_Font = 102,
	PIPE_Culling = 103,
	PIPE_Terrain = 104,
	PIPE_Terrain_Depth = 105,
	PIPE_Terrain_Shadow = 106,
	PIPE_ShadowFilter = 190,
	PIPE_ParticleEmit = 200,
	PIPE_ParticleUpdate = 201,
	PIPE_ParticleSort = 202,
	PIPE_ParticleDraw = 203,
	PIPE_Bounds = 220,
	PIPE_DebugLine = 221
};

class PipelineManager
{
public:
	static int Initialize();

	static VkPipeline GetPipeline(PipelineId id) { return _pipelines[id]; }
	static VkPipelineLayout GetPipelineLayout(PipelineLayoutId layout) { return _pipelineLayouts[layout]; }
	static VkDescriptorSetLayout GetDescriptorSetLayout(DescriptorLayoutId layout) { return _descriptorSetLayouts[layout]; }

	static int RecreatePipelines();

	static void Release();

private:
	static std::unordered_map<uint8_t, VkPipeline> _pipelines;
	static std::unordered_map<uint8_t, VkPipelineLayout> _pipelineLayouts;
	static std::unordered_map<uint8_t, VkDescriptorSetLayout> _descriptorSetLayouts;

	static int _LoadShaders();
	static int _CreatePipelines();
	static int _CreatePipelineLayouts();
	static int _CreateDescriptorSetLayouts();

	static int _LoadComputeShaders();
	static int _CreateComputePipelines();
	static int _CreateComputePipelineLayouts();
	static int _CreateComputeDescriptorSetLayouts();

	static void _DestroyPipelines();
};