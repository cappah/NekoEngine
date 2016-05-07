noperspective in vec2 v_uv;
noperspective in vec2 v_pixcoord;
noperspective in vec4 v_offset[3];

layout(location=O_FRAGCOLOR) out vec4 o_FragColor;
layout(location=O_COLORTEXTURE) out vec4 o_ColorTexture;

layout(location=U_TEXTURE0) uniform TEXTURE_2D PreviousImage;
layout(location=U_TEXTURE1) uniform TEXTURE_2D OriginalImage;
layout(location=U_TEXTURE2) uniform TEXTURE_2D BrightnessImage;
layout(location=U_TEXTURE3) uniform TEXTURE_2D DepthImage;

layout(location=U_TEXTURE4) uniform TEXTURE_2D AreaTexture;
layout(location=U_TEXTURE5) uniform TEXTURE_2D SearchTexture;

layout(std140) uniform PPSharedData
{
	vec4 SharedData;
};

layout(std140) uniform PPEffectData
{
	vec4 EffectData;
};

#define SMAA_RT_METRICS vec4( 1.0 / SharedData.x, 1.0 / SharedData.y, SharedData.x, SharedData.y)
#define SMAA_INCLUDE_VS 0
#define SMAA_GLSL_4 1 
#define SMAA_PRESET_ULTRA 1

#include "SMAA.h"

void main()
{
	if(EffectData.x == 2.0)
		o_FragColor = SMAANeighborhoodBlendingPS(v_uv, v_offset[0], GET_TEX_2D(OriginalImage), GET_TEX_2D(PreviousImage));
	else if(EffectData.x == 1.0)
		o_FragColor = SMAABlendingWeightCalculationPS(v_uv, v_pixcoord, v_offset, GET_TEX_2D(PreviousImage), GET_TEX_2D(AreaTexture), GET_TEX_2D(SearchTexture), ivec4(0));
	else
		o_FragColor = vec4(SMAAColorEdgeDetectionPS(v_uv, v_offset, GET_TEX_2D(PreviousImage)), 0.0, 0.0); 

	o_ColorTexture = o_FragColor;
}
