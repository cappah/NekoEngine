struct PSInput
{
	float4 pos : SV_POSITION;
	float2 v_uv : TEXCOORD0;
};

Texture2DMS<float4> PositionTexture : register(t0);
Texture2DMS<float4> NormalTexture : register(t1);
Texture2D NoiseTexture : register(t2);
SamplerState NoiseSampler : register(s2);

cbuffer SSAOMatrixBlock
{
	float4 View;
	float4 InverseView;
	float4 Projection;
};

cbuffer SSAODataBlock
{
	float4 FrameAndNoise;
	float4 KernelAndRadius;
	float4 Kernel[128];
};

float main(PSInput input) : SV_Target0
{
	int2 iuv = int2(int(input.v_uv.x * FrameAndNoise.x), int(input.v_uv.y * FrameAndNoise.y));

	float3 fragPos = PositionTexture.Load(iuv, 0).rgb;
	float3 normal = NormalTexture.Load(iuv, 0).rgb;
	float3 rand = NoiseTexture.Sample(NoiseSampler, input.v_uv * FrameAndNoise.zw).rgb;

	fragPos = mul(View, float4(fragPos, 1.0));
	normal = normalize(mul(float4(normal, 1.0), InverseView));
	float3 tgt = normalize(rand - normal * dot(rand, normal));
	float3 bitgt = cross(normal, tgt);
	float3x3 tbn = float3x3(tgt, bitgt, normal);

	float occlusion = 0.0;

	for(int i = 0; i < int(KernelAndRadius.x); i++)
	{
		float3 samplePos = mul(tbn, Kernel[i].xyz);
		samplePos = fragPos + samplePos * KernelAndRadius.y;

		float4 offset = float4(samplePos, 1.0);
		offset = Projection * offset;
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5 + 0.5;

		iuv = int2(int(offset.x * FrameAndNoise.x), int(offset.y * FrameAndNoise.y));

		float3 depthPos = PositionTexture.Load(iuv, 0).rgb;
		depthPos = mul(View, float4(depthPos, 1.0));

		float rangeCheck = smoothstep(0.0, 1.0, KernelAndRadius.y / abs(fragPos.z - depthPos.z));
		occlusion += (depthPos.z >= samplePos.z ? 1.0 : 0.0) * rangeCheck;
	}

	return pow(1.0 - (occlusion / KernelAndRadius.x), 2.0);
}