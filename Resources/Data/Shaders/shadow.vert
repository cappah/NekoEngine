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

out VertexData
{
	vec2 UV;
	vec2 TerrainUV;
	vec3 Position;
	vec3 Normal;
	vec3 Color;
	vec3 Tangent;
	vec3 CubemapUV;
	vec3 ViewSpacePosition;
} vertexData;

layout(location=U_TEXTURE4) uniform TEXTURE_2D HeightmapTexture;

void main()
{
	vertexData.UV = a_uv;
	gl_Position = (ModelViewProjection * vec4(a_pos, 1.0));
}
