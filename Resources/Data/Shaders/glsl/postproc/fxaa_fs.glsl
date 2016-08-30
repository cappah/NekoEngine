NOPERSPECTIVE in vec2 v_uv;

layout(location=O_FRAGCOLOR) out vec4 o_FragColor;
layout(location=O_COLORTEXTURE) out vec4 o_ColorTexture;

layout(location=U_TEXTURE0) uniform TEXTURE_2D PreviousImage;
layout(location=U_TEXTURE1) uniform TEXTURE_2D OriginalImage;
layout(location=U_TEXTURE2) uniform TEXTURE_2D BrightnessImage;
layout(location=U_TEXTURE3) uniform TEXTURE_2D DepthImage;

layout(std140) uniform PPSharedData
{
	vec4 SharedData;
};

layout(std140) uniform PPEffectData
{
	vec4 EffectData;
};

#define FrameSize			SharedData.xy
#define Near				SharedData.z
#define Far					SharedData.w

#define Subpix				EffectData.x
#define EdgeThreshold		EffectData.y
#define EdgeThresholdMin	EffectData.z

#define FXAA_PC					1
#define FXAA_GLSL_130			1
#define FXAA_QUALITY__PRESET	39
#define FXAA_GREEN_AS_LUMA		1
#define FXAA_EARLY_EXIT			0

#include "Fxaa3_11.h"

void main()
{
	o_FragColor = FxaaPixelShader(
		v_uv,												// pos
		vec4(0.0),											// fxaaConsolePosPos
		GET_TEX_2D(PreviousImage),							// tex
		GET_TEX_2D(PreviousImage),							// fxaaConsole360TexExpBiasNegOne
		GET_TEX_2D(PreviousImage),							// fxaaConsole360TexExpBiasNegTwo
		vec2(1.0 / SharedData.x, 1.0 / SharedData.y),		// fxaaQualityRcpFrame
		vec4(0.0),											// fxaaConsoleRcpFrameOpt
		vec4(0.0),											// fxaaConsoleRcpFrameOpt2
		vec4(0.0),											// fxaaConsole360RcpFrameOpt2
		Subpix,												// fxaaQualitySubpix
		EdgeThreshold,										// fxaaQualityEdgeThreshold
		EdgeThresholdMin,									// fxaaQualityEdgeThresholdMin
		0.0,												// fxaaConsoleEdgeSharpness
		0.0,												// fxaaConsoleEdgeThreshold
		0.0,												// fxaaConsoleEdgeThresholdMin
		vec4(0.0)											// fxaaConsole360ConstDir
		);		

	o_ColorTexture = o_FragColor;
}
