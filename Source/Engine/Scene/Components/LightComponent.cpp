/* NekoEngine
 *
 * LightComponent.cpp
 * Author: Alexandru Naiman
 *
 * Light component
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2017, Alexandru Naiman
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

#include <Scene/Object.h>
#include <Scene/CameraManager.h>
#include <Scene/Components/LightComponent.h>
#include <System/Logger.h>
#include <System/AssetLoader/AssetLoader.h>
#include <Renderer/ShadowRenderer.h>

#define MASK_HI_16			0x0000FFFF
#define MASK_LO_16			0xFFFF0000

#define MASK_POS_0			0xFC000000
#define MASK_POS_1			0x03F00000
#define MASK_POS_2			0x000FC000
#define MASK_POS_3			0x00003F00
#define MASK_POS_4			0x000000FC
#define MASK_POS_5			0xFFFF0000

#define SHIFT_POS_0(x)		x << 26
#define SHIFT_POS_1(x)		x << 20
#define SHIFT_POS_2(x)		x << 14	
#define SHIFT_POS_3(x)		x << 8
#define SHIFT_POS_4(x)		x << 2
#define SHIFT_POS_5(x)		x << 16

using namespace glm;

static vec3 __cubeTargets[6]
{
	vec3( 1.0f,  0.0f,  0.0f), 
	vec3(-1.0f,  0.0f,  0.0f),
	vec3( 0.0f,  1.0f,  0.0f),
	vec3( 0.0f, -1.0f,  0.0f),
	vec3( 0.0f,  0.0f,  1.0f),
	vec3( 0.0f,  0.0f, -1.0f)
};

static vec3 __cubeUp[6]
{
	vec3(0.0f, -1.0f,  0.0f),
	vec3(0.0f, -1.0f,  0.0f),
	vec3(0.0f,  0.0f, -1.0f),
	vec3(0.0f,  0.0f,  1.0f),
	vec3(0.0f, -1.0f,  0.0f),
	vec3(0.0f, -1.0f,  0.0f)
};

static mat4 __biasMatrix
{
	.5f, 0.f, 0.f, 0.f,
	0.f, .5f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	.5f, .5f, 0.f, 1.f
};

ENGINE_REGISTER_COMPONENT_CLASS(LightComponent);

LightComponent::LightComponent(ComponentInitializer *initializer) :
	ObjectComponent(initializer)
{
	_lightId = Renderer::GetInstance()->AllocLight();
	_shadowCasterId = 0;
	_lightMatrices[0] = _lightMatrices[1] = _lightMatrices[2] = _lightMatrices[3] = _lightMatrices[4] = _lightMatrices[5] = nullptr;

	if (_lightId < 0 || ((_light = Renderer::GetInstance()->GetLight(_lightId)) == nullptr))
	{ DIE("Maximum number of lights exceeded"); }

	_light->position = vec4(0.f);
	_light->color = vec4(1.f);
	_light->data = vec4(0.f);
	_light->direction = vec4(0.f, 0.f, 0.f, -1.f);
	_intensity = 1.f;
	
	ArgumentMapType::iterator it;
	const char *ptr = nullptr;
	
	if (((it = initializer->arguments.find("color")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		AssetLoader::ReadFloatArray(ptr, 3, &_light->color.x);
	
	if (((it = initializer->arguments.find("intensity")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_light->color.a = _intensity = (float)atof(ptr);

	if (((it = initializer->arguments.find("radius")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		AssetLoader::ReadFloatArray(ptr, 2, &_light->data.x);
	
	if (((it = initializer->arguments.find("direction")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		AssetLoader::ReadFloatArray(ptr, 3, &_light->direction.x);
	
	if (((it = initializer->arguments.find("angle")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
	{
		AssetLoader::ReadFloatArray(ptr, 2, &_light->data.z);

		_light->data.z = cos(radians(_light->data.z / 2.f));
		_light->data.w = cos(radians(_light->data.w / 2.f));
	}

	if (((it = initializer->arguments.find("type")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
	{
		size_t len = strlen(ptr);

		if (!strncmp(ptr, "directional", len))
			_light->position.w = LT_Directional;
		else if (!strncmp(ptr, "point", len))
			_light->position.w = LT_Point;
		else if (!strncmp(ptr, "spot", len))
			_light->position.w = LT_Spot;
	}

	if ((it = initializer->arguments.find("castshadows")) != initializer->arguments.end())
	{
		uint32_t v{ 0 };

		if (ShadowRenderer::RegisterShadowCaster(_lightId, _light->position.w == LT_Point ? 6 : 1, _shadowMapIds, _shadowCasterId) != ENGINE_OK)
		{ DIE("Out of resources"); }

		v |= SHIFT_POS_0(_shadowMapIds[0]);

		if (_light->position.w == LT_Point)
		{
			v |= SHIFT_POS_1(_shadowMapIds[1]);
			v |= SHIFT_POS_2(_shadowMapIds[2]);
			v |= SHIFT_POS_3(_shadowMapIds[3]);
			v |= SHIFT_POS_4(_shadowMapIds[4]);
		}

		_light->direction.w = (float)v;

		ShadowRenderer::GetMatrices(_shadowCasterId, _lightMatrices, _biasedLightMatrices);
	}
	
	_light->position = vec4(_parent->GetPosition() + _position, _light->position.w);
}

void LightComponent::Update(double deltaTime) noexcept
{
	ObjectComponent::Update(deltaTime);

	if (_lightMatrices[0] && _light->position.w == LT_Directional)
		UpdatePosition();
}

void LightComponent::UpdatePosition() noexcept
{
	ObjectComponent::UpdatePosition();

//	Camera *cam{ CameraManager::GetActiveCamera() };
	_light->position = vec4(_parent->GetPosition() + _position, _light->position.w);

	if (!_lightMatrices[0])
		return;

	mat4 projection{};
	mat4 view{};

	if (_light->position.w == LT_Directional)
	{
		float size = Engine::GetConfiguration().Renderer.ShadowMapSize / 10.f;
		/*float texelSize = (size * 2) / (float)(1 << Engine::GetConfiguration().Renderer.ShadowMapSize);

		vec3 position = cam->GetPosition() + cam->GetForward() * size;		

		float d = distance(vec3(0.f), position);*/

		projection = ortho(-size, size, -size, size, 0.1f, 1000.f);
		view = lookAt((-vec3(_light->direction) * 40.f) , vec3(0.f), vec3(0.f, 1.f, 0.f));
	}
	else if (_light->position.w == LT_Point)
	{
		projection = perspective(radians(90.f), 1.f, 1.f, 1000.f);
		view = lookAt(vec3(_light->position), __cubeTargets[0], __cubeUp[0]);
	}
	else if (_light->position.w == LT_Spot)
	{
		projection = perspective(radians(_light->data.w / 2.f), 1.0f, 0.1f, _light->data.y);
		view = lookAt(vec3(_light->position), vec3(_light->direction) * _light->data.y, vec3(0.f, 1.f, 0.f));
	}
	
	projection[1][1] *= -1;
	*_lightMatrices[0] = projection * view;
	*_biasedLightMatrices[0] = __biasMatrix * *_lightMatrices[0];

	if (_light->position.w == LT_Point)
	{
		view = lookAt(vec3(_light->position), __cubeTargets[1], __cubeUp[1]);
		*_lightMatrices[1] = projection * view;
		*_biasedLightMatrices[1] = __biasMatrix * *_lightMatrices[1];

		view = lookAt(vec3(_light->position), __cubeTargets[2], __cubeUp[2]);
		*_lightMatrices[2] = projection * view;
		*_biasedLightMatrices[2] = __biasMatrix * *_lightMatrices[2];

		view = lookAt(vec3(_light->position), __cubeTargets[3], __cubeUp[3]);
		*_lightMatrices[3] = projection * view;
		*_biasedLightMatrices[3] = __biasMatrix * *_lightMatrices[3];

		view = lookAt(vec3(_light->position), __cubeTargets[4], __cubeUp[4]);
		*_lightMatrices[4] = projection * view;
		*_biasedLightMatrices[4] = __biasMatrix * *_lightMatrices[4];

		view = lookAt(vec3(_light->position), __cubeTargets[5], __cubeUp[5]);
		*_lightMatrices[5] = projection * view;
		*_biasedLightMatrices[5] = __biasMatrix * *_lightMatrices[5];
	}
}

bool LightComponent::Unload()
{
	if (!ObjectComponent::Unload())
		return false;

	if(_lightMatrices[0])
		ShadowRenderer::UnregisterShadowCaster(_shadowCasterId);

	Renderer::GetInstance()->FreeLight(_lightId);

	return true;
}
