#ifdef MOBILE

#ifndef GL_FRAGMENT_PRECISION_HIGH
	precision mediump float;
	precision mediump int;
#else
	precision highp float;
	precision highp int;
#endif

#endif

layout(location = 0) out vec4 o_FragColor;
layout(location = 1) out vec4 o_BrightColor;

in vec3 v_color;
in vec3 v_pos;
in vec2 v_uv;

layout(location = 20) uniform vec3 u_camera_position;

layout(location = 21) uniform vec3 u_fog_color;
layout(location = 22) uniform vec3 u_obj_color;
layout(location = 23) uniform float u_r_clear;
layout(location = 24) uniform float u_r_fog;

layout(location = 30) uniform float u_fire_disp;
layout(location = 31) uniform float u_time;

#ifdef BINDLESS_TEXTURE_ARB
layout(location = 10) uniform uvec2 u_samplers[3];
#elif defined(BINDLESS_TEXTURE_NV)
layout(location = 10) uniform uint64_t u_samplers[3];
#else
layout(location = 10) uniform sampler2D u_texture0;
layout(location = 11) uniform sampler2D u_texture1;
layout(location = 12) uniform sampler2D u_texture2;
#endif

void main()
{
	float d = distance(u_camera_position, v_pos);
	float alpha = clamp((d - u_r_clear) / (u_r_fog - u_r_clear), 0.0, 1.0);

	vec4 fog_color = vec4(u_fog_color, 1.0);
	
	vec4 color;	
	
	#if defined(BINDLESS_TEXTURE_ARB) || defined(BINDLESS_TEXTURE_NV)
		vec2 disp = texture(sampler2D(u_samplers[2]), vec2(v_uv.x, v_uv.y + u_time)).xy;			
		vec2 offset = (2.0 * disp - 1.0) * u_fire_disp;
		vec2 coord = v_uv + offset;	
			
		color = texture(sampler2D(u_samplers[0]), coord);
		color.a = color.a * texture(sampler2D(u_samplers[1]), v_uv).r;
	#else
		vec2 disp = texture(u_texture2, vec2(v_uv.x, v_uv.y + u_time)).xy;
		vec2 offset = (2.0 * disp - 1.0) * u_fire_disp;
		vec2 coord = v_uv + offset;	
		
		color = texture(u_texture0, coord);
		color.a = color.a * texture(u_texture1, v_uv).r;
	#endif
		
	if(color.a <= 0.1)
		discard;

	o_FragColor = (1.0 - alpha) * color + alpha * fog_color;
	
	float brightness = dot(o_FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.7)
        o_BrightColor = vec4(o_FragColor.rgb, 1.0);
}
