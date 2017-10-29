/* NekoEngine
 *
 * Particle.h
 * Author: Alexandru Naiman
 *
 * CPU mesh particles
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

#include <Scene/Particles/MeshParticle.h>

ENGINE_REGISTER_OBJECT_CLASS(MeshParticle);

MeshParticle::MeshParticle(ObjectInitializer *initializer) :
	Particle(initializer),
	_smcInitializer{},
	_smc{ nullptr }
{
	ArgumentMapType::iterator it{};
	//const char *ptr{ nullptr };

	_smcInitializer.parent = this;
	_smcInitializer.arguments = initializer->arguments;
}

int MeshParticle::Load()
{
	_smc = (StaticMeshComponent *)Engine::NewComponent("StaticMeshComponent", &_smcInitializer);
	int ret{ _smc->Load() };
	if (ret != ENGINE_OK) return ret;

	AddComponent("Mesh", _smc);

	ret = Particle::Load();
	if (ret != ENGINE_OK) return ret;

	//SetVisible(false);
	SetNoCull(true);

	return ENGINE_OK;
}

MeshParticle::~MeshParticle() { }
