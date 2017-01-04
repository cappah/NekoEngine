SUBROUTINE_DELEGATE(setColorSub)
SUBROUTINE_DELEGATE(setNormalSub)

SUBROUTINE(0, setColorSub, setColor)
SUBROUTINE(1, setNormalSub, setNormal)

layout(location = O_POSITION) out vec4 o_Position;
layout(location = O_NORMAL) out vec4 o_GBuffer1;
layout(location = O_COLORSPECULAR) out vec4 o_GBuffer2;
layout(location = O_MATERIALINFO) out vec2 o_GBuffer3;

in VertexData
{
	vec2 UV;
	vec2 TerrainUV;
	vec3 Position;
	vec3 Normal;
	vec3 Color;
	vec3 CubemapUV;
	vec3 ViewSpacePosition;
	mat3 TBN;
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

vec3 normal;
vec3 color;
float shininess;

vec2 encodeNormal(vec3 n)
{
	float p = sqrt(n.z * 8.0 + 8.0);
	return vec2(n.xy / p + 0.5);
}

vec3 calculateMappedNormal()
{
	vec4 norm_tex = texture(GET_TEX_2D(u_texture1), vertexData.UV);
	vec3 map_norm = normalize(norm_tex.xyz * 2.0 - vec3(1.0));
	return normalize(vertexData.TBN * map_norm);
}

SUBROUTINE_FUNC(SH_SUB_C_SPEC_MAP, setColorSub)
void setColorSpecularMap()
{
	vec4 tex = texture(GET_TEX_2D(u_texture0), vertexData.UV);

	tex.a += NoDiscard;
	if(tex.a < 0.1)
		discard;

	color = pow(tex.xyz, vec3(2.2));
	color += ObjectColor.xyz;
	shininess = texture(GET_TEX_2D(u_texture2), vertexData.UV).r;
}

SUBROUTINE_FUNC(SH_SUB_C_SPEC_ARG, setColorSub)
void setColorSpecularArg()
{
	vec4 tex = texture(GET_TEX_2D(u_texture0), vertexData.UV);
	
	tex.a += NoDiscard;
	if(tex.a < 0.1)
		discard;
	
	color = pow(tex.xyz, vec3(2.2));
	color += ObjectColor.xyz;
	shininess = Shininess;
}

SUBROUTINE_FUNC(SH_SUB_C_TERRAIN, setColorSub)
void setColorTerrain()
{
	vec3 c0, c1, c2, c_map;
	
	c0 = texture(GET_TEX_2D(u_texture0), vertexData.UV).rgb;
	c1 = texture(GET_TEX_2D(u_texture1), vertexData.UV).rgb;
	c2 = texture(GET_TEX_2D(u_texture2), vertexData.UV).rgb;
	c_map = texture(GET_TEX_2D(u_texture3), vertexData.TerrainUV).rgb;

	color = c0 * c_map.r + c1 * c_map.g + c2 * c_map.b;
	color = pow(color, vec3(2.2));
	color += ObjectColor.xyz;
	shininess = Shininess;
}

SUBROUTINE_FUNC(SH_SUB_C_SKYREFLECT, setColorSub)
void setColorSkyReflection()
{
	vec3 eye = normalize(vertexData.Position - CameraPosition.xyz);
	vec3 r = reflect(eye, normal);

	// refraction - use index of 1 for transparent objects ?
	/*float ratio = 1.0 / 1.33;
	vec3 r = refract(eye, o_Normal.xyz, ratio);*/

	color = texture(GET_TEX_CUBE(u_texture_cube), r).rgb;

	color = pow(color, vec3(2.2));
	color += ObjectColor.xyz;

	vec3 x = max(vec3(0.0), color - 0.004);
	color.rgb = (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);

	shininess = Shininess;
}

SUBROUTINE_FUNC(SH_SUB_N_MAP, setNormalSub)
void setNormalMap()
{
	normal = calculateMappedNormal();
}

SUBROUTINE_FUNC(SH_SUB_N_ARG, setNormalSub)
void setNormalArg()
{
	normal = normalize(vertexData.Normal);
}

void main()
{
	#ifdef HAVE_SUBROUTINES
		setNormal();
		setColor();	
	#else
		if(ShaderType == SH_NM_SPEC || ShaderType == SH_NM)
			setNormalMap();
		else
			setNormalArg();

		if(ShaderType == SH_NM_SPEC || ShaderType == SH_SPEC)
			setColorSpecularMap();
		else if(ShaderType == SH_TERRAIN)
			setColorTerrain();
		else if(ShaderType == SH_SKYREFLECT)
			setColorSkyReflection();
		else
			setColorSpecularArg();

	
	#endif

	o_Position = vec4(vertexData.Position, MaterialType);
	
	o_GBuffer1 = vec4(encodeNormal(normal), 0.f, Bloom);
	o_GBuffer2 = vec4(color, shininess);
	o_GBuffer3 = vec2(DiffuseConstant, SpecularConstant);
}
