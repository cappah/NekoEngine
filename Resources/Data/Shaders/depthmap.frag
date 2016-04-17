layout(location = O_FRAGCOLOR) out vec4 o_FragColor;
layout(location = O_COLORTEXTURE) out vec4 o_ColorTexture;

in vec2 v_uv;

layout(std140) uniform shader_data
{
	vec2 FrameSize;
	TEXTURE_2D LoadScreenTexture;
};

void main()
{
	o_FragColor = texture(GET_TEX_2D(LoadScreenTexture), v_uv);
	o_ColorTexture = o_FragColor;
}
