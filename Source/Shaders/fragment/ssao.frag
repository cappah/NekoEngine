/* NekoEngine
 *
 * ssao.frag
 * Author: Alexandru Naiman
 *
 * Screen-Space Ambient Occlusion
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

#include "util.glh"
#include "normal.glh"

layout(location = 0) in vec2 v_uv;

layout(location = 0) out float o_FragColor;

layout(set = 0, binding = 0) uniform SceneDataBlock
{
	mat4 view;
	mat4 projection;
} sceneData;

layout(set = 0, binding = 1) uniform SSAODataBlock
{
	mat4 inverseView;
	mat4 inverseViewProjection;
	vec4 frameAndNoise;
	float kernelSize;
	float radius;
	float powerExponent;
	float threshold;
	float zNear;
	float zFar;
	int numSamples;
	float p0;
	vec4 kernel[128];
} ssaoData;

layout(set = 0, binding = 2) uniform sampler2DMS depthTexture;
layout(set = 0, binding = 3) uniform sampler2DMS normalTexture;
layout(set = 0, binding = 4) uniform sampler2D noiseTexture;

vec3 posFromDepth(vec2 uv, ivec2 iuv)
{
	float z = ssaoData.zNear / (ssaoData.zFar - textureAverage(depthTexture, iuv, ssaoData.numSamples).r * (ssaoData.zFar - ssaoData.zNear)) * ssaoData.zFar;
	vec4 pos = vec4(uv * 2.0 - 1.0, z, 1.0);
	pos = ssaoData.inverseViewProjection * pos;
	return (pos.xyz / pos.w);
}

void main()
{
	ivec2 iuv = ivec2(int(v_uv.x * ssaoData.frameAndNoise.x), int(v_uv.y * ssaoData.frameAndNoise.y));
	ivec2 normalIUV = ivec2(gl_FragCoord.xy);

	vec3 fragPos = posFromDepth(v_uv, iuv);
	vec3 normal = decodeNormal(textureAverage(normalTexture, normalIUV, ssaoData.numSamples).xy);
	//decodeNormal(textureAverage(wsNormalMap, location, sceneData.numSamples).xy);

	vec3 rand = texture(noiseTexture, v_uv * ssaoData.frameAndNoise.zw).rgb;

	fragPos = (sceneData.view * vec4(fragPos, 1.0)).xyz;
	normal = normalize((vec4(normal, 1.0) * ssaoData.inverseView).xyz);
	vec3 tgt = normalize(rand - normal * dot(rand, normal));
	vec3 bitgt = cross(normal, tgt);
	mat3 tbn = mat3(tgt, bitgt, normal);

	float occlusion = 0.0;

	for(int i = 0; i < int(ssaoData.kernelSize); i++)
	{
		if(dot(ssaoData.kernel[i].xyz, normal) < ssaoData.threshold)
			continue;

		vec3 samplePos = tbn * ssaoData.kernel[i].xyz;
		samplePos = fragPos + samplePos * ssaoData.radius;

		vec4 offset = vec4(samplePos, 1.0);
		offset = sceneData.projection * offset;
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5 + 0.5;

		iuv = ivec2(int(offset.x * ssaoData.frameAndNoise.x), int(offset.y * ssaoData.frameAndNoise.y));

		vec3 depthPos = posFromDepth(offset.xy, iuv);
		depthPos = (sceneData.view * vec4(depthPos, 1.0)).xyz;

		float rangeCheck = smoothstep(0.0, 1.0, ssaoData.radius / abs(fragPos.z - depthPos.z));
		occlusion += (depthPos.z >= samplePos.z ? 0.0 : 1.0) * rangeCheck;
	}

	o_FragColor = pow(1.0 - (occlusion / ssaoData.kernelSize), ssaoData.powerExponent);
}