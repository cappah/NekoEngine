layout(location = SHADER_POSITION_ATTRIBUTE) in vec3 a_pos;

layout(std140) uniform MatrixBlock
{
	mat4 ModelViewProjection;
};

void main()
{
	gl_Position =  ModelViewProjection * vec4(a_pos, 1.0);
}
