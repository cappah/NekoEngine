/* NekoEngine
 *
 * ssao.frag
 * Author: Alexandru Naiman
 *
 * Screen-Space Ambient Occlusion Blur
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

layout(location = 0) in vec2 v_uv;

layout(location = 0) out float o_FragColor;

layout(set = 0, binding = 0) uniform sampler2D aoTexture;

const float offsets[4] = { -2.0, -1.0, 0.0, 1.0 };

void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(aoTexture, 0));
	float color = 0.0;
	vec2 offset = vec2(0.0);

	offset = vec2(offsets[0], offsets[0]) * texelSize;
	color += texture(aoTexture, v_uv + offset).r;

	offset = vec2(offsets[0], offsets[1]) * texelSize;
	color += texture(aoTexture, v_uv + offset).r;

	offset = vec2(offsets[0], offsets[2]) * texelSize;
	color += texture(aoTexture, v_uv + offset).r;

	offset = vec2(offsets[0], offsets[3]) * texelSize;
	color += texture(aoTexture, v_uv + offset).r;


	offset = vec2(offsets[1], offsets[0]) * texelSize;
	color += texture(aoTexture, v_uv + offset).r;

	offset = vec2(offsets[1], offsets[1]) * texelSize;
	color += texture(aoTexture, v_uv + offset).r;

	offset = vec2(offsets[1], offsets[2]) * texelSize;
	color += texture(aoTexture, v_uv + offset).r;

	offset = vec2(offsets[1], offsets[3]) * texelSize;
	color += texture(aoTexture, v_uv + offset).r;


	offset = vec2(offsets[2], offsets[0]) * texelSize;
	color += texture(aoTexture, v_uv + offset).r;

	offset = vec2(offsets[2], offsets[1]) * texelSize;
	color += texture(aoTexture, v_uv + offset).r;

	offset = vec2(offsets[2], offsets[2]) * texelSize;
	color += texture(aoTexture, v_uv + offset).r;

	offset = vec2(offsets[2], offsets[3]) * texelSize;
	color += texture(aoTexture, v_uv + offset).r;


	offset = vec2(offsets[3], offsets[0]) * texelSize;
	color += texture(aoTexture, v_uv + offset).r;

	offset = vec2(offsets[3], offsets[1]) * texelSize;
	color += texture(aoTexture, v_uv + offset).r;

	offset = vec2(offsets[3], offsets[2]) * texelSize;
	color += texture(aoTexture, v_uv + offset).r;

	offset = vec2(offsets[3], offsets[3]) * texelSize;
	color += texture(aoTexture, v_uv + offset).r;

	o_FragColor = color / 16.0;
}