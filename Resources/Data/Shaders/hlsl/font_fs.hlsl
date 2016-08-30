struct VSInput
{
	float4 a_pos : SV_POSITION;
	float3 a_color : COLOR0;
};

cbuffer DataBlock
{
	float4x4 u_projection;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 v_color : COLOR0;
	float2 v_uv : TEXCOORD0;
};

struct PSOutput
{
	float4 o_FragColor : SV_Target0;
};

Texture2D u_texture : register(t0);
SamplerState u_textureSampler : register(s0);

PSOutput main(PSInput input)
{
	PSOutput output;

	output.o_FragColor = float4(input.v_color, u_texture.Sample(u_textureSampler, input.v_uv).r);

	return output;
}