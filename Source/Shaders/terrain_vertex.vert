/* NekoEngine
 *
 * terrain_vertex.vert
 * Author: Alexandru Naiman
 *
 * Terrain vertex shader
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

#version 450 core

#define SH_VTX						0
#define SH_VTX_NM					1

layout(set = 0, binding = 0) uniform SceneDataBlock
{
	mat4 view;
	mat4 projection;
	vec4 ambient;
	vec4 cameraPosition;
	ivec2 screenSize;
	int lightCount;
	int numberOfTilesX;
} sceneData;

layout(set = 1, binding = 0) uniform MatrixBlock
{
	mat4 model;
	mat4 modelViewProjection;
	mat4 normal;
} matrixBlock;

layout(constant_id = 0) const int shaderType = 0;

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec3 a_normal;
layout(location = 3) in vec3 a_tangent;
layout(location = 4) in vec2 a_heightmap_uv;

layout(location = 0) out vec2 v_uv;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec3 v_pos;
layout(location = 3) out mat3 v_tbn;

void main()
{
	v_normal = mat3(matrixBlock.normal) * a_normal;
	v_pos = (matrixBlock.model * vec4(a_pos, 1.0)).xyz;
	
	/*if(shaderType == 1)
	{
		vec3 t = normalize(vec3(matrixBlock.model * vec4(a_tangent, 0.0)));
		vec3 n = normalize(v_normal);
		vec3 bitgt = cross(t, n);
		vec3 b = normalize(vec3(matrixBlock.model * vec4(bitgt, 0.0)));

		v_tbn = mat3(t, b, n);
	}*/
	
	v_uv = a_uv;
	gl_Position = matrixBlock.modelViewProjection * vec4(a_pos, 1.0);
}