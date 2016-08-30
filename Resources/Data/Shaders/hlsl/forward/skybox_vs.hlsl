struct PSInput
{
	float4 a_pos : SV_POSITION;
	float3 uv : TEXCOORD0;
};

cbuffer MatrixBlock
{
	float4 ModelViewProjection;
	float4 Model;
	float4 View;
};

PSInput main(float3 a_pos : POSITION)
{
	PSInput output;

	float4 pos = mul(ModelViewProjection, float4(a_pos, 1.0));
	output.uv = a_pos.xyz;
	output.a_pos = pos;

	return output;
}
