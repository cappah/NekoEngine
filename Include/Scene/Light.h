/* Neko Engine
 *
 * Light.h
 * Author: Alexandru Naiman
 *
 * Light class definition 
 *
 * ----------------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman <alexandru dot naiman at icloud dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ALEXANDRU NAIMAN
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <glm/glm.hpp>

#include <Engine/Shader.h>
#include <Scene/Object.h>

enum class LightType : unsigned short
{
	Ambiental = LT_AMBIENTAL,
	Point = LT_POINT,
	Directional = LT_DIRECTIONAL,
	Spot = LT_SPOT
};

class Light :
	public Object
{
public:

	Light() :
		_type(LightType::Directional),
		_direction(0.f, 0.f, 0.f),
		_attenuation(0.f, 0.f),
		_intensity(1.f),
		_castShadows(false)
	{ };

	LightType GetType() noexcept { return _type; }
	glm::vec3& GetDirection() noexcept { return _direction; }
	glm::vec3& GetColor() noexcept { return _objectBlock.ObjectColor; }
	glm::vec2& GetAttenuation() noexcept { return _attenuation; }
	float GetIntensity() noexcept { return _intensity; }
	bool CastShadows() noexcept { return _castShadows; }

	void SetType(LightType type) noexcept { _type = type; }
	void SetDirection(glm::vec3& dir) noexcept { _direction = dir; }
	void SetAttenuation(glm::vec2& attenuation) noexcept { _attenuation = attenuation; }
	void SetIntensity(float intensity) noexcept { _intensity = intensity; }
	void SetCastShadows(bool cast) noexcept { _castShadows = cast; }

	virtual ~Light() {};

private:
	LightType _type;
	glm::vec3 _direction;
	glm::vec2 _attenuation;
	float _intensity;
	bool _castShadows;
};

