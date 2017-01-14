/* NekoEngine
 *
 * pp_dof.frag
 * Author: Alexandru Naiman
 *
 * Depth of field
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

layout(location = 0) out vec4 o_FragColor;

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput lastPassColor;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput sceneColor;
layout (set = 1, binding = 0) uniform sampler2D colorImage;
layout (set = 2, binding = 0) uniform sampler2DMS depthMap;

layout(push_constant) uniform PushConstants
{
	vec4 effectData;
} pushConstants;

#define PI				3.14159265
#define focalDepth		pushConstants.effectData.x
#define focalLength		pushConstants.effectData.y
#define fStop			pushConstants.effectData.z

const float zNear = 0.1;
const float zFar = 100.0;

const float CoC = 0.03;
const float nAmount = 0.0001;
const float maxBlur = 1.0;
const float bias = 0.5;
const float fringe = 0.7;
const float threshold = 0.5;
const float gain = 2.0;

const int rings = 3;
const int samples = 3;

vec3 getColor(vec2 uv, float blur, vec2 texelSize)
{
	vec3 color = vec3(0.0);

	color.r = texture(colorImage, uv + vec2(0.0, 1.0) * texelSize * fringe * blur).r;
	color.g = texture(colorImage, uv + vec2(-0.866, -0.5) * texelSize * fringe * blur).g;
	color.b = texture(colorImage, uv + vec2(0.866, 0.5) * texelSize * fringe * blur).b;

	vec3 lumCoeff = vec3(0.299, 0.587, 0.114);
	float lum = dot(color.rgb, lumCoeff);
	float thresh = max((lum - threshold) * gain, 0.0);
	return color + mix(vec3(0.0), color, thresh * blur);
}

vec2 rand(vec2 uv)
{
	float noiseX = clamp(fract(sin(dot(uv ,vec2(12.9898,78.233))) * 43758.5453),0.0,1.0)*2.0-1.0;
	float noiseY = clamp(fract(sin(dot(uv ,vec2(12.9898,78.233)*2.0)) * 43758.5453),0.0,1.0)*2.0-1.0;

	return vec2(noiseX, noiseY);
}

void main()
{
	vec2 imageSize = textureSize(colorImage, 0);
	vec2 texelSize = vec2(1.0) / imageSize;
	ivec2 iuv = ivec2(int(v_uv.x * imageSize.x), int(v_uv.y * imageSize.y));
	float depth = -zFar * zNear / (texelFetch(depthMap, iuv, 0).r * (zFar - zNear) - zFar);
	float fDepth = focalDepth;

	float d = fDepth * 1000.0;
	float o = depth * 1000.0;

	float a = (o * focalLength) / (o - focalLength);
	float b = (d * focalLength) / (d - focalLength);
	float c = (d - focalLength) / (d * fStop * CoC);

	float blur = clamp(abs(a - b) * c, 0.0, 1.0);

	vec2 noise = rand(v_uv) * nAmount * blur;
	
	float w = (1.0 / imageSize.x) * blur * maxBlur + noise.x;
	float h = (1.0 / imageSize.y) * blur * maxBlur + noise.y;

	vec3 color = texture(colorImage, v_uv).rgb;

	if(blur > 0.05)
	{
		float s = 1.0;
		int ringSamples;

		for(int i = 0; i <= rings; ++i)
		{
			ringSamples = i * samples;

			for(int j = 0; j < ringSamples; ++j)
			{
				float st = PI * 2.0 / float(ringSamples);
				float pw = cos(float(j) * st) * float(i);
				float ph = sin(float(j) * st) * float(i);
				float p = 1.0;

				color += getColor(v_uv + vec2(pw * w, ph * h), blur, texelSize) * mix(1.0, float(i) / float(rings), bias) * p;
				s += 1.0 * mix(1.0, float(i) / float(rings), bias) * p;
			}
		}

		color /= s;
	}

//	color = subpassLoad(lastPassColor).rgb;


	o_FragColor = vec4(color, 1.0);
	//o_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}