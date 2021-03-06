/* NekoEngine
 *
 * gui_vertex.vert
 * Author: Alexandru Naiman
 *
 * GUI vertex shader
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

#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform SceneDataBlock
{
	mat4 projection;
} sceneData;

layout(location = 0) in vec4 a_posAndUV;
layout(location = 1) in vec4 a_color;

layout(location = 0) out vec4 v_color;
layout(location = 1) out vec2 v_uv;

void main()
{
	v_color = a_color;
	v_uv = a_posAndUV.zw;
	gl_Position = sceneData.projection * vec4(a_posAndUV.xy, 0.0, 1.0);
}