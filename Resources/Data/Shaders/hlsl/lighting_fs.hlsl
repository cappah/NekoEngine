//#include "conditionals.h"

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 v_uv : TEXCOORD0;
	uint v_sampleID : SV_SampleIndex;
};

struct PSOutput
{
	float4 o_FragColor : SV_Target0;
	float3 o_BrightColor : SV_Target1;
};

struct PSData
{
	PSOutput output;
	float4 color; // rgb - color, a - specular
	float4 fragmentPosition;	// xyz - fragment position, w - shader type
	float4 material; // Diffuse, Specular, Specular Power, Bloom
	float2 uv;
	int3 iuv;
	float attenuation;
	float3 normal;
	float3 lightDirection;
};

cbuffer SceneLightData
{
	float4 CameraPositionAndAmbient;
	float4 AmbientColorAndRClear;
	float4 FogColorAndRFog;
	float4 FrameSizeAndSSAO;
};

cbuffer LightData
{
	float4 LightPositionAndShadow;
	float4 LightColor;
	float4 LightAttenuationAndData;
	float4x4 CameraToLight;
};

#define LightType int(LightAttenuationAndData.w)

Texture2DMS<float4> PositionTexture : register(t0);
Texture2DMS<float4> NormalTexture : register(t1);
Texture2DMS<float4> ColorTexture : register(t2);
Texture2DMS<float4> MaterialTexture : register(t3);
Texture2D SSAOTexture : register(t4);
SamplerState SSAOTextureSampler : register(s4);
Texture2DMS<float4> LightAccumulationTexture : register(t5);
Texture2D ShadowTexture : register(t6);
SamplerState ShadowTextureSamper : register(s6);

float getShadow(float3 eyeDir)
{
	//return 1.0;

	if(LightPositionAndShadow.w < 0.1)
		return 1.0;

	float4 projEyeDir = mul(CameraToLight, float4(eyeDir, 1.0));
	projEyeDir = projEyeDir / projEyeDir.w;

	float2 texCoords = projEyeDir.xy * float2(0.5, 0.5) + float2(0.5, 0.5);

	const float bias = 0.0001;
	float depthValue = ShadowTexture.Sample(ShadowTextureSamper, texCoords).z - bias;

	return projEyeDir.z * 0.5 + 0.5 < depthValue ? 1.0 : 0.2;
}

void blinnPhong(PSData data)
{
	float3 light_color = LightColor.xyz * LightAttenuationAndData.z;
	float3 view_direction = normalize(CameraPositionAndAmbient.xyz - data.fragmentPosition.xyz);

	float shadow = getShadow(view_direction);

	float diffuse_coef = max(0.0, dot(data.normal.xyz, data.lightDirection));
	float3 diffuse = (data.material.x * diffuse_coef * light_color);
		
	// Blinn-Phong
	float3 half_vec = normalize(data.lightDirection + view_direction);
	float spec_coef = pow(max(dot(half_vec, data.normal.xyz), 0.0), data.color.a * 4.0);

	// Phong
	//spec_coef = pow(max(0.0, dot(view_direction, reflect(-light_direction, norm))), color.a);		

	float3 specular = (data.material.y * light_color * spec_coef);
	data.output.o_FragColor.rgb = data.attenuation * (diffuse + specular) * shadow;
}

void calculatePointLight(PSData data)
{
	data.normal = NormalTexture.Load(data.iuv.xy, data.iuv.z).xyz;
	data.lightDirection = LightPositionAndShadow.xyz - data.fragmentPosition.xyz;
	
	float light_distance = length(data.lightDirection);
	data.attenuation = (smoothstep(LightAttenuationAndData.y, LightAttenuationAndData.x, light_distance));
	data.lightDirection = normalize(data.lightDirection);

	blinnPhong(data);
}

void calculateDirectionalLight(PSData data)
{
	data.normal = NormalTexture.Load(data.iuv.xy, data.iuv.z).xyz;
	data.lightDirection = normalize(LightPositionAndShadow.xyz - data.fragmentPosition.xyz);

	blinnPhong(data);
}

void calculateAmbientLight(PSData data)
{
	float3 ambient;
	float fog_alpha = 0.0;

	float d = distance(CameraPositionAndAmbient.xyz, data.fragmentPosition.xyz);
	fog_alpha = clamp((d - AmbientColorAndRClear.w) / (FogColorAndRFog.w - AmbientColorAndRClear.w), 0.0, 1.0);

	float3 fragmentLight = LightAccumulationTexture.Load(data.iuv.xy, data.iuv.z).rgb;
	float ssao = SSAOTexture.Sample(SSAOTextureSampler, data.uv).r;
	data.color.rgb = data.color.rgb * (fragmentLight + (CameraPositionAndAmbient.w * ((FrameSizeAndSSAO.z * float3(ssao, ssao, ssao)) + abs(AmbientColorAndRClear.xyz * (FrameSizeAndSSAO.z - 1.0)))));

	float3 x = max(float3(0.0, 0.0, 0.0), data.color.rgb - 0.004);
	float3 mapped = (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
	mapped = (1.0 - fog_alpha) * mapped + fog_alpha * FogColorAndRFog.xyz;

	data.output.o_FragColor = float4(mapped, 1.0);
	//o_FragColor = vec4(vec3(texture(GET_TEX_2D(ShadowTexture), uv).r), 1.0);

	float brightness = dot(data.color.rgb, float3(0.2126, 0.7152, 0.0722)) * (1.0 - fog_alpha);
//	data.output.o_BrightColor = (color.rgb * when_gt(brightness, 1.0)) * data.material.w;
}

PSOutput main(PSInput input)
{	
	PSData data;
	data.attenuation = 1.0;

	data.uv = input.v_uv / FrameSizeAndSSAO.xy;
	data.iuv = int3(int(input.v_uv.x), int(input.v_uv.y), input.v_sampleID);

	data.fragmentPosition = PositionTexture.Load(data.iuv.xy, data.iuv.z);
	data.color = ColorTexture.Load(data.iuv.xy, data.iuv.z);
	data.material = MaterialTexture.Load(data.iuv.xy, data.iuv.z);

	data.output.o_FragColor = float4(0.0, 0.0, 0.0, 1.0);
	data.output.o_BrightColor = float3(0.0, 0.0, 0.0);

	if(LightType == 0)
		calculatePointLight(data);

	if(LightType == 1)
		calculateDirectionalLight(data);

	if(LightType == 2)
		calculateAmbientLight(data);

	return data.output;
}
