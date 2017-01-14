/* NekoEngine
 *
 * GPUParticleSystemComponent.h
 * Author: Alexandru Naiman
 *
 * GPU Particle System Component
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

struct EmitterData
{
	glm::vec3 position;
	float lifespan;
	glm::vec3 initialVelocity;
	float rate;
	glm::vec3 rotation;
	float initialSize;
	glm::vec3 scale;
	float finalSize;
	glm::vec4 initialColor;
	glm::vec4 finalColor;
	float deltaTime;	
	float enableGravity;
	int32_t maxEmit;
	uint32_t maxParticles;	
	uint32_t emitterType;
	uint32_t particleType;
	uint32_t velocityCurve;
	uint32_t sizeCurve;
	uint32_t colorCurve;
	uint32_t enable;
	glm::vec4 _internalShaderData1;
	glm::vec2 _internalShaderData2;
};

struct ParticleTexture
{
	float age;
	int index;
};

class ENGINE_API GPUParticleSystemComponent : public ObjectComponent
{
public:
	GPUParticleSystemComponent(ComponentInitializer *initializer);

	virtual int Load() override;

	virtual void Enable(bool enable) override { ObjectComponent::Enable(enable); _emitterData.enable = enable ? 1 : 0; }

	virtual void Update(double deltaTime) noexcept override { ObjectComponent::Update(deltaTime); _emitterData.deltaTime = (float)deltaTime; }
	virtual void UpdatePosition() noexcept override;
	virtual void UpdateData(VkCommandBuffer commandBuffer) noexcept override;

	virtual bool Unload() override;

	~GPUParticleSystemComponent() noexcept { }

protected:
	Buffer *_particleBuffer;
	VkCommandBuffer _drawCommandBuffer, _computeCommandBuffer;
	VkDescriptorPool _descriptorPool;
	VkDescriptorSet _drawDescriptorSet, _computeDescriptorSet;
	VkDeviceSize _vertexBufferSize;
	EmitterData _emitterData;
	NArray<ParticleTexture> _particleTextures;
	NArray<uint32_t> _textureIds;
	Texture *_texture, *_noiseTexture;
};

#if defined(_MSC_VER)
template class ENGINE_API NArray<ParticleTexture>;
#endif