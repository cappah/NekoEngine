/* NekoEngine
 *
 * PhysXColliders.h
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

#pragma once

#include <PxPhysicsAPI.h>
#include <Scene/Object.h>
#include <Physics/Collider.h>
#include <Renderer/StaticMesh.h>

/*class PhysXBoxCollider : public BoxCollider
{
public:
	PhysXBoxCollider(Object *parent, glm::vec3 &halfExtents);

	//PxCol *GetCollisionObject() { return _object; }

	virtual void SetHalfExtents(glm::vec3 &halfExtents) override;

	virtual void SetPosition(glm::vec3 &position) override;
	virtual void SetRotation(glm::quat &rotation) override;
	virtual void SetScale(glm::vec3 &scale) override;

	virtual ~PhysXBoxCollider();

private:
	physx::PxShape *_shape;
	//btCollisionObject *_object;	
};

class PhysXSphereCollider : public SphereCollider
{
public:
	PhysXSphereCollider(Object *parent, double radius);

	//btCollisionObject *GetCollisionObject() { return _object; }

	virtual void SetRadius(double radius) override;

	virtual void SetPosition(glm::vec3 &position) override;
	virtual void SetRotation(glm::quat &rotation) override;
	virtual void SetScale(glm::vec3 &scale) override;

	virtual ~PhysXSphereCollider();

private:
	PxShape *_shape;
//	btCollisionObject *_object;	
};

class PhysXCapsuleCollider : public CapsuleCollider
{
public:
	PhysXCapsuleCollider(Object *parent, double radius, double height);

//	btCollisionObject *GetCollisionObject() { return _object; }

	virtual void SetRadius(double radius) override;
	virtual void SetHeight(double height) override;

	virtual void SetPosition(glm::vec3 &position) override;
	virtual void SetRotation(glm::quat &rotation) override;
	virtual void SetScale(glm::vec3 &scale) override;

	virtual ~PhysXCapsuleCollider();

private:
	PxShape *_shape;
//	btCollisionObject *_object;	
};

class PhysXMeshCollider : public MeshCollider
{
public:
	PhysXMeshCollider(Object *parent, const StaticMesh *mesh);

//	btCollisionObject *GetCollisionObject() { return _object; }

	virtual void SetMesh(const StaticMesh *mesh) override;

	virtual void SetPosition(glm::vec3 &position) override;
	virtual void SetRotation(glm::quat &rotation) override;
	virtual void SetScale(glm::vec3 &scale) override;

	virtual ~PhysXMeshCollider();

private:
	PxShape *_shape;
//	btCollisionObject *_object;	
};*/