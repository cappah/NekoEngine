/* Neko Engine
 *
 * Vertex.h
 * Author: Alexandru Naiman
 *
 * Vertex definition 
 *
 * ----------------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman <alexandru dot naiman at icloud dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ALEXANDRU NAIMAN
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <glm/glm.hpp>

#define VERTEX_POSITION_OFFSET		0
#define VERTEX_COLOR_OFFSET		sizeof(glm::vec3)
#define VERTEX_NORMAL_OFFSET		(sizeof(glm::vec3) + sizeof(glm::vec3))
#define VERTEX_BINORMAL_OFFSET		(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3))
#define VERTEX_TANGENT_OFFSET		(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3))
#define VERTEX_UV_OFFSET		(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3))
#define VERTEX_TUV_OFFSET		(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2))
#define VERTEX_INDEX_OFFSET		(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec2))
#define VERTEX_WEIGHT_OFFSET		(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec2) + sizeof(glm::vec4))
#define VERTEX_NUMBONES_OFFSET		(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec2) + sizeof(glm::vec4) + sizeof(glm::vec4))

struct Vertex 
{
	Vertex() :
		pos(glm::vec3(0.f)),
		color(glm::vec3(0.f)),
		norm(glm::vec3(0.f)),
		binorm(glm::vec3(0.f)),
		tgt(glm::vec3(0.f)),
		uv(glm::vec2(0.f)),
		terrainUv(glm::vec2(0.f)),
		index(glm::vec4(0.f)),
		weight(glm::vec4(0.f)),
		numBones(0.f)
	{ }

	Vertex(const Vertex& vert) :
		pos(vert.pos),
		color(vert.color),
		norm(vert.norm),
		binorm(vert.binorm),
		tgt(vert.tgt),
		uv(vert.uv),
		terrainUv(vert.terrainUv),
		index(vert.index),
		weight(vert.weight),
		numBones(vert.numBones)
	{ }

	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 norm;
	glm::vec3 binorm;
	glm::vec3 tgt;
	glm::vec2 uv;
	glm::vec2 terrainUv;
	glm::vec4 index;
	glm::vec4 weight;
	float numBones;
};
