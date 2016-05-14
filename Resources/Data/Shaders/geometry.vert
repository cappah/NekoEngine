layout(location = SHADER_POSITION_ATTRIBUTE) in vec3 a_pos;
layout(location = SHADER_NORMAL_ATTRIBUTE) in vec3 a_norm;
layout(location = SHADER_COLOR_ATTRIBUTE) in vec3 a_color;
layout(location = SHADER_TANGENT_ATTRIBUTE) in vec3 a_tgt;
layout(location = SHADER_UV_ATTRIBUTE) in vec2 a_uv;
layout(location = SHADER_TERRAINUV_ATTRIBUTE) in vec2 a_uv_terrain;
layout(location = SHADER_INDEX_ATTRIBUTE) in vec4 a_bone_index;
layout(location = SHADER_WEIGHT_ATTRIBUTE) in vec4 a_bone_weight;
layout(location = SHADER_NUMBONES_ATTRIBUTE) in float a_num_bones;

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
	mat4 BoneMatrices[30];
};

#define MaterialType MaterialData1.x
#define AnimatedMesh MaterialData1.z

layout(location=U_TEXTURE4) uniform TEXTURE_2D HeightmapTexture;

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

void get_anim_position()
{
}

void main()
{
	vec3 l_pos = vec3(0.0);

	if(AnimatedMesh >= 0.9)
	{
		vec4 new_pos = vec4(a_pos, 0.0);
		vec3 new_normal = a_norm;

		vec4 curIndex = a_bone_index;
		vec4 curWeight = a_bone_weight;

		for(int i = 0; i < int(a_num_bones); i++)
		{
			mat4 m44 = BoneMatrices[int(curIndex.x)];
			new_pos += m44 * vec4(a_pos, 1.0) * curWeight.x;

			mat3 m33 = mat3(m44[0].xyz,
							m44[1].xyz,
							m44[2].xyz);
			new_normal += m33 * a_norm * curWeight.x;

			curIndex = curIndex.yzwx;
			curWeight = curWeight.yzwx;
		}
		
		l_pos = new_pos.xyz;
		vertexData.Normal = (Model * vec4(new_normal, 0.0)).xyz;
	}
	else
	{
		l_pos = get_position();
		vertexData.Normal = (Model * vec4(a_norm, 0.0)).xyz;
	}


	vec4 pos = ModelViewProjection * vec4(l_pos, 1.0);
	vertexData.Color = a_color;
	vertexData.CubemapUV = a_pos;

	vertexData.UV = a_uv;
	vertexData.TerrainUV = a_uv_terrain;
	vertexData.Position = (Model * vec4(l_pos, 1.0)).xyz;
	vertexData.ViewSpacePosition = (View * Model * vec4(l_pos, 1.0)).xyz;

	vertexData.Tangent = (Model * vec4(a_tgt, 0.0)).xyz;	

	gl_Position = pos;
}

