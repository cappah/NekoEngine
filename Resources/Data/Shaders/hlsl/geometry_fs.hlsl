struct PSOutput
{
	float4 o_Position : SV_Target0;
	float4 o_Normal : SV_Target1;
	float4 o_ColorSpecular : SV_Target2;
	float4 o_MaterialInfo : SV_Target3;
};

struct PSInput
{
	float4 a_pos : SV_POSITION;
	float2 UV : TEXCOORD0;
	float2 TerrainUV : TEXCOORD1;
	float3 Position : POSITION0;
	float3 Normal : NORMAL0;
	float3 Color : COLOR0;
	float3 Tangent : TANGENT0;
	float3 CubemapUV : TEXCOORD2;
	float3 ViewSpacePosition : POSITION1;
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
#define MaterialType MaterialData1.x
#define ShaderType MaterialData1.x
#define NoDiscard MaterialData1.y

#define AnimatedMesh MaterialData1.z

Texture2D u_texture0 : register(t0);
Texture2D u_texture1 : register(t1);
Texture2D u_texture2 : register(t2);
Texture2D u_texture3 : register(t3);
TextureCube u_texture_cube : register(t9);

SamplerState u_sampler0 : register(s0);
SamplerState u_sampler1 : register(s1);
SamplerState u_sampler2 : register(s2);
SamplerState u_sampler3 : register(s3);
SamplerState u_sampler_cube : register(s9);

float3 calculateMappedNormal(PSInput input)
{
	float3 norm = normalize(input.Normal);
	float3 tgt = normalize(input.Tangent);
	tgt = normalize(tgt - dot(tgt, norm) * norm);
	float3 bitgt = cross(tgt, norm);

	float4 norm_tex = u_texture1.Sample(u_sampler1, input.UV);
	float3 map_norm = norm_tex.xyz * 2.0 - float3(1.0, 1.0, 1.0);

	float3x3 tbn = float3x3(tgt, bitgt, norm);
	float3 new_norm = mul(tbn, normalize(map_norm)); //normalize(tbn * map_norm);

	return new_norm;
}

float4 setColorSpecularMap(PSInput input)
{
	float4 ret;
	
	ret = u_texture0.Sample(u_sampler0, input.UV);

	ret.a += NoDiscard;
	if(ret.a < 0.1)
		discard;

	ret.a = u_texture2.Sample(u_sampler2, input.UV).r;
	ret.rgb = pow(ret.rgb, float3(2.2, 2.2, 2.2));
	ret.rgb += ObjectColor.xyz;

	return ret;
}

float4 setColorSpecularArg(PSInput input)
{
	float4 ret;

	ret = u_texture0.Sample(u_sampler0, input.UV);

	ret.a += NoDiscard;
	if(ret.a < 0.1)
		discard;

	ret.a = Shininess;
	ret.rgb = pow(ret.rgb, float3(2.2, 2.2, 2.2));
	ret.rgb += ObjectColor.xyz;

	return ret;
}

float4 setColorTerrain(PSInput input)
{
	float4 c0, c1, c2, c_map;
	
	c0 = u_texture0.Sample(u_sampler0, input.UV);
	c1 = u_texture1.Sample(u_sampler1, input.UV);
	c2 = u_texture2.Sample(u_sampler2, input.UV);
	c_map = u_texture3.Sample(u_sampler3, input.TerrainUV);

	float4 ret = c0 * c_map.r + c1 * c_map.g + c2 * c_map.b;
	ret.a = Shininess;
	ret.rgb = pow(ret.rgb, float3(2.2, 2.2, 2.2));
	ret.rgb += ObjectColor.xyz;

	return ret;
}

float4 setColorSkyReflection(PSInput input, float3 normal)
{
	float3 eye = normalize(input.Position - CameraPosition.xyz);
	float3 r = reflect(eye, normal);

	// refraction - use index of 1 for transparent objects ?
	/*float ratio = 1.0 / 1.33;
	vec3 r = refract(eye, o_Normal.xyz, ratio);*/

	float3 color = u_texture_cube.Sample(u_sampler_cube, r).rgb;

	color = pow(color, float3(2.2, 2.2, 2.2));
	color += ObjectColor.xyz;

	float3 x = max(float3(0.0, 0.0, 0.0), color - 0.004);
	color.rgb = (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);

	return float4(color, Shininess);
}

// Shader types
#define SH_NM_SPEC		0
#define SH_NM			1
#define SH_SPEC			2
#define SH_TERRAIN		3
#define SH_UNLIT		4
#define SH_LIT			5
#define SH_SKYBOX		6
#define SH_SKYREFLECT	7

PSOutput main(PSInput input)
{
	PSOutput output;

	if(ShaderType == SH_NM_SPEC || ShaderType == SH_NM)
		output.o_Normal = float4(calculateMappedNormal(input), 1);
	else
		output.o_Normal = float4(normalize(input.Normal), 1);

	if(ShaderType == SH_NM_SPEC || ShaderType == SH_SPEC)
		output.o_ColorSpecular = setColorSpecularMap(input);
	else if(ShaderType == SH_TERRAIN)
		output.o_ColorSpecular = setColorTerrain(input);
	else if(ShaderType == SH_SKYREFLECT)
		output.o_ColorSpecular = setColorSkyReflection(input, output.o_Normal.xyz);
	else
		output.o_ColorSpecular = setColorSpecularArg(input);

	output.o_Position = float4(input.Position, MaterialType);
	output.o_MaterialInfo = float4(DiffuseConstant, SpecularConstant, Shininess, Bloom);

	return output;
}