layout(location = O_FRAGCOLOR) out float o_FragColor;

in VertexData
{
	vec4 LightPosition;
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
#define MaterialType MaterialData1.

layout(location=U_TEXTURE0) uniform TEXTURE_2D u_ShadowMap;
layout(location=U_TEXTURE_CUBE) uniform TEXTURE_CUBE u_texture_cube;

SUBROUTINE_DELEGATE(setColorSub)
SUBROUTINE_DELEGATE(setNormalSub)

SUBROUTINE(0, setColorSub, setColor)
SUBROUTINE(1, setNormalSub, setNormal)

SUBROUTINE_FUNC(SH_SUB_C_SPEC_MAP, setColorSub)
void setColorSpecularMap() { }
SUBROUTINE_FUNC(SH_SUB_N_MAP, setNormalSub)
void setNormalMap() { }

void main()
{

	vec3 projectionCoords = LightPosition.xyz / LightPosition.w;
	projectionCoords = projectionCoords * 0.5 + 0.5;

	float closest = texture(GET_TEX_2D(u_ShadowMap), projectionCoords.xy).r;
	float current = projectionCoords.z;

	o_FragColor = current > closest ? 1.0 : 0.0;
}
