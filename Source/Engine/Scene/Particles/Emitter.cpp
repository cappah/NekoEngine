/* NekoEngine
 *
 * Emitter.cpp
 * Author: Alexandru Naiman
 *
 * CPU particle emitters
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

#define _USE_MATH_DEFINES
#include <math.h>

#include <System/Logger.h>
#include <Scene/SceneManager.h>
#include <Scene/Particles/Emitter.h>

#define EMITTER_MODULE	"Emitter"

using namespace glm;

Emitter::Emitter() :
	_particles{ nullptr },
	_position{ 0.f }, _rotation{ 0.f }, _scale{ 0.f },
	_maxEmit{ 0 }, _maxParticles{ 0 },
	_emitRate{ 0.0 }, _nextEmit{ 0.0 },
	_initialSize{ 0.0 }, _finalSize{ 0.0 },
	_lifespan{ 0.0 },
	_initialVelocity{ 0.f },
	_initialColor{ 0.f }, _finalColor{ 0.f },
	_particleClass{ "Particle" },
	_particleInitializer{},
	_deadList{ nullptr }
{ }
Emitter::~Emitter() { }
QuadEmitter::QuadEmitter() : Emitter() { }
QuadEmitter::~QuadEmitter() { }
CircleEmitter::CircleEmitter() : Emitter() { }
CircleEmitter::~CircleEmitter() { }
SphereEmitter::SphereEmitter() : Emitter() { }
SphereEmitter::~SphereEmitter() { }
BoxEmitter::BoxEmitter() : Emitter() { }
BoxEmitter::~BoxEmitter() { }
PointEmitter::PointEmitter() : Emitter() { }
PointEmitter::~PointEmitter() { }
MeshEmitter::MeshEmitter() : Emitter() { }
MeshEmitter::~MeshEmitter() { }

int Emitter::InitializeParticles()
{
	_particleClass = "MeshParticle";

	_particleInitializer.arguments.insert({ "mesh", "stm_m_house" });
	_particleInitializer.arguments.insert({ "material", "mat_m_house" });
	
	for (uint32_t i = 0; i < _maxParticles; ++i)
	{
		Particle *p{ (Particle *)Engine::NewObject(*_particleClass, &_particleInitializer) };
		if (!p)
		{
			Logger::Log(EMITTER_MODULE, LOG_CRITICAL, "Failed to create particle %d of type %s", i, *_particleClass);
			return ENGINE_FAIL;
		}

		p->SetVelocityCurve(PART_UPD_CURVE_LINEAR);
		p->SetSizeCurve(PART_UPD_CURVE_LINEAR);
		p->SetColorCurve(PART_UPD_CURVE_LINEAR);

		if (p->Load())
		{
			Logger::Log(EMITTER_MODULE, LOG_CRITICAL, "Failed to load particle %d of type %s", i, *_particleClass);
			return ENGINE_FAIL;
		}
		
		//SceneManager::GetLoadingScene()->AddObject(p);

		_particles->Add(p);
		_deadList->Add(i);
	}
	return ENGINE_OK;
}

void Emitter::Update(double deltaTime) noexcept
{
	_nextEmit -= deltaTime;
	if (_nextEmit > 0.0) return;
	Emit();
	_nextEmit = _emitRate;
}

void Emitter::Emit()
{
//	uint32_t firstParticle{ 0 };

	int64_t bound = _maxEmit > (int64_t)_deadList->Count() ? _maxEmit : (int64_t)_deadList->Count();

	for (int64_t i = 0; i < bound; ++i)
	{
		uint32_t id = (*_deadList)[0];
		_EmitParticle(id);
		//_deadList->Remove(0);
	}
}

void QuadEmitter::_EmitParticle(uint32_t id)
{
	Particle *p{ (*_particles)[id] };

	vec3 pos{ _position };

	uint32_t x = (uint32_t)(_scale.x / 2.f);
	uint32_t y = (uint32_t)(_scale.y / 2.f);

	pos.x += (Platform::Rand() % x) + 1;
	pos.y += (Platform::Rand() % y) + 1;

	p->SetPosition(pos);
	p->SetLifespan(_lifespan);
	p->SetAge(0);
	p->SetVisible(true);
}

void CircleEmitter::_EmitParticle(uint32_t id)
{
	Particle *p{ (*_particles)[id] };

	vec3 pos{ _position };

	double d = (double)(Platform::Rand() % (uint32_t)_scale.x);
	double angle = (double)((Platform::Rand() % 360) + 1) / (2.0 * M_PI);

	pos.x += d * cos(angle);
	pos.y += d * sin(angle);

	p->SetPosition(pos);
	p->SetLifespan(_lifespan);
	p->SetAge(0);
	p->SetVisible(true);
}

void SphereEmitter::_EmitParticle(uint32_t id)
{
	Particle *p{ (*_particles)[id] };

	vec3 pos{ _position };

	double d1 = (double)(Platform::Rand() % (uint32_t)_scale.x);
	double d2 = (double)(Platform::Rand() % (uint32_t)_scale.x);
	double angle = (double)((Platform::Rand() % 360) + 1) / (2.0 * M_PI);

	pos.x += d1 * cos(angle);
	pos.y += d1 * sin(angle);
	pos.z += d2 * cos(angle);

	p->SetPosition(pos);
	p->SetLifespan(_lifespan);
	p->SetAge(0);
	p->SetVisible(true);
}

void BoxEmitter::_EmitParticle(uint32_t id)
{
	Particle *p{ (*_particles)[id] };

	vec3 pos{ _position };

	uint32_t x = (uint32_t)(_scale.x / 2.f);
	uint32_t y = (uint32_t)(_scale.y / 2.f);
	uint32_t z = (uint32_t)(_scale.z / 2.f);

	pos.x += (Platform::Rand() % x) + 1;
	pos.y += (Platform::Rand() % y) + 1;
	pos.z += (Platform::Rand() % z) + 1;

	p->SetPosition(pos);
	p->SetLifespan(_lifespan);
	p->SetAge(0);
	p->SetVisible(true);
}

void PointEmitter::_EmitParticle(uint32_t id)
{
	Particle *p{ (*_particles)[id] };
	p->SetPosition(_position);
	p->SetLifespan(_lifespan);
	p->SetAge(0);
	p->SetVisible(true);
}

void MeshEmitter::_EmitParticle(uint32_t id)
{
	Particle *p{ (*_particles)[id] };

	vec3 pos{ _position };

	p->SetPosition(pos);
	p->SetLifespan(_lifespan);
	p->SetAge(0);
	p->SetVisible(true);
}
