/* NekoEngine
 *
 * LightComponent.h
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

#pragma once

#include <Engine/Engine.h>
#include <Renderer/Renderer.h>
#include <Scene/ObjectComponent.h>

class ENGINE_API LightComponent : public ObjectComponent
{
public:
	LightComponent(ComponentInitializer *initializer);

	Light *GetLight() noexcept { return _light; }

	float GetIntensity() const { return _intensity; }

	void SetColor(glm::vec3 &color) { _light->color.x = color.x; _light->color.x = color.y; _light->color.x = color.z; }
	void SetDirection(glm::vec3 &direction) { _light->direction.x = direction.x; _light->direction.y = direction.y; _light->direction.z = direction.z; }
	void SetIntensity(float intensity) { _intensity = intensity; if (_enabled) _light->color.a = intensity; }

	virtual void Enable(bool enable) override { ObjectComponent::Enable(enable); _light->color.a = enable ? _intensity : 0.f; }

	virtual void Update(double deltaTime) noexcept override;
	virtual void UpdatePosition() noexcept override;

	virtual bool Unload() override;

	virtual ~LightComponent() { };

protected:
	int32_t _lightId;
	uint8_t _shadowCasterId;
	Light *_light;
	float _intensity;
	uint32_t _shadowMapIds[6];
	glm::mat4 *_lightMatrices[6], *_biasedLightMatrices[6];
};
