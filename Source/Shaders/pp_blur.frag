/* NekoEngine
 *
 * pp_blur.frag
 * Author: Alexandru Naiman
 *
 * Kawase blur filter
 *
 * ////////////////////////////////////////////////////////////////////////////
 * // Developed by Masaki Kawase, Bunkasha Games
 * // Used in DOUBLE-S.T.E.A.L. (aka Wreckless)
 * // From his GDC2003 Presentation:
 * // Frame Buffer Postprocessing Effects in DOUBLE-S.T.E.A.L (Wreckless)
 * ////////////////////////////////////////////////////////////////////////////
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
layout (set = 1, binding = 0) uniform sampler2D blurColor;

layout(push_constant) uniform PushConstants
{
	vec4 effectData;
} pushConstants;

#define pixelSize	pushConstants.effectData.xy
#define iteration	pushConstants.effectData.z

void main()
{
    vec2 halfPixelSize = pixelSize / 2.0;
    vec2 dUV = (pixelSize.xy * vec2(iteration, iteration)) + halfPixelSize.xy;

	vec2 uvSample;
    vec3 color;

    uvSample = vec2(v_uv.x - dUV.x, v_uv.y + dUV.y);
    color = texture(blurColor, uvSample).xyz;

    uvSample = vec2(v_uv.x + dUV.x, v_uv.y + dUV.y);
    color += texture(blurColor, uvSample).xyz;

    uvSample = vec2(v_uv.x + dUV.x, v_uv.y - dUV.y);
    color += texture(blurColor, uvSample).xyz;

    uvSample = vec2(v_uv.x - dUV.x, v_uv.y - dUV.y);
    color += texture(blurColor, uvSample).xyz;
    
	o_FragColor = vec4(color * 0.25, 1.0);
}