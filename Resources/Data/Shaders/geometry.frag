SUBROUTINE_DELEGATE(setColorSub)
SUBROUTINE_DELEGATE(setNormalSub)

SUBROUTINE(0, setColorSub, setColor)
SUBROUTINE(1, setNormalSub, setNormal)

layout(location = O_POSITION) out vec4 o_Position;
layout(location = O_NORMAL) out vec4 o_Normal;
layout(location = O_COLORSPECULAR) out vec4 o_ColorSpecular;
layout(location = O_MATERIALINFO) out vec4 o_MaterialInfo;

in VertexData
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

layout(std140) uniform ObjectBlock
{
	vec4 CameraPosition;
	vec4 ObjectColor;
};

layout(std140) uniform MaterialBlock
{
	vec4 MaterialData;
	vec4 MaterialData1;
};

#define DiffuseConstant MaterialData.x
#define SpecularConstant MaterialData.y
#define Shininess MaterialData.z
#define Bloom MaterialData.w
#define MaterialType MaterialData1.x
#define ShaderType MaterialData1.x
#define NoDiscard MaterialData1.y

#define AnimatedMesh MaterialData1.z

layout(location=U_TEXTURE0) uniform TEXTURE_2D u_texture0;
layout(location=U_TEXTURE1) uniform TEXTURE_2D u_texture1;
layout(location=U_TEXTURE2) uniform TEXTURE_2D u_texture2;
layout(location=U_TEXTURE3) uniform TEXTURE_2D u_texture3;
layout(location=U_TEXTURE_CUBE) uniform TEXTURE_CUBE u_texture_cube;

vec3 calculateMappedNormal()
{
	vec3 norm = normalize(vertexData.Normal);
	vec3 tgt = normalize(vertexData.Tangent);
	tgt = normalize(tgt - dot(tgt, norm) * norm);
	vec3 bitgt = cross(tgt, norm);

	vec4 norm_tex = texture(GET_TEX_2D(u_texture1), vertexData.UV);
	vec3 map_norm = norm_tex.xyz * 2.0 - vec3(1.0);

	mat3 tbn = mat3(tgt, bitgt, norm);
	vec3 new_norm = tbn * normalize(map_norm); //normalize(tbn * map_norm);

	return new_norm;
}

SUBROUTINE_FUNC(SH_SUB_C_SPEC_MAP, setColorSub)
void setColorSpecularMap()
{
	o_ColorSpecular = texture(GET_TEX_2D(u_texture0), vertexData.UV);

	o_ColorSpecular.a += NoDiscard;
	if(o_ColorSpecular.a < 0.1)
		discard;

	o_ColorSpecular.a = texture(GET_TEX_2D(u_texture2), vertexData.UV).r;
	o_ColorSpecular.rgb = pow(o_ColorSpecular.rgb, vec3(2.2));
	o_ColorSpecular.rgb += ObjectColor.xyz;
}

SUBROUTINE_FUNC(SH_SUB_C_SPEC_ARG, setColorSub)
void setColorSpecularArg()
{
	o_ColorSpecular = texture(GET_TEX_2D(u_texture0), vertexData.UV);

	o_ColorSpecular.a += NoDiscard;
	if(o_ColorSpecular.a < 0.1)
		discard;

	o_ColorSpecular.a = Shininess;
	o_ColorSpecular.rgb = pow(o_ColorSpecular.rgb, vec3(2.2));
	o_ColorSpecular.rgb += ObjectColor.xyz;
}

SUBROUTINE_FUNC(SH_SUB_C_TERRAIN, setColorSub)
void setColorTerrain()
{
	vec4 c0, c1, c2, c_map;
	
	c0 = texture(GET_TEX_2D(u_texture0), vertexData.UV);
	c1 = texture(GET_TEX_2D(u_texture1), vertexData.UV);
	c2 = texture(GET_TEX_2D(u_texture2), vertexData.UV);
	c_map = texture(GET_TEX_2D(u_texture3), vertexData.TerrainUV);

	o_ColorSpecular = c0 * c_map.r + c1 * c_map.g + c2 * c_map.b;
	o_ColorSpecular.a = Shininess;
	o_ColorSpecular.rgb = pow(o_ColorSpecular.rgb, vec3(2.2));
	o_ColorSpecular.rgb += ObjectColor.xyz;
}

SUBROUTINE_FUNC(SH_SUB_C_SKYREFLECT, setColorSub)
void setColorSkyReflection()
{
	vec3 eye = normalize(vertexData.Position - CameraPosition.xyz);
	vec3 r = reflect(eye, o_Normal.xyz);

	// refraction - use index of 1 for transparent objects ?
	/*float ratio = 1.0 / 1.33;
	vec3 r = refract(eye, o_Normal.xyz, ratio);*/

	vec3 color = texture(GET_TEX_CUBE(u_texture_cube), r).rgb;

	color = pow(color, vec3(2.2));
	color += ObjectColor.xyz;

	vec3 x = max(vec3(0.0), color - 0.004);
	color.rgb = (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);

	o_ColorSpecular.rgb = color;
	o_ColorSpecular.a = Shininess;
}

SUBROUTINE_FUNC(SH_SUB_N_MAP, setNormalSub)
void setNormalMap()
{
	o_Normal.xyz = calculateMappedNormal();
}

SUBROUTINE_FUNC(SH_SUB_N_ARG, setNormalSub)
void setNormalArg()
{
	o_Normal.xyz = normalize(vertexData.Normal);
}

void main()
{
	#ifdef HAVE_SUBROUTINES
		setNormal();
		setColor();	
	#else
		if(ShaderType == SH_NM_SPEC || ShaderType == SH_SPEC)
			setColorSpecularMap();
		else if(ShaderType == SH_TERRAIN)
			setColorTerrain();
		else if(ShaderType == SH_SKYREFLECT)
			setColorSkyReflection();
		else
			setColorSpecularArg();

		if(ShaderType == SH_NM_SPEC || ShaderType == SH_NM)
			setNormalMap();
		else
			setNormalArg();
	#endif

	o_Position = vec4(vertexData.Position, MaterialType);
	o_MaterialInfo = vec4(DiffuseConstant, SpecularConstant, Shininess, Bloom);
}