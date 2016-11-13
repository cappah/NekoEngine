/* NekoEngine
 *
 * LightComponent.cpp
 * Author: Alexandru Naiman
 *
 * Light component
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman
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

#include <Scene/Object.h>
#include <Scene/Components/LightComponent.h>
#include <System/AssetLoader/AssetLoader.h>

using namespace glm;

ENGINE_REGISTER_COMPONENT_CLASS(LightComponent);

LightComponent::LightComponent(ComponentInitializer *initializer) :
	ObjectComponent(initializer)
{
	if ((_light = Renderer::GetInstance()->AllocLight()) == nullptr)
	{ DIE("Maximum number of lights exceeded"); }

	_light->position = vec4(0.f);
	_light->color = vec4(1.f);
	_light->data = vec4(0.f);
	
	ArgumentMapType::iterator it;
	const char *ptr = nullptr;
	
	if (((it = initializer->arguments.find("color")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		AssetLoader::ReadFloatArray(ptr, 3, &_light->color.x);
	
	if (((it = initializer->arguments.find("intensity")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_light->color.a = (float)atof(ptr);

	if (((it = initializer->arguments.find("radius")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		AssetLoader::ReadFloatArray(ptr, 2, &_light->data.x);
	
	if (((it = initializer->arguments.find("direction")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		AssetLoader::ReadFloatArray(ptr, 3, &_light->direction.x);
	
	if (((it = initializer->arguments.find("angle")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
	{
		AssetLoader::ReadFloatArray(ptr, 2, &_light->data.z);

		_light->data.z = cos(radians(_light->data.z));
		_light->data.w = cos(radians(_light->data.w));
	}

	if (((it = initializer->arguments.find("type")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
	{
		size_t len = strlen(ptr);

		if (!strncmp(ptr, "directional", len))
			_light->position.w = LT_Directional;
		else if (!strncmp(ptr, "point", len))
			_light->position.w = LT_Point;
		else if (!strncmp(ptr, "spot", len))
			_light->position.w = LT_Spot;
	}

	_light->position = vec4(_parent->GetPosition(), _light->position.w);
}

void LightComponent::UpdatePosition() noexcept
{
	ObjectComponent::UpdatePosition();
	_light->position = vec4(_parent->GetPosition(), 1.f);
}