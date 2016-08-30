layout(location = SHADER_POSITION_ATTRIBUTE) in vec3 a_pos;
layout(location = SHADER_NORMAL_ATTRIBUTE) in vec3 a_norm;
layout(location = SHADER_COLOR_ATTRIBUTE) in vec3 a_color;
layout(location = SHADER_TANGENT_ATTRIBUTE) in vec3 a_tgt;
layout(location = SHADER_UV_ATTRIBUTE) in vec2 a_uv;
layout(location = SHADER_TERRAINUV_ATTRIBUTE) in vec2 a_uv_terrain;
layout(location = SHADER_INDEX_ATTRIBUTE) in ivec4 a_bone_index;
layout(location = SHADER_WEIGHT_ATTRIBUTE) in vec4 a_bone_weight;
layout(location = SHADER_NUMBONES_ATTRIBUTE) in int a_num_bones;

layout(std140) uniform MatrixBlock
{
	mat4 ModelViewProjection;
	mat4 Model;
	mat4 View;
};

layout(std140) uniform MaterialBlock
{
	vec4 MaterialData;
	vec4 MaterialData1;
};

layout(std140) uniform BoneBlock
{
	mat4 BoneMatrices[SH_MAX_BONES];
};

layout(std140) uniform LightMatrixBlock
{
	mat4 LightMVP;
};

#define MaterialType MaterialData1.x
#define AnimatedMesh MaterialData1.z

layout(location=U_TEXTURE4) uniform TEXTURE_2D HeightmapTexture;

out VertexData
{
	vec4 LightPosition;
} vertexData;

vec3 get_position()
{
	if (MaterialType != SH_TERRAIN)
		return a_pos;
	else
	{
		vec3 height;
		height = texture(GET_TEX_2D(HeightmapTexture), a_uv_terrain).xyz;
		return a_pos + vec3(0.0, 15.0 * height.x, 0.0) + vec3(0.0, 0.0 * height.y, 0.0) + vec3(0.0, 7.0 * height.z, 0.0);
	}
}

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