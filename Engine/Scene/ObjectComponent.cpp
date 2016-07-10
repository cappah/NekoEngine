/* Neko Engine
 *
 * ObjectComponent.cpp
 * Author: Alexandru Naiman
 *
 * ObjectComponent class implementation
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

#define ENGINE_INTERNAL

#include <Scene/Object.h>
#include <Scene/ObjectComponent.h>

#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

ObjectComponent::ObjectComponent(ComponentInitializer *initializer)
	: _parent(initializer->parent),
	_position(vec3(0.f)),
	_rotation(vec3(0.f)),
	_scale(vec3(1.f))
{
	ArgumentMapType::iterator it;
	const char *ptr = nullptr;

	if (((it = initializer->arguments.find("position")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		EngineUtils::ReadFloatArray(ptr, 3, &_position.x);

	if (((it = initializer->arguments.find("rotation")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		EngineUtils::ReadFloatArray(ptr, 3, &_rotation.x);

	if (((it = initializer->arguments.find("scale")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		EngineUtils::ReadFloatArray(ptr, 3, &_scale.x);
}

void ObjectComponent::SetPosition(vec3 &position) noexcept
{
	_position = position;
}

void ObjectComponent::SetRotation(vec3 &rotation) noexcept
{
	_rotation = rotation;
}

void ObjectComponent::SetScale(vec3 &newScale) noexcept
{
	_scale = newScale;
}

