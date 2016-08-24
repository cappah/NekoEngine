#include "conditionals.h"

layout(location = O_FRAGCOLOR) out vec4 o_FragColor;

layout(std140) uniform ShadowData
{
	vec2 ClipPlanes;
};

in vec2 v_uv;

void main()
{	
	float d01 = (LightToView.z * 0.5 + 0.5);
	float z = ((LightToView.w < 0.0) ? -LightToView.w : d01);
	float d = (z - ClipPlanes.x) / (ClipPlanes.y - ClipPlanes.x);

	o_FragColor = vec4(d, d * d, 0.0, 0.0);
}
