/* NekoEngine
 *
 * ShadowRenderer.cpp
 * Author: Alexandru Naiman
 *
 * Deferred shadow renderer
 *
 * Based on: https://mynameismjp.wordpress.com/the-museum/samples-tutorials-tools/deferred-shadow-maps-sample/
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

#include <Renderer/VKUtil.h>
#include <Renderer/ShadowRenderer.h>
#include <Engine/CameraManager.h>

using namespace glm;

VkCommandBuffer _commandBuffer = VK_NULL_HANDLE;
Texture *_occlusionTexture = nullptr;
uint32_t _shadowMapSize = 4096;
vec3 _vsFrustumCorners[8];
vec3 _wsFrustumCorners[8];
vec3 _lsFrustumCorners[8];
vec3 _farFrustumCorners[4];

int ShadowRenderer::Initialize()
{
	return ENGINE_OK;
}

bool ShadowRenderer::BuildCommandBuffers()
{
	return true;
}

void ShadowRenderer::UpdateData(VkCommandBuffer cmdBuffer)
{
	// get bounding frustum in _vsFrustumCorners
	for (uint32_t i = 0; i < 4; ++i)
		_farFrustumCorners[i] = _vsFrustumCorners[i + 4];

	_CalculateFrustum(0);
}

VkCommandBuffer ShadowRenderer::GetCommandBuffer() { return _commandBuffer; }
Texture *ShadowRenderer::GetOcclusionTexture() { return _occlusionTexture; }
Camera *_shadowCamera = nullptr;

void ShadowRenderer::_CalculateFrustum(uint32_t lightId)
{
/*	Camera *cam = CameraManager::GetActiveCamera();
	Light *light = Renderer::GetInstance()->GetLight(lightId);

	vec3 frustumCenter(0.f);
	for (int i = 0; i < 8; ++i)
		frustumCenter += _wsFrustumCorners[i];
	frustumCenter /= 8;

	vec3 lightDirection = vec3(light->position.x, light->position.y, light->position.z);

	float dist = max(cam->GetFar() - cam->GetNear(), distance(_vsFrustumCorners[4], _vsFrustumCorners[5])) + 50.f;
	mat4 viewMatrix = lookAt(frustumCenter - (lightDirection * frustumCenter), frustumCenter, vec3(0.f, 1.f, 0.f));

	vec3 mins, maxes;
	for (uint32_t i = 0; i < 8; ++i)
	{
		_lsFrustumCorners[i] = vec3(viewMatrix * vec4(_wsFrustumCorners[i], 1.0));

		if (!i) mins = maxes = _lsFrustumCorners[0];

		if (_lsFrustumCorners[i].x > maxes.x)
			maxes.x = _lsFrustumCorners[i].x;
		else if (_lsFrustumCorners[i].x < mins.x)
			mins.x = _lsFrustumCorners[i].x;

		if (_lsFrustumCorners[i].y > maxes.y)
			maxes.y = _lsFrustumCorners[i].y;
		else if (_lsFrustumCorners[i].y < mins.y)
			mins.y = _lsFrustumCorners[i].y;

		if (_lsFrustumCorners[i].z > maxes.z)
			maxes.z = _lsFrustumCorners[i].z;
		else if (_lsFrustumCorners[i].z < mins.z)
			mins.z = _lsFrustumCorners[i].z;
	}

	float nearClipOffset = 100.f;
	_shadowCamera->Ortho(mins.x, maxes.x, mins.y, maxes.y, -maxes.z - nearClipOffset, -mins.z);
	_shadowCamera->SetView(viewMatrix);*/
}

void ShadowRenderer::Release()
{
	if (_commandBuffer != VK_NULL_HANDLE)
		VKUtil::FreeCommandBuffer(_commandBuffer);

	delete _occlusionTexture;
}
