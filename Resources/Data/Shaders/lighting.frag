SUBROUTINE_DELEGATE(calculateLightingSub)
SUBROUTINE(0, calculateLightingSub, calculateLighting)

#include "conditionals.h"

layout(location = O_FRAGCOLOR) out vec4 o_FragColor;
layout(location = O_BRIGHTCOLOR) out vec3 o_BrightColor;

layout(std140) uniform SceneLightData
{
	vec4 CameraPositionAndAmbient;
	vec4 AmbientColorAndRClear;
	vec4 FogColorAndRFog;
	vec4 FrameSizeAndSSAO;
};

layout(std140) uniform LightData
{
	vec4 LightPositionAndShadow;
	vec4 LightColor;
	vec4 LightAttenuationAndData;
	mat4 CameraToLight;
};

#define LightType int(LightAttenuationAndData.w)

layout(location=U_TEXTURE0) uniform TEXTURE_2DMS PositionTexture;
layout(location=U_TEXTURE1) uniform TEXTURE_2DMS NormalTexture;
layout(location=U_TEXTURE2) uniform TEXTURE_2DMS ColorTexture;
layout(location=U_TEXTURE3) uniform TEXTURE_2DMS MaterialTexture;
layout(location=U_TEXTURE4) uniform TEXTURE_2D SSAOTexture;
layout(location=U_TEXTURE5) uniform TEXTURE_2DMS LightAccumulationTexture;
layout(location=U_TEXTURE6) uniform TEXTURE_2D ShadowTexture;

vec4 color; // rgb - color, a - specular
vec4 fragmentPosition;	// xyz - fragment position, w - shader type
vec4 material; // Diffuse, Specular, Specular Power, Bloom
vec2 uv;
ivec2 iuv;
float attenuation = 1.0;
vec3 normal;
vec3 lightDirection;

float getShadow(vec3 eyeDir)
{
	//return 1.0;

	if(LightPositionAndShadow.w < 0.1)
		return 1.0;

	vec4 projEyeDir = CameraToLight * vec4(eyeDir, 1.0);
	projEyeDir = projEyeDir / projEyeDir.w;

	vec2 texCoords = projEyeDir.xy * vec2(0.5, 0.5) + vec2(0.5, 0.5);

	const float bias = 0.0001;
	float depthValue = texture(GET_TEX_2D(ShadowTexture), texCoords).z - bias;

	return projEyeDir.z * 0.5 + 0.5 < depthValue ? 1.0 : 0.2;
}

void blinnPhong()
{
	vec3 light_color = LightColor.xyz * LightAttenuationAndData.z;
	vec3 view_direction = normalize(CameraPositionAndAmbient.xyz - fragmentPosition.xyz);

	float shadow = getShadow(view_direction);

	float diffuse_coef = max(0.0, dot(normal.xyz, lightDirection));
	vec3 diffuse = (material.x * diffuse_coef * light_color);
		
	// Blinn-Phong
	vec3 half_vec = normalize(lightDirection + view_direction);
	float spec_coef = pow(max(dot(half_vec, normal.xyz), 0.0), color.a * 4.0);

	// Phong
	//spec_coef = pow(max(0.0, dot(view_direction, reflect(-light_direction, norm))), color.a);		

	vec3 specular = (material.y * light_color * spec_coef);
	o_FragColor.rgb = attenuation * (diffuse + specular) * shadow;
}

SUBROUTINE_FUNC(LT_POINT, calculateLightingSub)
void calculatePointLight()
{
	normal = texelFetch(GET_TEX_2DMS(NormalTexture), iuv, gl_SampleID).xyz;
	lightDirection = LightPositionAndShadow.xyz - fragmentPosition.xyz;
	
	float light_distance = length(lightDirection);
	attenuation = (smoothstep(LightAttenuationAndData.y, LightAttenuationAndData.x, light_distance));
	lightDirection = normalize(lightDirection);

	blinnPhong();
}

SUBROUTINE_FUNC(LT_DIRECTIONAL, calculateLightingSub)
void calculateDirectionalLight()
{
	normal = texelFetch(GET_TEX_2DMS(NormalTexture), iuv, gl_SampleID).xyz;
	lightDirection = normalize(LightPositionAndShadow.xyz - fragmentPosition.xyz);

	blinnPhong();
}

SUBROUTINE_FUNC(LT_AMBIENTAL, calculateLightingSub)
void calculateAmbientLight()
{
	vec3 ambient;
	float fog_alpha = 0.0;

	float d = distance(CameraPositionAndAmbient.xyz, fragmentPosition.xyz);
	fog_alpha = clamp((d - AmbientColorAndRClear.w) / (FogColorAndRFog.w - AmbientColorAndRClear.w), 0.0, 1.0);

	vec3 fragmentLight = texelFetch(GET_TEX_2DMS(LightAccumulationTexture), iuv, gl_SampleID).rgb;
	color.rgb = color.rgb * (fragmentLight + (CameraPositionAndAmbient.w * ((FrameSizeAndSSAO.z * vec3(texture(GET_TEX_2D(SSAOTexture), uv).r)) + abs(AmbientColorAndRClear.xyz * (FrameSizeAndSSAO.z - 1.0)))));

	vec3 x = max(vec3(0.0), color.rgb - 0.004);
	vec3 mapped = (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
	mapped = (1.0 - fog_alpha) * mapped + fog_alpha * FogColorAndRFog.xyz;

	o_FragColor = vec4(mapped, 1.0);
	//o_FragColor = vec4(vec3(texture(GET_TEX_2D(ShadowTexture), uv).r), 1.0);

	float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722)) * (1.0 - fog_alpha);
	o_BrightColor = (color.rgb * when_gt(brightness, 1.0)) * material.w;
}

void main()
{	
	uv = gl_FragCoord.xy / FrameSizeAndSSAO.xy;
	iuv = ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y));

	fragmentPosition = texelFetch(GET_TEX_2DMS(PositionTexture), iuv, gl_SampleID);
	color = texelFetch(GET_TEX_2DMS(ColorTexture), iuv, gl_SampleID);
	material = texelFetch(GET_TEX_2DMS(MaterialTexture), iuv, gl_SampleID);

	o_FragColor.rgb = vec3(0.0);
	
	#ifdef HAVE_SUBROUTINES
		calculateLighting();
	#else
		if(LightType == LT_POINT)
			calculatePointLight();

		if(LightType == LT_DIRECTIONAL)
			calculateDirectionalLight();

		if(LightType == LT_AMBIENTAL)
			calculateAmbientLight();
	#endif
}
