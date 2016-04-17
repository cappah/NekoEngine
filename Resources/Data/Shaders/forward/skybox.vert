in vec3 a_pos;

layout(std140) uniform MatrixBlock
{
	mat4 ModelViewProjection;
	mat4 Model;
	mat4 View;
};

out vec3 v_cube_uv;

void main()
{
	vec4 pos = ModelViewProjection * vec4(a_pos, 1.0);
	v_cube_uv = a_pos;
	gl_Position = pos;
}
