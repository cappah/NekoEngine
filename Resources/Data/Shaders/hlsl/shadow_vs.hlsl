struct VSInput
{
	float3 a_pos : POSITION;
	float3 a_norm : NORMAL0;
	float3 a_color : COLOR0;
	float3 a_tgt : TANGENT0;
	float2 a_uv : TEXCOORD0;
	float2 a_uv_terrain : TEXCOORD1;
	int4 a_bone_index : BLENDINDICES0;
	float4 a_bone_weight : BLENDWEIGHT0;
	int a_num_bones : TEXCOORD2;
};

cbuffer MatrixBlock
{
	float4x4 ModelViewProjection;
	float4x4 Model;
	float4x4 View;
};

cbuffer MaterialBlock
{
	float4 MaterialData;
	float4 MaterialData1;
};

cbuffer BoneBlock
{
	float4x4 BoneMatrices[100];
};

#define MaterialType MaterialData1.x
#define AnimatedMesh MaterialData1.z

Texture2D HeightmapTexture : register(t4);
SamplerState HeightmapSampler : register(s4);

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

float3 get_position(float3 a_pos, float2 uv)
{
	//if (MaterialType != 3)
		return a_pos;
//	else
	//{
	//	float3 height;
	//	height = HeightmapTexture.Sample(HeightmapSampler, uv).xyz;
	//	return a_pos + float3(0.0, 15.0 * height.x, 0.0) + float3(0.0, 0.0 * height.y, 0.0) + float3(0.0, 7.0 * height.z, 0.0);
	//}
}

PSInput main(VSInput input)
{
	PSInput output;
	float4 l_pos = float4(1.0, 1.0, 1.0, 1.0);

	if(input.a_num_bones > 0)
	{
		float4x4 boneTransform = mul(BoneMatrices[input.a_bone_index.x], input.a_bone_weight.x);
		boneTransform += mul(BoneMatrices[input.a_bone_index.y], input.a_bone_weight.y);
		boneTransform += mul(BoneMatrices[input.a_bone_index.z], input.a_bone_weight.z);
		boneTransform += mul(BoneMatrices[input.a_bone_index.w], input.a_bone_weight.w);
		
		l_pos = mul(boneTransform, float4(input.a_pos, 1.0));
		
		float4 new_normal = mul(boneTransform, float4(input.a_norm, 0.0));
		output.Normal = mul(Model, new_normal).xyz;
	}
	else
	{
		l_pos = float4(get_position(input.a_pos, input.a_uv_terrain), 1.0);
		output.Normal = mul(Model, float4(input.a_norm, 0.0)).xyz;
	}

	output.Color = input.a_color;
	output.CubemapUV = input.a_pos;

	output.UV = input.a_uv;
	output.TerrainUV = input.a_uv_terrain;
	output.Position = mul(Model, l_pos).xyz;
	output.ViewSpacePosition = mul(View * Model, l_pos).xyz;

	output.Tangent = mul(Model, float4(input.a_tgt, 0.0)).xyz;	

	output.a_pos = mul(ModelViewProjection, l_pos);

	return output;
}
/*
void main()
{
	vec4 l_pos = vec4(1.0);

	if(a_num_bones > 0)
	{
		mat4 boneTransform = BoneMatrices[a_bone_index.x] * a_bone_weight.x;
		boneTransform += BoneMatrices[a_bone_index.y] * a_bone_weight.y;
		boneTransform += BoneMatrices[a_bone_index.z] * a_bone_weight.z;
		boneTransform += BoneMatrices[a_bone_index.w] * a_bone_weight.w;
		
		l_pos = boneTransform * vec4(a_pos, 1.0);
	}
	else
		l_pos = vec4(get_position(), 1.0);

	gl_Position = ModelViewProjection * l_pos;
	vertexData.LightPosition = LightMVP * l_pos;
}
*/