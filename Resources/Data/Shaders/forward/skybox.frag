layout(location = O_FRAGCOLOR) out vec4 o_FragColor;
layout(location = O_COLORTEXTURE) out vec4 o_ColorTexture;
layout(location = O_BRIGHTCOLOR) out vec4 o_BrightColor;

in vec3 v_cube_uv;

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
	vec4 color;

	color = texture(GET_TEX_CUBE(u_texture_cube), v_cube_uv);
	color.rgb = pow(color.rgb, vec3(2.2));

	vec3 x = max(vec3(0.0), color.rgb - 0.004);
	color.rgb = (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);

	o_FragColor = color;
	o_ColorTexture = o_FragColor;
}