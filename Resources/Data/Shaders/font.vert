layout(location = SHADER_POSITION_ATTRIBUTE) in vec3 a_pos;
layout(location = SHADER_COLOR_ATTRIBUTE) in vec3 a_color;
layout(location = SHADER_UV_ATTRIBUTE) in vec2 a_uv;

layout(std140) uniform DataBlock
{
	mat4 u_projection;
};

out vec3 v_color;
out vec2 v_uv;

void main()
{
	v_color = a_color;
	v_uv = a_uv;
	gl_Position = u_projection * vec4(a_pos, 1.0);
}
