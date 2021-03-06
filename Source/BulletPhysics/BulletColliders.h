/* NekoEngine
 *
 * BulletColliders.h
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

#include <Scene/Object.h>
#include <Physics/Collider.h>
#include <Renderer/StaticMesh.h>
#include <bullet/btBulletCollisionCommon.h>

class BulletBoxCollider : public BoxCollider
{
public:
	BulletBoxCollider(Object *parent, const glm::vec3 &halfExtents);

	btCollisionObject *GetCollisionObject() { return _object; }

	virtual void SetHalfExtents(const glm::vec3 &halfExtents) override;

	virtual void SetPosition(const glm::vec3 &position) override;
	virtual void SetRotation(const glm::quat &rotation) override;
	virtual void SetScale(const glm::vec3 &scale) override;

	virtual ~BulletBoxCollider();

private:
	btBoxShape *_shape;
	btCollisionObject *_object;	
};

class BulletSphereCollider : public SphereCollider
{
public:
	BulletSphereCollider(Object *parent, double radius);

	btCollisionObject *GetCollisionObject() { return _object; }

	virtual void SetRadius(double radius) override;

	virtual void SetPosition(const glm::vec3 &position) override;
	virtual void SetRotation(const glm::quat &rotation) override;
	virtual void SetScale(const glm::vec3 &scale) override;

	virtual ~BulletSphereCollider();

private:
	btSphereShape *_shape;
	btCollisionObject *_object;	
};

class BulletCapsuleCollider : public CapsuleCollider
{
public:
	BulletCapsuleCollider(Object *parent, double radius, double height);

	btCollisionObject *GetCollisionObject() { return _object; }

	virtual void SetRadius(double radius) override;
	virtual void SetHeight(double height) override;
	virtual void SetRadiusAndHeight(double radius, double height) override;

	virtual void SetPosition(const glm::vec3 &position) override;
	virtual void SetRotation(const glm::quat &rotation) override;
	virtual void SetScale(const glm::vec3 &scale) override;

	virtual ~BulletCapsuleCollider();

private:
	btCapsuleShape *_shape;
	btCollisionObject *_object;	
};

class BulletMeshCollider : public MeshCollider
{
public:
	BulletMeshCollider(Object *parent, const StaticMesh *mesh);

	btCollisionObject *GetCollisionObject() { return _object; }

	virtual void SetMesh(const StaticMesh *mesh) override;

	virtual void SetPosition(const glm::vec3 &position) override;
	virtual void SetRotation(const glm::quat &rotation) override;
	virtual void SetScale(const glm::vec3 &scale) override;

	virtual ~BulletMeshCollider();

private:
	btTriangleIndexVertexArray *_ivArray;
	btConvexTriangleMeshShape *_shape;
	btCollisionObject *_object;	
};
