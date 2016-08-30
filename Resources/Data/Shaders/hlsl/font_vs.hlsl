struct VSInput
{
	float4 a_pos : POSITION;
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

PSInput main(VSInput input)
{
	PSInput output;

	output.v_color = input.a_color;
	output.v_uv = input.a_pos.zw;
	output.pos = mul(u_projection, float4(input.a_pos.xy, 0.0, 1.0));

	return output;
}