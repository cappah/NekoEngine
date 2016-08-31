/* NekoEngine
 *
 * D3D11HBAO.cpp
 * Author: Alexandru Naiman
 *
 * DirectX 11 Renderer Implementation - HBAO+ Integration
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "D3D11Renderer.h"
#include "D3D11Texture.h"
#include "D3D11Framebuffer.h"
#include "config.h"

#ifdef ENABLE_HBAO

#include <GFSDK_SSAO.h>

static GFSDK_SSAO_Context_D3D11 *_ssaoContext;
static GFSDK_SSAO_GLFunctions _ssaoGLFunctions;
static GFSDK_SSAO_Parameters_D3D11 _ssaoParameters;

bool D3D11Renderer::IsHBAOSupported() { return true; }

bool D3D11Renderer::InitializeHBAO()
{
	GFSDK_SSAO_Status status = GFSDK_SSAO_CreateContext_D3D11(_ctx.device, &_ssaoContext);
	if (status != GFSDK_SSAO_OK)
		return false;

	_ssaoParameters.Radius = 4.f;
	_ssaoParameters.Bias = 0.3f;
	_ssaoParameters.PowerExponent = 2.f;
	_ssaoParameters.Blur.Enable = true;
	_ssaoParameters.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_8;
	_ssaoParameters.Blur.Sharpness = 8.f;
	_ssaoParameters.Blur.SharpnessProfile.Enable = true;
	_ssaoParameters.Output.BlendMode = GFSDK_SSAO_OVERWRITE_RGB;

	return true;
}

bool D3D11Renderer::RenderHBAO(RHBAOArgs *args, RFramebuffer *fbo)
{
	GFSDK_SSAO_InputData_D3D11 inputData;

	D3D11Texture *depthTexture = (D3D11Texture *)args->depthTexture;
	D3D11Texture *normalTexture = (D3D11Texture *)args->normalTexture;

	inputData.DepthData.DepthTextureType = GFSDK_SSAO_DepthTextureType::GFSDK_SSAO_HARDWARE_DEPTHS;
	inputData.DepthData.pFullResDepthTextureSRV = depthTexture->GetSRV();
	inputData.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4(args->projection);
	inputData.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
	inputData.DepthData.MetersToViewSpaceUnits = 1.2f;

	inputData.NormalData.Enable = true;
	inputData.NormalData.pFullResNormalTextureSRV = normalTexture->GetSRV();
	inputData.NormalData.WorldToViewMatrix.Data = GFSDK_SSAO_Float4x4(args->worldToView);
	inputData.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;

	GFSDK_SSAO_Status status = _ssaoContext->RenderAO(_ctx.deviceContext, &inputData, &_ssaoParameters, ((D3D11Framebuffer *)fbo)->GetRTV());
	if (status != GFSDK_SSAO_OK)
		return false;

	return true;
}

#else

bool D3D11Renderer::IsHBAOSupported()
{
	return false;
}

bool D3D11Renderer::InitializeHBAO()
{
	return false;
}

bool D3D11Renderer::RenderHBAO(RHBAOArgs *args, RFramebuffer *fbo)
{
	return false;
}

#endif