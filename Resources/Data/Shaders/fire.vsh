#ifdef MOBILE

#ifndef GL_FRAGMENT_PRECISION_HIGH
	precision mediump float;
	precision mediump int;
#else
	precision highp float;
	precision highp int;
#endif

#endif

in vec3 a_pos;
in vec3 a_color;
in vec2 a_uv;

out vec3 v_color;
out vec3 v_pos;
out vec2 v_uv;

layout(location = 0) uniform mat4 u_mvp;
layout(location = 1) uniform mat4 u_world;

void main()
{
	vec4 pos = u_mvp * vec4(a_pos, 1.0);
	v_color = a_color;
	v_uv = a_uv;
	v_pos = (u_world * vec4(a_pos, 1.0)).xyz;
	gl_Position = pos;
}
