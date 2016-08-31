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

cbuffer shader_data
{
	float2 FrameSize;
};

Texture2D LoadScreenTexture : register(t0);
SamplerState sam : register(s0);

PSOutput main(PSInput input)
{
	PSOutput output;
	float2 uv = float2(input.v_uv.x, 1.0 - input.v_uv.y);

	output.o_FragColor = LoadScreenTexture.Sample(sam, uv);
	output.o_ColorTexture = output.o_FragColor;

	return output;
}
