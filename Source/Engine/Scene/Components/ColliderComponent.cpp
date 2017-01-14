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

#include <Scene/Object.h>
#include <Physics/Physics.h>
#include <System/AssetLoader/AssetLoader.h>
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
			if (((it = initializer->arguments.find("halfextents")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
				AssetLoader::ReadFloatArray(ptr, 3, &tmp.x);
			if ((_collider = (Collider *)Physics::GetInstance()->CreateBoxCollider(_parent, tmp)) == nullptr)
			{ DIE("Out of resources"); }
			_colliderType = ColliderType::Box;
		}
		else if (!strncmp(ptr, "sphere", len))
		{
			if (((it = initializer->arguments.find("radius")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
				d1 = atof(ptr);
			if ((_collider = (Collider *)Physics::GetInstance()->CreateSphereCollider(_parent, d1)) == nullptr)
			{ DIE("Out of resources"); }
			_colliderType = ColliderType::Sphere;
		}
		else if (!strncmp(ptr, "capsule", len))
		{
			if (((it = initializer->arguments.find("radius")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
				d1 = atof(ptr);
			if (((it = initializer->arguments.find("height")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
				d2 = atof(ptr);
			if ((_collider = (Collider *)Physics::GetInstance()->CreateCapsuleCollider(_parent, d1, d2)) == nullptr)
			{ DIE("Out of resources"); }
			_colliderType = ColliderType::Capsule;
		}
		else if (!strncmp(ptr, "mesh", len))
		{
			if (((it = initializer->arguments.find("meshcomponent")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
				_meshComponentName = ptr;
			_colliderType = ColliderType::Mesh;
		}
	}
}

int ColliderComponent::InitializeComponent()
{
	int ret = ObjectComponent::InitializeComponent();

	if(ret != ENGINE_OK)
		return ret;

	if (_colliderType != ColliderType::Mesh)
		return ENGINE_OK;

	StaticMeshComponent *smc{ (StaticMeshComponent*)_parent->GetComponent(*_meshComponentName) };;
	if (!smc)
		return ENGINE_INVALID_ARGS;

	if ((_collider = (Collider *)Physics::GetInstance()->CreateMeshCollider(_parent, smc->GetMesh())) == nullptr)
		return ENGINE_INVALID_ARGS;

	return ENGINE_OK;
}

void ColliderComponent::SetPosition(glm::vec3 &position) noexcept
{
	ObjectComponent::SetPosition(position);
	_collider->SetPosition(_position);
}

void ColliderComponent::SetRotation(glm::vec3 &rotation) noexcept
{
	ObjectComponent::SetRotation(rotation);
	quat rot = rotate(quat(), radians(rotation));
	_collider->SetRotation(rot);
}

void ColliderComponent::SetScale(glm::vec3 &scale) noexcept
{
	ObjectComponent::SetScale(scale);
	_collider->SetScale(scale);
}

void ColliderComponent::Enable(bool enable)
{
	ObjectComponent::Enable(enable);
}

bool ColliderComponent::ColliderComponent::Unload()
{
	if (!ObjectComponent::Unload())
		return false;

	return true;
}

ColliderComponent::~ColliderComponent() { }