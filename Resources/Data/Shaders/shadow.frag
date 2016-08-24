layout(location = O_POSITION) out vec4 o_Position;
layout(location = O_NORMAL) out vec4 o_Normal;
layout(location = O_COLORSPECULAR) out vec4 o_ColorSpecular;
layout(location = O_MATERIALINFO) out vec4 o_MaterialInfo;

in VertexData
{
	vec2 UV;
	vec2 TerrainUV;
	vec3 Position;
	vec3 Normal;
	vec3 Color;
	vec3 Tangent;
	vec3 CubemapUV;
	vec3 ViewSpacePosition;
} vertexData;

layout(std140) uniform ObjectBlock
{
	vec4 CameraPosition;
	vec4 ObjectColor;
};

layout(std140) uniform MaterialBlock
{
	vec4 MaterialData;
	vec4 MaterialData1;
};

layout(location=U_TEXTURE0) uniform TEXTURE_2D u_texture0;
layout(location=U_TEXTURE1) uniform TEXTURE_2D u_texture1;
layout(location=U_TEXTURE2) uniform TEXTURE_2D u_texture2;
layout(location=U_TEXTURE3) uniform TEXTURE_2D u_texture3;
layout(location=U_TEXTURE_CUBE) uniform TEXTURE_CUBE u_texture_cube;

//layout(location = O_FRAGCOLOR) out vec4 o_FragColor;


void main()
{	
	/*float d01 = (LightToView.z * 0.5 + 0.5);
	float z = ((LightToView.w < 0.0) ? -LightToView.w : d01);
	float d = (z - ClipPlanes.x) / (ClipPlanes.y - ClipPlanes.x);

	o_FragColor = vec4(d, d * d, 0.0, 0.0);*/
	//o_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
