/* NekoEngine
 *
 * pp_hdr.frag
 * Author: Alexandru Naiman
 *
 * HDR post process
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

layout(location = 0) in vec2 v_uv;

layout(location = 0) out vec4 o_FragColor;

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput lastPassColor;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput sceneColor;

layout(push_constant) uniform PushConstants
{
	vec4 effectData;
} pushConstants;

const float exposure = 1.0;
const float gamma = 2.2;

void main()
{
	vec3 color = subpassLoad(sceneColor).rgb;
	vec3 bloomColor = subpassLoad(lastPassColor).rgb;

	color += bloomColor;

	vec3 result = vec3(1.0) - exp(-color * exposure);

	result = pow(result, vec3(1.0 / gamma));
	o_FragColor = vec4(result, 1.0);
}