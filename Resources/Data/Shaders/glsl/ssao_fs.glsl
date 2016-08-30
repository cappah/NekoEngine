layout(location = O_FRAGCOLOR) out float o_FragColor;

in vec2 v_uv;

layout(location=U_TEXTURE0) uniform TEXTURE_2DMS PositionTexture;
layout(location=U_TEXTURE1) uniform TEXTURE_2DMS NormalTexture;
layout(location=U_TEXTURE2) uniform TEXTURE_2D NoiseTexture;

layout(std140) uniform SSAOMatrixBlock
{
	mat4 View;
	mat4 InverseView;
	mat4 Projection;
};

layout(std140) uniform SSAODataBlock
{
	vec4 FrameAndNoise;
	vec4 KernelAndRadius;
	vec4 Kernel[SSAO_MAX_SAMPLES];
};

void main()
{
	ivec2 iuv = ivec2(int(v_uv.x * FrameAndNoise.x), int(v_uv.y * FrameAndNoise.y));

	vec3 fragPos = texelFetch(GET_TEX_2DMS(PositionTexture), iuv, 0).rgb;
	vec3 normal = texelFetch(GET_TEX_2DMS(NormalTexture), iuv, 0).rgb;
	vec3 rand = texture(GET_TEX_2D(NoiseTexture), v_uv * FrameAndNoise.zw).rgb;

	fragPos = (View * vec4(fragPos, 1.0)).xyz;
	normal = normalize((vec4(normal, 1.0) * InverseView).xyz);
	vec3 tgt = normalize(rand - normal * dot(rand, normal));
	vec3 bitgt = cross(normal, tgt);
	mat3 tbn = mat3(tgt, bitgt, normal);

	float occlusion = 0.0;

	for(int i = 0; i < int(KernelAndRadius.x); i++)
	{
		vec3 samplePos = tbn * Kernel[i].xyz;
		samplePos = fragPos + samplePos * KernelAndRadius.y;

		vec4 offset = vec4(samplePos, 1.0);
		offset = Projection * offset;
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5 + 0.5;

		iuv = ivec2(int(offset.x * FrameAndNoise.x), int(offset.y * FrameAndNoise.y));

		vec3 depthPos = texelFetch(GET_TEX_2DMS(PositionTexture), iuv, 0).rgb;
		depthPos = (View * vec4(depthPos, 1.0)).xyz;

		float rangeCheck = smoothstep(0.0, 1.0, KernelAndRadius.y / abs(fragPos.z - depthPos.z));
		occlusion += (depthPos.z >= samplePos.z ? 1.0 : 0.0) * rangeCheck;
	}

	o_FragColor = pow(1.0 - (occlusion / KernelAndRadius.x), 2.0);
}