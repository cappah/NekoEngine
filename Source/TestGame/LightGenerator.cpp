/* NekoEngine
 *
 * MovingObject.h
 * Author: Alexandru Naiman
 *
 * MovingObject class definition 
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

#include "LightGenerator.h"
#include <System/AssetLoader/AssetLoader.h>
#include <Scene/Components/LightComponent.h>

using namespace std;
using namespace glm;

REGISTER_OBJECT_CLASS(LightGenerator);

LightGenerator::LightGenerator(ObjectInitializer *initializer) noexcept : Object(initializer)
{
	ArgumentMapType::iterator it;
	const char *ptr = nullptr;

	if (((it = initializer->arguments.find("light_count")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_lightCount = atoi(ptr);
	else
		_lightCount = 1024;

	if (((it = initializer->arguments.find("light_radius")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_lightRadius = (float)atof(ptr);
	else
		_lightRadius = 20.f;

	if (((it = initializer->arguments.find("max_bounds")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		AssetLoader::ReadFloatArray(ptr, 3, &_lightMaxBounds.x);
	else
		_lightMaxBounds = vec3(135.0f, 170.0f, 60.0f);

	if (((it = initializer->arguments.find("min_bounds")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		AssetLoader::ReadFloatArray(ptr, 3, &_lightMinBounds.x);
	else
		_lightMinBounds = vec3(-135.0f, -20.0f, -60.0f);
}

int LightGenerator::Load()
{
	int ret = Object::Load();
	if (ret != ENGINE_OK)
		return ret;

	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<> dis(0, 1);

	for (int i = 0; i < _lightCount; ++i)
	{
		ObjectInitializer oi = {};
		oi.id = 1000 + i;
		oi.position = _RandomPosition(dis, gen);

		Object *obj = Engine::NewObject("Object", &oi);

		vec3 color = vec3(dis(gen), dis(gen), dis(gen));

		char buff[1024];
		(void)snprintf(buff, 1024, "%.02f, %.02f, %.02f", color.x, color.y, color.z);

		ComponentInitializer ci = {};
		ci.parent = obj;
		ci.arguments.insert(make_pair("color", buff));
		ci.arguments.insert(make_pair("type", "point"));

		(void)snprintf(buff, 1024, "%.02f, %.02f", _lightRadius - 10, _lightRadius);
		ci.arguments.insert(make_pair("radius", buff));

		LightComponent *comp = (LightComponent *)Engine::NewComponent("LightComponent", &ci);

		obj->AddComponent("Light", comp);

		_lights.Add(obj);
	}

	return ENGINE_OK;
}

void LightGenerator::Update(double deltaTime) noexcept
{
	for (size_t i = 0; i < _lights.Count(); ++i)
	{
		vec3 pos = _lights[i]->GetPosition();
		pos.y = (float)fmod((pos.y + (-4.5f * deltaTime) - _lightMinBounds[1] + _lightMaxBounds[1]), _lightMaxBounds[1]) + _lightMinBounds[1];
		_lights[i]->SetPosition(pos);
	}
}

vec3 LightGenerator::_RandomPosition(uniform_real_distribution<> dis, mt19937 gen)
{
	vec3 pos = vec3(0.f);

	for (int i = 0; i < 3; ++i)
		pos[i] = (float)dis(gen) * (_lightMaxBounds[i] - _lightMinBounds[i]) + _lightMinBounds[i];

	return pos;
}

LightGenerator::~LightGenerator() noexcept
{
}