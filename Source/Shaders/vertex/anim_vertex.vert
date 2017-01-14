/* NekoEngine
 *
 * vertex_anim.vert
 * Author: Alexandru Naiman
 *
 * Animated vertex shader
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

#version 450 core
#extension GL_GOOGLE_include_directive : require

#define SH_VTX						0
#define SH_VTX_NM					1
#define FS_DATA_OUT					1

#include "fs_data.glh"
#include "scenedata.glh"
#include "animblock.glh"
#include "matrixblock.glh"
#include "vertex_attribs.glh"
#include "anim_vertex_attribs.glh"

layout(constant_id = 0) const int shaderType = 0;

void main()
{
	mat4 boneTransform = animationBlock.bones[a_boneIndices.x] * a_boneWeights.x;
	boneTransform += animationBlock.bones[a_boneIndices.y] * a_boneWeights.y;
	boneTransform += animationBlock.bones[a_boneIndices.z] * a_boneWeights.z;
	boneTransform += animationBlock.bones[a_boneIndices.w] * a_boneWeights.w;
		
	vec4 l_pos = boneTransform * vec4(a_pos, 1.0);
	vec4 new_tangent = boneTransform * vec4(a_tangent, 0.0);
	v_pos = vec3(matrixBlock.model * vec4(l_pos.xyz, 1.0));

	v_uv = a_uv;
	gl_Position = matrixBlock.modelViewProjection * vec4(l_pos.xyz, 1.0);
}