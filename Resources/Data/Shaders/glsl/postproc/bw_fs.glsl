in vec2 v_uv;

layout(location=O_FRAGCOLOR) out vec4 o_FragColor;
layout(location=O_COLORTEXTURE) out vec4 o_ColorTexture;

layout(location=U_TEXTURE0) uniform TEXTURE_2D PreviousImage;
layout(location=U_TEXTURE1) uniform TEXTURE_2D OriginalImage;
layout(location=U_TEXTURE2) uniform TEXTURE_2D BrightnessImage;
layout(location=U_TEXTURE3) uniform TEXTURE_2D DepthImage;

layout(std140) uniform PPSharedData
{
	vec4 SharedData;
};

layout(std140) uniform PPEffectData
{
	vec4 EffectData;
};

#define FrameSize SharedData.xy
#define Near SharedData.z
#define Far SharedData.w

void main()
{
	vec4 color;	
	color = texture(GET_TEX_2D(PreviousImage), v_uv);
	
	float lum = 0.299 * color.r + 0.587 * color.g + 0.114 * color.b;	

	o_FragColor = vec4(lum, lum, lum, 1.0);
	o_ColorTexture = o_FragColor;
}
