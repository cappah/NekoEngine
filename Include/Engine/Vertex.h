/* NekoEngine
 *
 * Vertex.h
 * Author: Alexandru Naiman
 *
 * Vertex definition 
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

#pragma once

#ifdef ENGINE_INTERNAL
	#include <vulkan/vulkan.h>
	#include <Runtime/Runtime.h>
#endif

#include <stddef.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#define VERTEX_POSITION_OFFSET		offsetof(Vertex, pos)
#define VERTEX_COLOR_OFFSET			offsetof(Vertex, color)
#define VERTEX_NORMAL_OFFSET		offsetof(Vertex, norm)
#define VERTEX_TANGENT_OFFSET		offsetof(Vertex, tgt)
#define VERTEX_UV_OFFSET			offsetof(Vertex, uv)
#define VERTEX_TUV_OFFSET			offsetof(Vertex, terrainUv)
#define VERTEX_INDEX_OFFSET			offsetof(Vertex, boneIndices)
#define VERTEX_WEIGHT_OFFSET		offsetof(Vertex, boneWeights)
#define VERTEX_NUMBONES_OFFSET		offsetof(Vertex, numBones)

struct Vertex 
{
	Vertex() :
		position(glm::vec3(0.f)),
		uv(glm::vec2(0.f)),
		normal(glm::vec3(0.f)),
		tangent(glm::vec3(0.f))
	{ }

	Vertex(glm::vec3 inPos) :
		position(inPos),
		uv(glm::vec2(0.f)),
		normal(glm::vec3(0.f)),
		tangent(glm::vec3(0.f))
	{ }

	/*Vertex(const Vertex& vert) :
		pos(vert.pos),
		color(vert.color),
		norm(vert.norm),
		binorm(vert.binorm),
		tgt(vert.tgt),
		uv(vert.uv),
		terrainUv(vert.terrainUv),
		boneIndices(),
		boneWeights(),
		numBones(vert.numBones)
	{ }*/

	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec3 tangent;

#ifdef ENGINE_INTERNAL
	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription desc = {};

		desc.binding = 0;
		desc.stride = sizeof(Vertex);
		desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return desc;
	}

	static NArray<VkVertexInputAttributeDescription> GetAttributeDescriptions()
	{
		NArray<VkVertexInputAttributeDescription> desc;
		desc.Resize(4);
		desc.Fill();

		desc[0].binding = 0;
		desc[0].location = 0;
		desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		desc[0].offset = offsetof(Vertex, position);

		desc[1].binding = 0;
		desc[1].location = 1;
		desc[1].format = VK_FORMAT_R32G32_SFLOAT;
		desc[1].offset = offsetof(Vertex, uv);

		desc[2].binding = 0;
		desc[2].location = 2;
		desc[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		desc[2].offset = offsetof(Vertex, normal);

		desc[3].binding = 0;
		desc[3].location = 3;
		desc[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		desc[3].offset = offsetof(Vertex, tangent);

		return desc;
	}
#endif
};

struct SkeletalVertex 
{
	SkeletalVertex() :
		position(glm::vec3(0.f)),
		uv(glm::vec2(0.f)),
		normal(glm::vec3(0.f)),
		tangent(glm::vec3(0.f)),
		boneIndices(glm::ivec4(0)),
		boneWeights(glm::vec4(0)),
		numBones(0)
	{ }

	SkeletalVertex(glm::vec3 inPos) :
		position(inPos),
		uv(glm::vec2(0.f)),
		normal(glm::vec3(0.f)),
		tangent(glm::vec3(0.f)),
		boneIndices(glm::ivec4(0)),
		boneWeights(glm::vec4(0)),
		numBones(0)
	{ }

	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::ivec4 boneIndices;
	glm::vec4 boneWeights;
	int32_t numBones;

#ifdef ENGINE_INTERNAL
	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription desc = {};

		desc.binding = 0;
		desc.stride = sizeof(SkeletalVertex);
		desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return desc;
	}

	static NArray<VkVertexInputAttributeDescription> GetAttributeDescriptions()
	{
		NArray<VkVertexInputAttributeDescription> desc;
		desc.Resize(7);
		desc.Fill();

		desc[0].binding = 0;
		desc[0].location = 0;
		desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		desc[0].offset = offsetof(SkeletalVertex, position);

		desc[1].binding = 0;
		desc[1].location = 1;
		desc[1].format = VK_FORMAT_R32G32_SFLOAT;
		desc[1].offset = offsetof(SkeletalVertex, uv);

		desc[2].binding = 0;
		desc[2].location = 2;
		desc[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		desc[2].offset = offsetof(SkeletalVertex, normal);

		desc[3].binding = 0;
		desc[3].location = 3;
		desc[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		desc[3].offset = offsetof(SkeletalVertex, tangent);

		desc[4].binding = 0;
		desc[4].location = 4;
		desc[4].format = VK_FORMAT_R32G32B32A32_SINT;
		desc[4].offset = offsetof(SkeletalVertex, boneIndices);

		desc[5].binding = 0;
		desc[5].location = 5;
		desc[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		desc[5].offset = offsetof(SkeletalVertex, boneWeights);

		desc[6].binding = 0;
		desc[6].location = 6;
		desc[6].format = VK_FORMAT_R32_SINT;
		desc[6].offset = offsetof(SkeletalVertex, numBones);

		return desc;
	}
#endif
};

struct TerrainVertex
{
	TerrainVertex() :
		position(glm::vec3(0.f)),
		uv(glm::vec2(0.f)),
		normal(glm::vec3(0.f)),
		tangent(glm::vec3(0.f)),
		heightmapUV(glm::vec2(0.f))
	{ }

	TerrainVertex(glm::vec3 inPos) :
		position(inPos),
		uv(glm::vec2(0.f)),
		normal(glm::vec3(0.f)),
		tangent(glm::vec3(0.f)),
		heightmapUV(glm::vec2(0.f))
	{ }

	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec2 heightmapUV;

#ifdef ENGINE_INTERNAL
	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription desc = {};

		desc.binding = 0;
		desc.stride = sizeof(TerrainVertex);
		desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return desc;
	}

	static NArray<VkVertexInputAttributeDescription> GetAttributeDescriptions()
	{
		NArray<VkVertexInputAttributeDescription> desc;
		desc.Resize(5);
		desc.Fill();

		desc[0].binding = 0;
		desc[0].location = 0;
		desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		desc[0].offset = offsetof(TerrainVertex, position);

		desc[1].binding = 0;
		desc[1].location = 1;
		desc[1].format = VK_FORMAT_R32G32_SFLOAT;
		desc[1].offset = offsetof(TerrainVertex, uv);

		desc[2].binding = 0;
		desc[2].location = 2;
		desc[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		desc[2].offset = offsetof(TerrainVertex, normal);

		desc[3].binding = 0;
		desc[3].location = 3;
		desc[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		desc[3].offset = offsetof(TerrainVertex, tangent);

		desc[4].binding = 0;
		desc[4].location = 4;
		desc[4].format = VK_FORMAT_R32G32_SFLOAT;
		desc[4].offset = offsetof(TerrainVertex, heightmapUV);

		return desc;
	}
#endif
};
