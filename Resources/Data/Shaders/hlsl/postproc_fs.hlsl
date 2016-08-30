struct PSOutput
{
	float4 o_FragColor : SV_Target0;
	float4 o_ColorTexture : SV_Target1;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 v_uv : TEXCOORD0;
};

cbuffer PPSharedData
{
	float4 SharedData;
};

cbuffer PPEffectData
{
	float4 EffectData;
};

Texture2D PreviousImage : register(t0);
Texture2D OriginalImage : register(t1);
Texture2D BrightnessImage : register(t2);
Texture2D DepthImage : register(t3);
SamplerState sam0 : register(s0);
SamplerState sam1 : register(s1);
SamplerState sam2 : register(s2);
SamplerState sam3 : register(s3);

#define FrameSize SharedData.xy
#define Near SharedData.z
#define Far SharedData.w

PSOutput main(PSInput input)
{
	PSOutput output;

	output.o_FragColor = PreviousImage.Sample(sam0, input.v_uv);
	output.o_ColorTexture = output.o_FragColor;

	return output;
}
