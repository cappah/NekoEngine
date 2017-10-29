/* NekoEngine
 *
 * CPUParticleSystemComponent.cpp
 * Author: Alexandru Naiman
 *
 * CPU Particle System Component
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

#include <random>

#include <Renderer/VKUtil.h>
#include <Renderer/DebugMarker.h>
#include <Renderer/RenderPassManager.h>
#include <Engine/ResourceManager.h>
#include <Scene/Object.h>
#include <Scene/CameraManager.h>
#include <Scene/Components/CPUParticleSystemComponent.h>
#include <System/AssetLoader/AssetLoader.h>

#define CPU_PSYSCOMP_MODULE	"CPU_ParticleSystemComponent"

using namespace glm;
using namespace std;

ENGINE_REGISTER_COMPONENT_CLASS(CPUParticleSystemComponent);

CPUParticleSystemComponent::CPUParticleSystemComponent(ComponentInitializer *initializer) :
	ObjectComponent(initializer),
	_emitter{ nullptr },
	_particles{},
	_deadList{}
{
	ArgumentMapType::iterator it{};
	const char *ptr{ nullptr };

	if (((it = initializer->arguments.find("emittertype")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
	{
		switch (atoi(ptr))
		{
		case EMITTER_QUAD: _emitter = new QuadEmitter(); break;
		case EMITTER_CIRCLE: _emitter = new CircleEmitter(); break;
		case EMITTER_SPHERE: _emitter = new SphereEmitter(); break;
		case EMITTER_BOX: _emitter = new BoxEmitter(); break;
		case EMITTER_MESH: _emitter = new BoxEmitter(); break;
		case EMITTER_POINT: default: _emitter = new SphereEmitter(); break;
		}
	}

	if (!_emitter) { DIE("Emitter MUST be set"); }

	if (((it = initializer->arguments.find("maxemit")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_emitter->SetMaxEmit(atoi(ptr));

	if (((it = initializer->arguments.find("maxparticles")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_emitter->SetMaxParticles(atoi(ptr));

	if (((it = initializer->arguments.find("rate")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_emitter->SetEmitRate(atof(ptr));

	if (((it = initializer->arguments.find("initialsize")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_emitter->SetInitialSize(atof(ptr));

	if (((it = initializer->arguments.find("finalsize")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_emitter->SetFinalSize(atof(ptr));

	if (((it = initializer->arguments.find("lifespan")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_emitter->SetLifespan(atof(ptr));

	if (((it = initializer->arguments.find("initialvelocity")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
	{
		vec3 tmp{};
		AssetLoader::ReadFloatArray(ptr, 3, &tmp.x);
		_emitter->SetInitialVelocity(tmp);
	}

	if (((it = initializer->arguments.find("initialcolor")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
	{
		vec4 tmp{};
		AssetLoader::ReadFloatArray(ptr, 4, &tmp.x);
		_emitter->SetInitialColor(tmp);
	}

	if (((it = initializer->arguments.find("finalcolor")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
	{
		vec4 tmp{};
		AssetLoader::ReadFloatArray(ptr, 4, &tmp.x);
		_emitter->SetFinalColor(tmp);
	}

	if (((it = initializer->arguments.find("velocitycurve")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_emitter->SetVelocityCurve((uint8_t)atoi(ptr));

	if (((it = initializer->arguments.find("sizecurve")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_emitter->SetSizeCurve((uint8_t)atoi(ptr));

	if (((it = initializer->arguments.find("colorcurve")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_emitter->SetColorCurve((uint8_t)atoi(ptr));

	/*if (((it = initializer->arguments.find("particletype")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_particleType = atoi(ptr);*/
}

int CPUParticleSystemComponent::Load()
{
	int ret = ObjectComponent::Load();
	if (ret != ENGINE_OK)
		return ret;
	
	if (!_emitter) return ENGINE_INVALID_ARGS;

	vec3 tmp{ _parent->GetPosition() + _position };
	_emitter->SetPosition(tmp);

	tmp = _parent->GetRotationAngles() + _rotation;
	_emitter->SetRotation(tmp);

	tmp = _parent->GetScale() + _scale;
	_emitter->SetScale(tmp);

	_emitter->SetParticles(&_particles);
	_emitter->SetDeadList(&_deadList);

	return _emitter->InitializeParticles();
}

void CPUParticleSystemComponent::Update(double deltaTime) noexcept
{
	ObjectComponent::Update(deltaTime);
	_emitter->Update(deltaTime);
}

void CPUParticleSystemComponent::UpdatePosition() noexcept
{
	vec3 tmp{ _parent->GetPosition() + _position };
	_emitter->SetPosition(tmp);

	tmp = _parent->GetRotationAngles() + _rotation;
	_emitter->SetRotation(tmp);

	tmp = _parent->GetScale() + _scale;
	_emitter->SetScale(tmp);
}

void CPUParticleSystemComponent::UpdateData(VkCommandBuffer commandBuffer) noexcept
{
/*	Camera *cam{ CameraManager::GetActiveCamera() };
	BillboardData billboardData
	{
		cam->GetProjectionMatrix() * cam->GetView(),
		vec4(cam->GetPosition(), 1.0)
	};

	_particleBuffer->UpdateData((uint8_t *)&billboardData, _billboardDataOffset, sizeof(BillboardData), commandBuffer);
	_particleBuffer->UpdateData((uint8_t *)&_emitterData, _emitterDataOffset, sizeof(EmitterData) - sizeof(vec4) - sizeof(vec2), commandBuffer);*/
}

bool CPUParticleSystemComponent::Unload()
{
	if (!ObjectComponent::Unload())
		return false;

	delete _emitter;

	return true;
}
