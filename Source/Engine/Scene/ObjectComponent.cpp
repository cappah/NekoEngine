/* NekoEngine
 *
 * ObjectComponent.cpp
 * Author: Alexandru Naiman
 *
 * ObjectComponent class implementation
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

#include <vector>

#include <Scene/Object.h>
#include <Scene/ObjectComponent.h>
#include <Scene/Components/CameraComponent.h>

using namespace std;
using namespace glm;

ObjectComponent::ObjectComponent(ComponentInitializer *initializer)
	: _parent(initializer->parent),
	_loaded(false),
	_enabled(true),
	_visible(true),
	_attachedToCamera(false),
	_cmdBuffer(VK_NULL_HANDLE)
{
	SetPosition(initializer->position);
	SetRotation(initializer->rotation);
	SetScale(initializer->scale);
}

void ObjectComponent::SetPosition(vec3 &position) noexcept
{
	_position = position;

	/*quat invRot = _parent->GetRotation();
	invRot = inverse(invRot);*/

//	_worldPosition rotate()
}

void ObjectComponent::SetRotation(vec3 &rotation) noexcept
{
	_rotation = rotation;
}

void ObjectComponent::SetScale(vec3 &newScale) noexcept
{
	_scale = newScale;
}

int ObjectComponent::InitializeComponent()
{
	if (_parent->GetComponentsOfType<CameraComponent>().size()) {
		_parent->SetNoCull(true);
		_attachedToCamera = true;
	}

	return ENGINE_OK;
}

bool ObjectComponent::Unload()
{
	if (_loaded) {
		_loaded = false;
		return true;
	}

	return false;
}
