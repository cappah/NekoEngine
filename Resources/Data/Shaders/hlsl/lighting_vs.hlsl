struct PSInput
{
	float4 pos : SV_POSITION;
	float2 v_uv : TEXCOORD0;
};

cbuffer MatrixBlock
{
	float4x4 ModelViewProjection;
};

PSInput main(float3 a_pos : POSITION)
{
	PSInput output;
	output.pos = mul(ModelViewProjection, float4(a_pos, 1.0));
	output.v_uv = (a_pos.xy + 1.0) / 2.0;
	return output;
}