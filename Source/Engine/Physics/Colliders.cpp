/* NekoEngine
 *
 * Colliders.cpp
 * Author: Alexandru Naiman
 *
 * Bullet physics Module
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

#include <Engine/Engine.h>
#include <Physics/Physics.h>
#include <Physics/Collider.h>

#include <System/Logger.h>

#define BLT_MODULE	"BulletPhysics"

using namespace glm;

Collider::Collider(Object *parent)
{
	_parent = parent;
	_shape = nullptr;
	_object = nullptr;
}

void Collider::SetPosition(const vec3 &position)
{
	btVector3 vec(position.x, position.y, position.z);
	_object->getWorldTransform().setOrigin(vec);
}

void Collider::SetRotation(const quat &rotation)
{
	btQuaternion quat(rotation.x, rotation.y, rotation.z, rotation.w);
	_object->getWorldTransform().setRotation(quat);
}

void Collider::SetScale(const vec3 &scale)
{
	btVector3 vec(scale.x, scale.y, scale.z);
	_shape->setLocalScaling(vec);
}

Collider::~Collider()
{
	Physics::RemoveCollisionObject(_object);

	delete _shape;
	delete _object;
}

/*********************
 * Box Collider
 *********************/

BoxCollider::BoxCollider(Object *parent, const vec3 &halfExtents) :
	Collider(parent),
	_halfExtents(halfExtents)
{
	_object = new btCollisionObject();
	_object->setUserPointer(_parent);

	SetHalfExtents(halfExtents);
}

void BoxCollider::SetHalfExtents(const glm::vec3 &halfExtents)
{
	delete _shape;
	btVector3 vec{ halfExtents.x, halfExtents.y, halfExtents.z };
	_halfExtents = halfExtents;
	_shape = new btBoxShape(vec);
	_object->setCollisionShape(_shape);
}

BoxCollider::~BoxCollider() { }

/*********************
 * Sphere Collider
 *********************/

SphereCollider::SphereCollider(Object *parent, double radius) :
	Collider(parent),
	_radius(radius)
{
	_object = new btCollisionObject();
	_object->setUserPointer(_parent);

	SetRadius(radius);
}

void SphereCollider::SetRadius(double radius)
{
	delete _shape;
	_radius = radius;
	_shape = new btSphereShape(radius);
	_object->setCollisionShape(_shape);
}

SphereCollider::~SphereCollider() { }

/*********************
 * Capsule Collider
 *********************/

CapsuleCollider::CapsuleCollider(Object *parent, double radius, double height) :
	Collider(parent),
	_radius(radius),
	_height(height)
{
	_object = new btCollisionObject();
	_object->setUserPointer(_parent);

	SetRadiusAndHeight(radius, height);
}

void CapsuleCollider::SetRadius(double radius)
{
	delete _shape;
	_radius = radius;	
	_shape = new btCapsuleShape(radius, _height);
	_object->setCollisionShape(_shape);
}

void CapsuleCollider::SetHeight(double height)
{
	delete _shape;
	_height = height;	
	_shape = new btCapsuleShape(_radius, height);
	_object->setCollisionShape(_shape);
}

void CapsuleCollider::SetRadiusAndHeight(double radius, double height)
{
	delete _shape;
	_radius = radius;
	_height = height;
	_shape = new btCapsuleShape(radius, height);
	_object->setCollisionShape(_shape);
}

CapsuleCollider::~CapsuleCollider() { }