/* NekoEngine
 *
 * Emitter.h
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

#pragma once

#include <Engine/Defs.h>
#include <Runtime/Runtime.h>
#include <Scene/Particles/Particle.h>

#define EMITTER_QUAD		0
#define EMITTER_CIRCLE		1
#define EMITTER_SPHERE		2
#define EMITTER_BOX			3
#define EMITTER_POINT		4
#define EMITTER_MESH		5

class Emitter
{
public:
	ENGINE_API Emitter();

	ENGINE_API void SetParticles(NArray<Particle *> *particles) { _particles = particles; }
	ENGINE_API void SetDeadList(NArrayTS<uint32_t> *deadList) { _deadList = deadList; }
	ENGINE_API void SetPosition(glm::vec3 &position) { _position = position; }
	ENGINE_API void SetRotation(glm::vec3 &rotation) { _rotation = rotation; }
	ENGINE_API void SetScale(glm::vec3 &scale) { _scale = scale; }
	ENGINE_API void SetMaxEmit(uint32_t maxEmit) { _maxEmit = maxEmit; }
	ENGINE_API void SetMaxParticles(uint32_t maxParticles) { _maxParticles = maxParticles; }
	ENGINE_API void SetEmitRate(double emitRate) { _emitRate = emitRate; }
	ENGINE_API void SetInitialSize(double initialSize) { _initialSize = initialSize; }
	ENGINE_API void SetFinalSize(double finalSize) { _finalSize = finalSize; }
	ENGINE_API void SetLifespan(double lifespan) { _lifespan = lifespan; }
	ENGINE_API void SetInitialVelocity(glm::vec3 &initialVelocity) { _initialVelocity = initialVelocity; }
	ENGINE_API void SetInitialColor(glm::vec4 &initialColor) { _initialColor = initialColor; }
	ENGINE_API void SetFinalColor(glm::vec4 &finalColor) { _finalColor = finalColor; }
	ENGINE_API void SetVelocityCurve(uint8_t curveId) { _velocityCurve = curveId; }
	ENGINE_API void SetColorCurve(uint8_t curveId) { _colorCurve = curveId; }
	ENGINE_API void SetSizeCurve(uint8_t curveId) { _sizeCurve = curveId; }

	ENGINE_API int InitializeParticles();

	ENGINE_API virtual void Update(double deltaTime) noexcept;

	ENGINE_API void Emit();

	ENGINE_API virtual ~Emitter();

protected:
	NArray<Particle *> *_particles;
	glm::vec3 _position, _rotation, _scale;
	uint32_t _maxEmit, _maxParticles;
	uint8_t _velocityCurve, _colorCurve, _sizeCurve;
	double _emitRate, _nextEmit;
	double _initialSize, _finalSize;
	double _lifespan;
	glm::vec3 _initialVelocity;
	glm::vec4 _initialColor, _finalColor;
	NString _particleClass;
	ObjectInitializer _particleInitializer;
	NArrayTS<uint32_t> *_deadList;

	virtual void _EmitParticle(uint32_t id) = 0;
};

class QuadEmitter : public Emitter
{
public:
	ENGINE_API QuadEmitter();
	ENGINE_API ~QuadEmitter();

protected:
	virtual void _EmitParticle(uint32_t id) override;
};

class CircleEmitter : public Emitter
{
public:
	ENGINE_API CircleEmitter();
	ENGINE_API ~CircleEmitter();

protected:
	virtual void _EmitParticle(uint32_t id) override;
};

class SphereEmitter : public Emitter
{
public:
	ENGINE_API SphereEmitter();
	ENGINE_API ~SphereEmitter();

protected:
	virtual void _EmitParticle(uint32_t id) override;
};

class BoxEmitter : public Emitter
{
public:
	ENGINE_API BoxEmitter();
	ENGINE_API ~BoxEmitter();

protected:
	virtual void _EmitParticle(uint32_t id) override;
};

class PointEmitter : public Emitter
{
public:
	ENGINE_API PointEmitter();
	ENGINE_API ~PointEmitter();

protected:
	virtual void _EmitParticle(uint32_t id) override;
};

class MeshEmitter : public Emitter
{
public:
	ENGINE_API MeshEmitter();
	ENGINE_API ~MeshEmitter();

	ENGINE_API void SetMesh(StaticMesh *mesh) { _mesh = mesh; }

protected:
	StaticMesh *_mesh;
	virtual void _EmitParticle(uint32_t id) override;
};
