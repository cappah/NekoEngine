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

#include <stddef.h>
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
		pos(glm::vec3(0.f)),
		color(glm::vec3(0.f)),
		norm(glm::vec3(0.f)),
		tgt(glm::vec3(0.f)),
		uv(glm::vec2(0.f)),
		terrainUv(glm::vec2(0.f)),
        boneIndices(glm::ivec4(0)),
        boneWeights(glm::vec4(0)),
		numBones(0)
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

	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 norm;
	glm::vec3 tgt;
	glm::vec2 uv;
	glm::vec2 terrainUv;
    glm::ivec4 boneIndices;
    glm::vec4 boneWeights;
	int numBones;
};

struct GUIVertex
{
	glm::vec4 posAndUV;
	glm::vec4 color;
};