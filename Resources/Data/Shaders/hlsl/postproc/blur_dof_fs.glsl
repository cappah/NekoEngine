#define GAMMA_POW 0.45454545454

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

#define DofStep EffectData.x
#define Fade EffectData.y
#define Clarity EffectData.z

float linearDepth(vec2 uv)
{
	float depth = texture(GET_TEX_2D(DepthImage), uv).x;

	depth = depth * 2.0 - 1.0;
	return ((2.0 * Near * Far) / (Far + Near - depth * (Far - Near))) / Far;
}

float calculate_mix_factor(vec2 uv)
{
	return -clamp(abs(linearDepth(uv) - Clarity) / Fade, 0.0, 1.0);
}

void main()
{
	vec4 color[9];
	vec2 uv[9];
	
	vec4 blurStep = vec4(DofStep / 1000.0);
	
	uv[0] = v_uv;
	uv[1] = v_uv + vec2(blurStep.x, 0.0);
	uv[2] = v_uv + vec2(blurStep.z, blurStep.w);
	uv[3] = v_uv + vec2(0.0, blurStep.y);
	uv[4] = v_uv + vec2(-blurStep.z, blurStep.w);
	uv[5] = v_uv + vec2(-blurStep.x, 0.0);
	uv[6] = v_uv + vec2(-blurStep.z, -blurStep.w);
	uv[7] = v_uv + vec2(0.0, -blurStep.y);
	uv[8] = v_uv + vec2(blurStep.z, -blurStep.w);
	
	color[0] = texture(GET_TEX_2D(PreviousImage), uv[0]);
	color[1] = texture(GET_TEX_2D(PreviousImage), uv[1]);
	color[2] = texture(GET_TEX_2D(PreviousImage), uv[2]);
	color[3] = texture(GET_TEX_2D(PreviousImage), uv[3]);
	color[4] = texture(GET_TEX_2D(PreviousImage), uv[4]);
	color[5] = texture(GET_TEX_2D(PreviousImage), uv[5]);
	color[6] = texture(GET_TEX_2D(PreviousImage), uv[6]);
	color[7] = texture(GET_TEX_2D(PreviousImage), uv[7]);
	color[8] = texture(GET_TEX_2D(PreviousImage), uv[8]);
	
	float d[9];
	d[1] = calculate_mix_factor(uv[1]);
	d[2] = calculate_mix_factor(uv[2]);
	d[3] = calculate_mix_factor(uv[3]);
	d[4] = calculate_mix_factor(uv[4]);
	d[5] = calculate_mix_factor(uv[5]);
	d[6] = calculate_mix_factor(uv[6]);
	d[7] = calculate_mix_factor(uv[7]);
	d[8] = calculate_mix_factor(uv[8]);
	
	float total = 2.0 + d[1] + d[2] + d[3] + d[4] + d[5] + d[6] + d[7] + d[8];
	o_FragColor = (2.0 * color[0] + d[1] * color[1] + d[2] * color[2] + d[3] * color[3]
	+ d[4] * color[4] + d[5] * color[5] + d[6] * color[6] + d[7] * color[7] + d[8] * color[8]) / total;
	
	o_ColorTexture = o_FragColor;
}
