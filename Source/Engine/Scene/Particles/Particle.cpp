/* NekoEngine
 *
 * Particle.h
 * Author: Alexandru Naiman
 *
 * CPU particles
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

#include <Scene/Particles/Easing.h>
#include <Scene/Particles/Particle.h>
#include <System/Logger.h>
#include <System/AssetLoader/AssetLoader.h>

#define PART_MODULE	"Particle"

ENGINE_REGISTER_OBJECT_CLASS(Particle);

using namespace glm;

static inline CurveFunction __part_getCurveFunction(uint8_t id)
{
	switch (id)
	{
		case PART_UPD_CURVE_LINEAR: return &linearEase;
		case PART_UPD_CURVE_QUADRATIC: return &quadraticEaseInOut;
		case PART_UPD_CURVE_CUBIC: return &cubicEaseInOut;
		case PART_UPD_CURVE_QUARTIC: return &quarticEaseInOut;
		case PART_UPD_CURVE_QUINTIC: return &quinticEaseInOut;
		case PART_UPD_CURVE_SINE: return &sineEaseInOut;
		case PART_UPD_CURVE_CIRCULAR: return &circularEaseInOut;
		case PART_UPD_CURVE_EXPONENTIAL: return &exponentialEaseInOut;
		case PART_UPD_CURVE_ELASTIC: return &elasticEaseInOut;
		case PART_UPD_CURVE_BACK: return &backEaseInOut;
		case PART_UPD_CURVE_BOUNCE: return &bounceEaseInOut;
	}

	Logger::Log(PART_MODULE, LOG_WARNING, "Unknown curve specified %d; using linear", id);

	return &linearEase;
}

Particle::Particle(ObjectInitializer *initializer) :
	Object(initializer),
	_age{ 0.0 }, _weight{ 0.0 },
	_destination{ 0.f }, _finalScale{ 0.f },
	_finalColor{ 0.f },
	_velocityCurve{ nullptr },
	_colorCurve{ nullptr },
	_sizeCurve{ nullptr },
	_material{ nullptr }
{
	ArgumentMapType::iterator it{};
	const char *ptr{ nullptr };

	if (((it = initializer->arguments.find("velocitycurve")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_velocityCurve = __part_getCurveFunction(atoi(ptr));

	if (((it = initializer->arguments.find("sizecurve")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_sizeCurve = __part_getCurveFunction(atoi(ptr));

	if (((it = initializer->arguments.find("colorcurve")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_colorCurve = __part_getCurveFunction(atoi(ptr));

	if (((it = initializer->arguments.find("weight")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_weight = atof(ptr);

	if (((it = initializer->arguments.find("destination")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		AssetLoader::ReadFloatArray(ptr, 3, &_destination.x);

	if (((it = initializer->arguments.find("finalscale")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		AssetLoader::ReadFloatArray(ptr, 3, &_finalScale.x);

	if (((it = initializer->arguments.find("finalcolor")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		AssetLoader::ReadFloatArray(ptr, 4, &_finalColor.x);
}

void Particle::SetVelocityCurve(uint8_t id) { _velocityCurve = __part_getCurveFunction(id); }
void Particle::SetSizeCurve(uint8_t id) { _sizeCurve = __part_getCurveFunction(id); }
void Particle::SetColorCurve(uint8_t id) { _colorCurve = __part_getCurveFunction(id); }

int Particle::Load()
{
	int ret{ Object::Load() };
	if (ret != ENGINE_OK) return ret;

	if (!_velocityCurve || !_sizeCurve || !_colorCurve)
		return ENGINE_INVALID_ARGS;

	return ENGINE_OK;
}

void Particle::Update(double deltaTime) noexcept
{
	if (_age > _lifespan)
	{
		if(_visible) SetVisible(false);
		return;
	}

	double val = _velocityCurve(1.0 * _age / _lifespan);
	_position = mix(_position, _destination, val);

	/*val = _sizeCurve(1.0 * _age / _lifespan);
	_scale = mix(_scale, _finalScale, val);

	val = _colorCurve(1.0 * _age / _lifespan);
	vec4 color = mix(_material->GetDiffuseColor(), _finalColor, val);
	_material->SetDiffuseColor(color);*/

	_age += deltaTime;

	Object::Update(deltaTime);
}

bool Particle::Unload() noexcept
{
	if (!Object::Unload())
		return false;

	return true;
}

Particle::~Particle() { }
