layout(location = O_FRAGCOLOR) out vec4 o_FragColor;

in vec3 v_color;
in vec2 v_uv;

layout(location = U_TEXTURE0) uniform TEXTURE_2D u_texture;

void main()
{
	o_FragColor = texture(GET_TEX_2D(u_texture), v_uv) + vec4(v_color, 0.0);
}
