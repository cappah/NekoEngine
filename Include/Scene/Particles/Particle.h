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

#pragma once

#include <Scene/Object.h>

#define PART_UPD_CURVE_LINEAR		0
#define PART_UPD_CURVE_QUADRATIC	1
#define PART_UPD_CURVE_CUBIC		2
#define PART_UPD_CURVE_QUARTIC		3
#define PART_UPD_CURVE_QUINTIC		4
#define PART_UPD_CURVE_SINE			5
#define PART_UPD_CURVE_CIRCULAR		6
#define PART_UPD_CURVE_EXPONENTIAL	7
#define PART_UPD_CURVE_ELASTIC		8
#define PART_UPD_CURVE_BACK			9
#define PART_UPD_CURVE_BOUNCE		11

typedef double (*CurveFunction)(double);

class Particle : public Object
{
public:
	ENGINE_API Particle(ObjectInitializer *initializer);

	ENGINE_API void SetVelocityCurve(uint8_t id);
	ENGINE_API void SetSizeCurve(uint8_t id);
	ENGINE_API void SetColorCurve(uint8_t id);

	ENGINE_API void SetAge(double age) { _age = age; }
	ENGINE_API void SetWeight(double weight) { _weight = weight; }
	ENGINE_API void SetLifespan(double lifespan) { _lifespan = lifespan; }
	ENGINE_API void SetDestination(glm::vec3 &destination) { _destination = destination; }
	ENGINE_API void SetFinalScale(glm::vec3 &finalScale) { _finalScale = finalScale; }
	ENGINE_API void SetFinalColor(glm::vec4 &finalColor) { _finalColor = finalColor; }
	ENGINE_API void SetMaterial(Material *material) { _material = material; }

	ENGINE_API double GetAge() { return _age; }
	ENGINE_API double GetWeight() { return _weight; }
	ENGINE_API double GetLifespan() { return _lifespan; }
	ENGINE_API const glm::vec3 &GetDestination() const { return _destination; }
	ENGINE_API const glm::vec3 &GetFinalScale() const { return _finalScale; }
	ENGINE_API const glm::vec4 &GetFinalColor() const { return _finalColor; }
	ENGINE_API Material *GetMaterial() { return _material; }

	ENGINE_API virtual int Load() override;
	ENGINE_API virtual void Update(double deltaTime) noexcept override;
	ENGINE_API virtual bool Unload() noexcept override;

	ENGINE_API ~Particle();

protected:
	double _age, _weight, _lifespan;
	glm::vec3 _destination, _finalScale;
	glm::vec4 _finalColor;
	CurveFunction _velocityCurve;
	CurveFunction _colorCurve;
	CurveFunction _sizeCurve;
	Material *_material;
};