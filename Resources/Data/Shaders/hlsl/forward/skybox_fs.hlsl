struct PSOutput
{
	float4 o_FragColor : SV_Target0;
	float4 o_ColorTexture : SV_Target1;
	float4 o_BrightColor : SV_Target2;
};

struct PSInput
{
	float4 a_pos : SV_POSITION;
	float3 uv : TEXCOORD0;
};

cbuffer ObjectBlock
{
	float4 CameraPosition;
	float4 ObjectColor;
};

cbuffer MaterialBlock
{
	float4 MaterialData;
	float4 MaterialData1;
};

#define DiffuseConstant MaterialData.x
#define SpecularConstant MaterialData.y
#define Shininess MaterialData.z
#define Bloom MaterialData.w
#define MaterialType MaterialData1.

TextureCube u_texture_cube : register(t9);
SamplerState u_sampler : register(s9);

PSOutput main(PSInput input)
{
	PSOutput output;
	float4 color;

	color = u_texture_cube.Sample(u_sampler, input.uv);
	color.rgb = pow(color.rgb, float3(2.2, 2.2, 2.2));

	float3 x = max(float3(0.0, 0.0, 0.0), color.rgb - 0.004);
	color.rgb = (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);

	output.o_FragColor = color;
	output.o_ColorTexture = color;

	return output;
}