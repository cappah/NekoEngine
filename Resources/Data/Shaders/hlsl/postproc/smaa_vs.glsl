layout(location = SHADER_POSITION_ATTRIBUTE) in vec2 a_pos;

noperspective out vec2 v_uv;
noperspective out vec2 v_pixcoord;
noperspective out vec4 v_offset[3];

layout(std140) uniform PPSharedData
{
	vec4 SharedData;
};

layout(std140) uniform PPEffectData
{
	vec4 EffectData;
};

#define SMAA_RT_METRICS vec4( 1.0 / SharedData.x, 1.0 / SharedData.y, SharedData.x, SharedData.y)
#define SMAA_INCLUDE_PS 0
#define SMAA_GLSL_4 1 
#define SMAA_PRESET_ULTRA 1

#include "SMAA.h"

void main()
{
	v_uv = a_pos * 0.5 + 0.5;

	if(EffectData.x == 2)
		SMAANeighborhoodBlendingVS(v_uv, v_offset[0]);
	else if(EffectData.x == 1)
		SMAABlendingWeightCalculationVS(v_uv, v_pixcoord, v_offset);
	else
		SMAAEdgeDetectionVS(v_uv, v_offset);

	gl_Position =  vec4(a_pos, 0.0, 1.0);
}
