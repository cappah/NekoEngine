layout(location = O_FRAGCOLOR) out float o_FragColor;

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

layout(std140) uniform ShadowMapParameters
{
	mat4 inverseView;
	mat4 lightViewProjection;
	vec3 frustumCorners[4];
	ivec2 occlusionSize;
	ivec2 shadowMapSize;
};

in VertexData
{
	vec2 UV;
	vec4 FragPosLS;
} vertexData;

layout(location=U_TEXTURE9) uniform TEXTURE_2D u_ShadowMap;

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
	vec3 projectionCoords = FragPosLS.xyz / FragPosLS.w;
	projectionCoords = projectionCoords * 0.5 + 0.5;

	float closest = texture(GET_TEX_2D(u_ShadowMap), projectionCoords.xy).r;
	float current = projectionCoords.z;

	o_FragColor = current > closest ? 1.0 : 0.0;
}
