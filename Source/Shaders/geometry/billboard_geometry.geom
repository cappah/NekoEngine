/* NekoEngine
 *
 * billboard.vert
 * Author: Alexandru Naiman
 *
 * Billboard geometry shader
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

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

layout(set = 0, binding = 0) uniform BillboardData
{
	mat4 viewProjection;
	vec4 cameraPosition;
} billboardData;

layout(location = 0) flat in float g_size[];
layout(location = 1) in vec4 g_color[];
layout(location = 2) flat in uint g_visible[];

layout(location = 0) out vec2 v_uv;
layout(location = 1) out vec4 v_color;

void main()
{
	if (g_visible[0] == 0)
		return;

	float size = g_size[0];
	v_color = g_color[0];

	vec3 pos = gl_in[0].gl_Position.xyz;

	vec3 camera = normalize(billboardData.cameraPosition.xyz - pos);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(camera, up);

	pos -= (right * (size / 2)) - (size / 2);
	gl_Position = billboardData.viewProjection * vec4(pos, 1.0);
	v_uv = vec2(0.0, 0.0);
	EmitVertex();

	pos.y += size;
	gl_Position = billboardData.viewProjection * vec4(pos, 1.0);
	v_uv = vec2(0.0, 1.0);
	EmitVertex();

	pos.y -= size;
	pos += right * size;
	gl_Position = billboardData.viewProjection * vec4(pos, 1.0);
	v_uv = vec2(1.0, 0.0);
	EmitVertex();

	pos.y += size;
	gl_Position = billboardData.viewProjection * vec4(pos, 1.0);
	v_uv = vec2(1.0, 1.0);
	EmitVertex();

	EndPrimitive();
}