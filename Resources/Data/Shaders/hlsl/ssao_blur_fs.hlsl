struct PSInput
{
	float4 pos : SV_POSITION;
	float2 v_uv : TEXCOORD0;
};

cbuffer SSAOBlurDataBlock
{
	int Radius;
};

Texture2D SSAOTexture : register(t0);
SamplerState SSAOSampler : register(s0);

float main(PSInput input) : SV_Target0
{
	uint width, height;
	SSAOTexture.GetDimensions(width, height);

	float2 texelSize = 1.0 / float2(float(width), float(height));
	float result = 0.0;

	int blur_half = Radius / 2;
	float blur_total = float(Radius * Radius);

	for(int x = -blur_half; x < blur_half; ++x)
	{
		for(int y = -blur_half; y < blur_half; ++y)
		{
			float2 offset = float2(float(x), float(y)) * texelSize;
			result += SSAOTexture.Sample(SSAOSampler, input.v_uv + offset).r;
		}
	}

	return result / blur_total;
}
