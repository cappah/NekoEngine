/* NekoEngine
 *
 * Emitter.h
 * Author: Alexandru Naiman
 *
 * CPU Particle System Emitter
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

class Emitter
{
public:
	Emitter();

	uint8_t GetVelocityCurve() noexcept const { return _velocityCurve; }
	uint8_t GetScaleCurve() noexcept const { return _scaleCurve; }
	uint8_t GetColorCurve() noexcept const { return _colorCurve; }
	uint8_t GetType() noexcept const { return _type; }
	uint8_t GetParticleType() noexcept const { return _particleType; }
	uint32_t GetMaxEmit() noexcept const { return _maxEmit; }
	uint32_t GetMaxParticles() noexcept const { return _maxParticles; }
	float GetInitialSize() noexcept const { return _initialSize; }
	float GetFinalSize() noexcept const { return _finalSize; }
	float GetLifespan() noexcept const { return _lifespan; }
	float GetEmitRate() noexcept const { return _emitRate; }
	const glm::vec3 &GetPosition() noexcept const { return _position; }
	const glm::vec3 &GetRotation() noexcept const { return _rotation; }
	const glm::vec3 &GetScale() noexcept const { return _scale; }
	const glm::vec4 &GetInitialColor() noexcept const { return _initialColor; }
	const glm::vec4 &GetFinalColor() noexcept const { return _finalColor; }

	void SetVelocityCurve(uint8_t velocityCurve) noexcept { _velocityCurve = velocityCurve; }
	void SetScaleCurve(uint8_t scaleCurve) noexcept { _scaleCurve = scaleCurve; }
	void SetColorCurve(uint8_t colorCurve) noexcept { _colorCurve = colorCrve; }
	void SetType(uint8_t type) noexcept { _type = type; }
	void SetParticleType(uint8_t particleType) noexcept { _particleType = particleType; }
	void SetMaxEmit(uint32_t maxEmit) noexcept { _maxEmit = maxEmit; }
	void SetMaxParticles(uint32_t maxParticles) noexcept { _maxParticles = maxParticles; }
	void SetInitialSize(float initialSize) noexcept { _initialSize = initialSize; }
	void SetFinalSize(float finalSize) noexcept { _finalSize = finalSize; }
	void SetLifespan(float lifespan) noexcept { _lifespan = lifespan; }
	void SetEmitRate(float emitRate) noexcept { _emitRate = emitRate; }
	void SetPosition(glm::vec3 &position) noexcept { _position = position; }
	void SetRotation(glm::vec3 &rotation) noexcept { _rotation = rotation; }
	void SetScale(glm::vec3 &scale) noexcept { _scale = scale; }
	void SetInitialColor(glm::vec4 &initialColor) noexcept { _initialColor = initialColor; }
	void SetFinalColor(glm::vec4 &finalColor) noexcept { _finalColor = finalColor; }

	void Emit();

	virtual ~Emitter();
private:
	glm::vec3 _position, _rotation, _scale;
	glm::vec4 _initialColor, _finalColor;
	uint32_t _maxEmit, _maxParticles;
	uint8_t _velocityCurve, _scaleCurve, _colorCurve, _type, _particleType;
	float _initialSize, _finalSize, _lifespan, _emitRate;
};
