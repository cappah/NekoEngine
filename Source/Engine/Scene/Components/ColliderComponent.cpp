/* NekoEngine
 *
 * ColliderComponent.cpp
 * Author: Alexandru Naiman
 *
 * Collider component
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

#include <Physics/Physics.h>
#include <Engine/EngineUtils.h>
#include <System/AssetLoader/AssetLoader.h>
#include <Scene/Object.h>
#include <Scene/Components/ColliderComponent.h>
#include <Scene/Components/StaticMeshComponent.h>

using namespace glm;

ENGINE_REGISTER_COMPONENT_CLASS(ColliderComponent);

ColliderComponent::ColliderComponent(ComponentInitializer *initializer) :
	ObjectComponent(initializer),
	_collider(nullptr), _colliderType(ColliderType::Box)
{
	ArgumentMapType::iterator it;
	const char *ptr = nullptr;
	vec3 tmp{ vec3(0.f) };
	double d1, d2;

	if (((it = initializer->arguments.find("type")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
	{
		size_t len = strlen(ptr);
		if (!strncmp(ptr, "box", len))
		{
			_colliderType = ColliderType::Box;
			if (((it = initializer->arguments.find("halfextents")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
			{
				EngineUtils::ReadFloatArray(ptr, 3, &tmp.x);
				if ((_collider = (Collider *)Physics::CreateBoxCollider(_parent, tmp)) == nullptr)
				{ DIE("Out of resources"); }
			}			
		}
		else if (!strncmp(ptr, "sphere", len))
		{
			_colliderType = ColliderType::Sphere;
			if (((it = initializer->arguments.find("radius")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
			{
				d1 = atof(ptr);
				if ((_collider = (Collider *)Physics::CreateSphereCollider(_parent, d1)) == nullptr)
				{ DIE("Out of resources"); }
			}			
		}
		else if (!strncmp(ptr, "capsule", len))
		{
			_colliderType = ColliderType::Capsule;
			if (((it = initializer->arguments.find("radius")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
			{
				d1 = atof(ptr);
				if (((it = initializer->arguments.find("height")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
					d2 = atof(ptr);
				if ((_collider = (Collider *)Physics::CreateCapsuleCollider(_parent, d1, d2)) == nullptr)
				{ DIE("Out of resources"); }
			}
		}
	}
}

int ColliderComponent::InitializeComponent()
{
	int ret = ObjectComponent::InitializeComponent();

	if (ret != ENGINE_OK)
		return ret;

	UpdatePosition();

	if (_collider)
		return ENGINE_OK;

	/*switch (_colliderType)
	{
		case ColliderType::Box:
			if ((_collider = Physics::CreateBoxCollider(_parent, _parent->GetBounds().GetBox().GetHalf())) == nullptr)
				return ENGINE_OUT_OF_RESOURCES;
		break;
		case ColliderType::Sphere:
			if ((_collider = Physics::CreateSphereCollider(_parent, _parent->GetBounds().GetSphere().GetRadius())) == nullptr)
				return ENGINE_OUT_OF_RESOURCES;
		break;
		case ColliderType::Capsule:
			if ((_collider = Physics::CreateCapsuleCollider(_parent, _parent->GetBounds().GetBox().GetHalf().x * 2.0, _parent->GetBounds().GetBox().GetHalf().y * 2.0)) == nullptr)
				return ENGINE_OUT_OF_RESOURCES;
		break;
	}*/

	return ENGINE_OK;
}

void ColliderComponent::UpdatePosition() noexcept
{
	ObjectComponent::UpdatePosition();
	if (!_collider) return;
	_collider->SetPosition(_position);
	quat rot = rotate(quat(), radians(_rotation));
	_collider->SetRotation(rot);
	_collider->SetScale(_parent->GetScale() * _localScale);
}

bool ColliderComponent::ColliderComponent::Unload()
{
	if (!ObjectComponent::Unload())
		return false;

	delete _collider;

	return true;
}

ColliderComponent::~ColliderComponent() { }