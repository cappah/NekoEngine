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

#define Step EffectData.x

void main()
{
	vec3 sample0, sample1, sample2, sample3;
	float blurStep = Step / 100.0;
	
	sample0 = texture(GET_TEX_2D(BrightnessImage), vec2(v_uv.x - blurStep, v_uv.y - blurStep)).rgb; 
	sample1 = texture(GET_TEX_2D(BrightnessImage), vec2(v_uv.x + blurStep, v_uv.y + blurStep)).rgb;
	sample2 = texture(GET_TEX_2D(BrightnessImage), vec2(v_uv.x + blurStep, v_uv.y - blurStep)).rgb;
	sample3 = texture(GET_TEX_2D(BrightnessImage), vec2(v_uv.x - blurStep, v_uv.y + blurStep)).rgb;

	o_FragColor.rgb = (sample0 + sample1 + sample2 + sample3) / 4.0;
}
