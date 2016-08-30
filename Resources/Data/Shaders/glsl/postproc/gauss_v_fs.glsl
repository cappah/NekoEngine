in vec2 v_uv;

layout(location = O_FRAGCOLOR) out vec4 o_FragColor;
layout(location = O_COLORTEXTURE) out vec4 o_ColorTexture;

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

float offset[3] = float[]( 0.0, 1.3846153846, 3.2307692308 );
float weight[3] = float[]( 0.2270270270, 0.3162162162, 0.0702702703 );

void main()
{	
	o_FragColor = texture(GET_TEX_2D(PreviousImage), vec2(gl_FragCoord)/1024.0) * weight[0];

	for (int i = 1; i < 3; i++) 
	{
		o_FragColor += texture(GET_TEX_2D(PreviousImage), (vec2(gl_FragCoord) + vec2(0.0, offset[i])) / 1024.0) * weight[i];
		o_FragColor += texture(GET_TEX_2D(PreviousImage), (vec2(gl_FragCoord) - vec2(0.0, offset[i])) / 1024.0) * weight[i];
	}
}
