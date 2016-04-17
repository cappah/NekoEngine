layout(location = O_FRAGCOLOR) out float o_FragColor;

in vec2 v_uv;

layout(std140) uniform SSAOBlurDataBlock
{
	int Radius;
};

layout(location=U_TEXTURE0) uniform TEXTURE_2D SSAOTexture;

void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(GET_TEX_2D(SSAOTexture), 0));
	float result = 0.0;

	int blur_half = Radius / 2;
	float blur_total = float(Radius * Radius);

	for(int x = -blur_half; x < blur_half; ++x)
	{
		for(int y = -blur_half; y < blur_half; ++y)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(GET_TEX_2D(SSAOTexture), v_uv + offset).r;
		}
	}

	o_FragColor = result / blur_total;
}
