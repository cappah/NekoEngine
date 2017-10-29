/* NekoEngine
 *
 * phong.frag
 * Author: Alexandru Naiman
 *
 * Blinn-Phong shader
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

#define SH_FRAG_UNLIT				0
#define SH_FRAG_PHONG				1
#define SH_FRAG_PHONG_SPEC			2
#define SH_FRAG_PHONG_SPEC_EM		3
#define SH_FRAG_PHONG_NM			4
#define SH_FRAG_PHONG_SPEC_NM		5
#define SH_FRAG_PHONG_SPEC_EM_NM	6

// Output
layout(location = 0) out vec4 o_FragColor;
layout(location = 1) out vec4 o_Brightness;

#define SHADOW_MATRICES_BINDING		4

#include "util.glh"
#include "fs_data.glh"
#include "scenedata.glh"
#include "matrixblock.glh"
#include "lightbuffers.glh"
#include "shadowmatrices.glh"

layout(set = 0, binding = 3) uniform sampler2D aoMap;
layout(set = 0, binding = 5) uniform sampler2DArray shadowMap;
layout(set = 0, binding = 6) uniform sampler2DMS wsNormalMap;

layout(push_constant) uniform PushConstants
{
	vec4 diffuse;
	vec4 specular;
	vec4 emission;
	float shininess;
	float ior;
	float bloom;
	int type;
} pushConstants;

layout(constant_id = 10) const int shaderType = 1;
layout(constant_id = 11) const int numTextures = 5;

layout(set = 2, binding = 0) uniform sampler2D textures[numTextures];

const uvec2 mask_pos[5] = { { 0xFC000000, 26 }, { 0x03F00000, 20 }, { 0x000FC000, 14 }, { 0x00003F00, 8 }, { 0x000000FC, 2 } };

#define diffuseMap textures[0]
#define normalMap textures[1]

float attenuate(vec3 lightDirection, float radius)
{
	float cutoff = 0.5;
	float attenuation = dot(lightDirection, lightDirection) / (100.0 * radius);
	attenuation = 1.0 / (attenuation * 15.0 + 1.0);
	attenuation = (attenuation - cutoff) / (1.0 - cutoff);

	return clamp(attenuation, 0.0, 1.0);
}

float linstep(float low, float high, float val)
{
	return clamp((val - low) / (high - low), 0.0, 1.0);
}

float getShadowMapId(uint mapId, float v)
{
	uint ret = uint(v) & mask_pos[mapId].x;
	return float(ret >> mask_pos[mapId].y);
}

float calculateShadow(uint lightIndex)
{
	float shadowMapId = getShadowMapId(0, lightBuffer.data[lightIndex].direction.w);

	vec4 lightSpacePos = shadowMatrices.data[int(shadowMapId)] * vec4(v_pos, 1.0);
	vec3 shadowCoords = (lightSpacePos.xyz / lightSpacePos.w);

	vec2 moments = texture(shadowMap, vec3(shadowCoords.xy, shadowMapId)).rg;
	
	float p = step(shadowCoords.z, moments.x);
	float var = max(moments.y - moments.x * moments.x, 0.000002);

	float d = shadowCoords.z - moments.x;
	//float pMax = linstep(0.2, 1.0, var / (var + d * d));
	float pMax = smoothstep(0.2, 1.0, var / (var + d * d));

	return min(max(p, pMax), 1.0);	
}

float calculateDirectionalShadow(uint lightIndex)
{
	// TODO: Cascaded
	float shadowMapId = getShadowMapId(0, lightBuffer.data[lightIndex].direction.w);

	vec4 lightSpacePos = shadowMatrices.data[int(shadowMapId)] * vec4(v_pos, 1.0);
	vec3 shadowCoords = (lightSpacePos.xyz / lightSpacePos.w);

	vec2 moments = texture(shadowMap, vec3(shadowCoords.xy, shadowMapId)).rg;
	
	float p = step(shadowCoords.z, moments.x);
	float var = max(moments.y - moments.x * moments.x, 0.000002);

	float d = shadowCoords.z - moments.x;
	//float pMax = linstep(0.2, 1.0, var / (var + d * d));
	float pMax = smoothstep(0.2, 1.0, var / (var + d * d));

	return min(max(p, pMax), 1.0);	
}

void main()
{
	vec4 color = texture(diffuseMap, v_uv);
	color.rgb *= pushConstants.diffuse.rgb;
	color.rgb = pow(color.rgb, vec3(sceneData.gamma));

	// AO debugging
	/*o_FragColor.rgb = vec3(texture(aoMap, gl_FragCoord.xy / vec2(sceneData.screenSize)).r);
	o_FragColor.a = 1.0;

	return;*/

	if (shaderType == SH_FRAG_UNLIT)
	{
		o_FragColor = color;
		return;
	}

	ivec2 location = ivec2(gl_FragCoord.xy);
	ivec2 tile = location / ivec2(16, 16);
	uint index = tile.y * int(sceneData.numberOfTilesX) + tile.x;

	vec3 specularColor;
	vec3 normal = textureAverage(wsNormalMap, location, sceneData.numSamples).xyz;

	if (shaderType == SH_FRAG_PHONG_SPEC || shaderType == SH_FRAG_PHONG_SPEC_EM || shaderType == SH_FRAG_PHONG_SPEC_NM || shaderType == SH_FRAG_PHONG_SPEC_EM_NM)
		specularColor = texture(textures[1], v_uv).rgb;
	else
		specularColor = pushConstants.specular.rgb;

	vec3 ambient = sceneData.ambient.rgb * vec3(sceneData.ambient.a) * color.rgb;
	vec3 emissionColor = pushConstants.emission.rgb;

	if (shaderType == SH_FRAG_PHONG_SPEC_EM || shaderType == SH_FRAG_PHONG_SPEC_EM_NM)
		emissionColor = vec3(texture(textures[2], v_uv));

	vec3 lightColor = vec3(0.0);
	uint offset = index * 1024;	
	for (uint i = 0; i < 1024 && visibleIndicesBuffer.data[offset + i] != -1; ++i)
	{
		uint lightIndex = visibleIndicesBuffer.data[offset + i];
		vec3 lightDirection = vec3(0.0);
		vec3 viewDirection = normalize(sceneData.cameraPosition.xyz - v_pos);
		vec3 light = lightBuffer.data[lightIndex].color.rgb * lightBuffer.data[lightIndex].color.a;
		float attenuation = 1.0, spotAttenuation = 1.0, shadow = 1.0;

		uint lightType = uint(lightBuffer.data[lightIndex].position.w);
		if (lightType == LT_DIRECTIONAL)
		{
			lightDirection = normalize(-lightBuffer.data[lightIndex].direction.xyz);
			if (lightBuffer.data[lightIndex].direction.w > -1.0)
				shadow = calculateDirectionalShadow(lightIndex);
		}
		else if (lightType == LT_POINT)
		{
			lightDirection = lightBuffer.data[lightIndex].position.xyz - v_pos;
			float d = length(lightDirection);
			attenuation = smoothstep(lightBuffer.data[lightIndex].data.y, lightBuffer.data[lightIndex].data.x, d);

			lightDirection = normalize(lightDirection);

			if (lightBuffer.data[lightIndex].direction.w > -1.0)
				shadow = calculateShadow(lightIndex);
		}
		else if (lightType == LT_SPOT)
		{
			lightDirection = lightBuffer.data[lightIndex].position.xyz - v_pos;
			float d = length(lightDirection);
			attenuation = smoothstep(lightBuffer.data[lightIndex].data.y, lightBuffer.data[lightIndex].data.x, d);
			lightDirection = normalize(lightDirection);

			float innerCutoff = lightBuffer.data[lightIndex].data.z;
			float outerCutoff = lightBuffer.data[lightIndex].data.w;

			float e = innerCutoff - outerCutoff;
			float theta = dot(lightDirection, normalize(-lightBuffer.data[lightIndex].direction.xyz));
			spotAttenuation = smoothstep(0.0, 1.0, (theta - outerCutoff) / e);

			if (theta < lightBuffer.data[lightIndex].data.w)
				continue;

			if (lightBuffer.data[lightIndex].direction.w > -1.0)
				shadow = calculateShadow(lightIndex);
		}		

		float diffuseCoef = max(dot(normal, lightDirection), 0.0);

		if(diffuseCoef <= 0.0)
			continue;

		vec3 diffuse = light * diffuseCoef * color.rgb;

		vec3 halfVec = normalize(lightDirection + viewDirection);
		float specularCoef = pow(max(dot(normal, halfVec), 0.0), pushConstants.shininess);

		vec3 specular = light * specularCoef * specularColor;

		lightColor += shadow * (diffuse + specular) * attenuation * spotAttenuation;
	}

	o_FragColor = vec4(vec3(ambient + lightColor + emissionColor) * vec3(texture(aoMap, (gl_FragCoord.xy / vec2(sceneData.screenSize))).r), color.a);

	float brightness = dot(o_FragColor.rgb, vec3(0.2126, 0.7152, 0.0722)) * pushConstants.bloom;
    if(brightness > 1.3) o_Brightness = vec4(o_FragColor.rgb, 1.0);
}