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
	mat4 InverseViewProjection;
	float Near;
	float Far;
};

layout(std140) uniform LightData
{
	vec4 LightPositionAndShadow;
	vec4 LightColor;
	vec4 LightAttenuationAndData;
	mat4 LightVP;
};

#define LightType int(LightAttenuationAndData.w)

layout(location=U_TEXTURE0) uniform TEXTURE_2DMS GBuffer0;
layout(location=U_TEXTURE1) uniform TEXTURE_2DMS GBuffer1;
layout(location=U_TEXTURE2) uniform TEXTURE_2DMS GBuffer2;
layout(location=U_TEXTURE3) uniform TEXTURE_2DMS GBuffer3;
layout(location=U_TEXTURE4) uniform TEXTURE_2D SSAOTexture;
layout(location=U_TEXTURE5) uniform TEXTURE_2DMS LightAccumulationTexture;
layout(location=U_TEXTURE6) uniform TEXTURE_2D ShadowTexture;
layout(location=U_TEXTURE7) uniform TEXTURE_2DMS DepthTexture;

vec3 color;
vec4 fragmentPosition;	// xyz - fragment position
float kDiffuse;
float kSpecular;
float bloom;
float shininess;
vec2 uv;
ivec2 iuv;
float attenuation = 1.0;
vec3 normal;
vec3 lightDirection;

vec3 decodeNormal(vec2 enc)
{
	vec2 fenc = enc * 4.0 - 2.0;
	float f = dot(fenc, fenc);
	float g = sqrt(1.0 - f / 4.0);
	vec3 n;
	n.xy = fenc * g;
	n.z = 1.0 - f / 2.0;
	return n;
}

float getShadow()
{
	if(LightPositionAndShadow.w < 0.1)
		return 1.0;

	vec4 shadowPosition = LightVP * vec4(fragmentPosition.xyz, 1.0);
	vec3 shadowCoords = (shadowPosition.xyz / shadowPosition.w) * vec3(0.5) + vec3(0.5);

	return shadowCoords.z - 0.005 > texture(GET_TEX_2D(ShadowTexture), shadowCoords.xy).z ? 1.0 : 0.0;
}

vec3 posFromDepth(vec2 uv, ivec2 iuv)
{
	float z = Near / (Far - texelFetch(GET_TEX_2DMS(DepthTexture), iuv, gl_SampleID).r * (Far - Near)) * Far;
	vec4 pos = vec4(uv * 2.0 - 1.0, z, 1.0);
	pos = InverseViewProjection * pos;
	return (pos.xyz / pos.w);
}

void blinnPhong()
{
	vec3 light_color = LightColor.xyz * LightAttenuationAndData.z;
	vec3 view_direction = normalize(CameraPositionAndAmbient.xyz - fragmentPosition.xyz);

	float diffuse_coef = max(0.0, dot(normal.xyz, lightDirection));
	vec3 diffuse = (kDiffuse * diffuse_coef * light_color);
		
	// Blinn-Phong
	vec3 half_vec = normalize(lightDirection + view_direction);
	float spec_coef = pow(max(dot(half_vec, normal.xyz), 0.0), shininess * 4.0);

	// Phong
	//spec_coef = pow(max(0.0, dot(view_direction, reflect(-light_direction, norm))), shininess);

	vec3 specular = (kSpecular * light_color * spec_coef);
	o_FragColor.rgb = attenuation * (diffuse + specular) * getShadow();
}

SUBROUTINE_FUNC(LT_POINT, calculateLightingSub)
void calculatePointLight()
{
	lightDirection = LightPositionAndShadow.xyz - fragmentPosition.xyz;
	
	float light_distance = length(lightDirection);
	attenuation = (smoothstep(LightAttenuationAndData.y, LightAttenuationAndData.x, light_distance));
	lightDirection = normalize(lightDirection);

	blinnPhong();
}

SUBROUTINE_FUNC(LT_DIRECTIONAL, calculateLightingSub)
void calculateDirectionalLight()
{
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
	//o_FragColor = vec4(vec3(texelFetch(GET_TEX_2DMS(GBuffer1), iuv, gl_SampleID).xyz), 1.0);

	float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722)) * (1.0 - fog_alpha);
	o_BrightColor = (color.rgb * when_gt(brightness, 1.0)) * bloom;
}

void readGBuffer()
{
	vec4 gb0 = texelFetch(GET_TEX_2DMS(GBuffer0), iuv, gl_SampleID);
	vec4 gb1 = texelFetch(GET_TEX_2DMS(GBuffer1), iuv, gl_SampleID);
	vec4 gb2 = texelFetch(GET_TEX_2DMS(GBuffer2), iuv, gl_SampleID);
	vec2 gb3 = texelFetch(GET_TEX_2DMS(GBuffer3), iuv, gl_SampleID).xy;
	
	fragmentPosition = gb0;
	
	normal = normalize(decodeNormal(gb1.xy));
	bloom = gb1.w;
	
	color = gb2.rgb;
	shininess = gb2.w;
	
	kDiffuse = gb3.x;
	kSpecular = gb3.y;
}

void main()
{	
	uv = gl_FragCoord.xy / FrameSizeAndSSAO.xy;
	iuv = ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y));

	//fragmentPosition = vec4(posFromDepth(uv, iuv), 1.0);
	
	readGBuffer();

	o_FragColor = vec4(0.0);
	
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
