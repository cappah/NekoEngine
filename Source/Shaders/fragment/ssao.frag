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
	mat4 inverseProjection;
	vec4 frameAndNoise;
	float kernelSize;
	float radius;
	float powerExponent;
	float threshold;
	float zNear;
	float zFar;
	int numSamples;
	float bias;
	vec4 kernel[128];
} ssaoData;

layout(set = 0, binding = 2) uniform sampler2DMS depthTexture;
layout(set = 0, binding = 3) uniform sampler2DMS normalTexture;
layout(set = 0, binding = 4) uniform sampler2D noiseTexture;

vec3 posFromDepth(vec2 uv, ivec2 iuv)
{
	vec4 projPos = vec4(uv.x * 2.0 - 1.0, uv.y * 2.0 - 1.0, textureAverage(depthTexture, iuv, ssaoData.numSamples).r, 1.0);
	vec4 pos = ssaoData.inverseProjection * projPos;
	return (pos.xyz / pos.w);
}

void main()
{
	ivec2 iuv = ivec2(int(v_uv.x * ssaoData.frameAndNoise.x), int(v_uv.y * ssaoData.frameAndNoise.y)); 
	vec3 fragPos = posFromDepth(v_uv, iuv);

	iuv = ivec2(gl_FragCoord.xy);
	vec3 normal = textureAverage(normalTexture, iuv, ssaoData.numSamples).xyz;
	normal = normalize((vec4(normal, 1.0) * ssaoData.inverseView).xyz);

	vec3 rand = vec3(texture(noiseTexture, v_uv * ssaoData.frameAndNoise.zw).xy, 0.0);
	vec3 tangent = normalize(rand - normal * dot(rand, normal));
	vec3 bitangent = cross(tangent, normal);
	mat3 tbn = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for(int i = 0; i < int(ssaoData.kernelSize); ++i)
	{
		if(dot(ssaoData.kernel[i].xyz, normal) < ssaoData.threshold)
			continue;

		vec3 samplePos = tbn * ssaoData.kernel[i].xyz;
		samplePos = fragPos + samplePos * ssaoData.radius; 

		vec4 offset = vec4(samplePos, 1.0);
		offset = sceneData.projection * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		iuv = ivec2(int(offset.x * ssaoData.frameAndNoise.x), int(offset.y * ssaoData.frameAndNoise.y));
		float sampleDepth = posFromDepth(offset.xy, iuv).z;

		float rangeCheck = smoothstep(0.0, 1.0, ssaoData.radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + ssaoData.bias ? 1.0 : 0.0) * rangeCheck; 
	}

	o_FragColor = pow(1.0 - (occlusion / ssaoData.kernelSize), ssaoData.powerExponent);
}