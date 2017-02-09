/* NekoEngine
 *
 * RoadPatch.h
 * Author: Alexandru Naiman
 *
 * RoadPatch class definition
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2017, NekoEngine
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

#include "RoadPatch.h"

using namespace glm;

static ComponentInitializer _rpCompInit(nullptr);
static ComponentInitializer _groundInit(nullptr, vec3(0.f), vec3(0.f), vec3(10.f, .1f, 10.f));

static bool _initComp{ false };

RoadPatch::RoadPatch(ObjectInitializer *initializer) :
	Object(initializer)
{
	if (_initComp) return;

	_groundInit.arguments.insert({ "mesh", "stm_quad" });
	_groundInit.arguments.insert({ "material", "mat_road" });
}

int RoadPatch::Load()
{
	int ret{ ENGINE_OK };
	ObjectComponent *comp{ nullptr };
	_rpCompInit.parent = this;

	comp = Engine::NewComponent("RoadPatchComponent", &_rpCompInit);
	if (!comp) return ENGINE_OUT_OF_RESOURCES;
	if ((ret = comp->Load()) != ENGINE_OK) return ret;

	comp = Engine::NewComponent("StaticMeshComponent", &_groundInit);
	if (!comp) return ENGINE_OUT_OF_RESOURCES;
	if ((ret = comp->Load()) != ENGINE_OK) return ret;

	if ((ret = Object::Load()) != ENGINE_OK) return ret;
	return ENGINE_OK;
}

RoadPatch::~RoadPatch()
{
	//
}