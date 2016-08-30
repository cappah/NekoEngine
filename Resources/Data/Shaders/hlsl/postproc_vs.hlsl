struct PSInput
{
	float4 pos : SV_POSITION;
	float2 v_uv : TEXCOORD0;
};

PSInput main(float2 a_pos : POSITION)
{
	PSInput output;

	output.v_uv = (a_pos + 1.0) / 2.0;
	output.pos =  float4(a_pos, 0.0, 1.0);

	return output;
}