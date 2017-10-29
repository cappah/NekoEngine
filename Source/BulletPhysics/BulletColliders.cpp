/* NekoEngine
 *
 * Colliders.cpp
 * Author: Alexandru Naiman
 *
 * Bullet physics Module colliders
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

#include "BulletPhysics.h"
#include "BulletColliders.h"

#include <System/Logger.h>

#define BLT_MODULE	"BulletPhysics"

/*********************
 * Box Collider
 *********************/

BulletBoxCollider::BulletBoxCollider(Object *parent, const glm::vec3 &halfExtents) :
	BoxCollider(parent, halfExtents),
	_shape(nullptr),
	_object(nullptr)
{
	_object = new btCollisionObject();
	_object->setUserPointer(_parent);

	SetHalfExtents(halfExtents);
}

void BulletBoxCollider::SetHalfExtents(const glm::vec3 &halfExtents)
{
	BoxCollider::SetHalfExtents(halfExtents);
	btVector3 vec{ halfExtents.x, halfExtents.y, halfExtents.z };
	delete _shape;
	_shape = new btBoxShape(vec);
	_object->setCollisionShape(_shape);
}

void BulletBoxCollider::SetPosition(const glm::vec3 &position)
{
	btVector3 vec(position.x, position.y, position.z);
	_object->getWorldTransform().setOrigin(vec);
}

void BulletBoxCollider::SetRotation(const glm::quat &rotation)
{
	btQuaternion quat(rotation.x, rotation.y, rotation.z, rotation.w);
	_object->getWorldTransform().setRotation(quat);
}

void BulletBoxCollider::SetScale(const glm::vec3 &scale)
{
	btVector3 vec(scale.x, scale.y, scale.z);
	_shape->setLocalScaling(vec);
}

BulletBoxCollider::~BulletBoxCollider()
{
	((BulletPhysics *)Physics::GetInstance())->RemoveCollisionObject(_object);

	delete _shape;
	delete _object;
}

/*********************
 * Sphere Collider
 *********************/

BulletSphereCollider::BulletSphereCollider(Object *parent, double radius) :
	SphereCollider(parent, radius),
	_shape(nullptr),
	_object(nullptr)
{
	_object = new btCollisionObject();
	_object->setUserPointer(_parent);

	SetRadius(radius);
}

void BulletSphereCollider::SetRadius(double radius)
{
	SphereCollider::SetRadius(radius);
	delete _shape;
	_shape = new btSphereShape(radius);
	_object->setCollisionShape(_shape);
}

void BulletSphereCollider::SetPosition(const glm::vec3 &position)
{
	btVector3 vec(position.x, position.y, position.z);
	_object->getWorldTransform().setOrigin(vec);
}

void BulletSphereCollider::SetRotation(const glm::quat &rotation)
{
	btQuaternion quat(rotation.x, rotation.y, rotation.z, rotation.w);
	_object->getWorldTransform().setRotation(quat);
}

void BulletSphereCollider::SetScale(const glm::vec3 &scale)
{
	btVector3 vec(scale.x, scale.y, scale.z);
	_shape->setLocalScaling(vec);
}

BulletSphereCollider::~BulletSphereCollider()
{
	((BulletPhysics *)Physics::GetInstance())->RemoveCollisionObject(_object);

	delete _shape;
	delete _object;
}

/*********************
 * Capsule Collider
 *********************/

BulletCapsuleCollider::BulletCapsuleCollider(Object *parent, double radius, double height) :
	CapsuleCollider(parent, radius, height),
	_shape(nullptr),
	_object(nullptr)
{
	_object = new btCollisionObject();
	_object->setUserPointer(_parent);

	SetRadiusAndHeight(radius, height);
}

void BulletCapsuleCollider::SetRadius(double radius)
{
	CapsuleCollider::SetRadius(radius);
	delete _shape;
	_shape = new btCapsuleShape(radius, _height);
	_object->setCollisionShape(_shape);
}

void BulletCapsuleCollider::SetHeight(double height)
{
	CapsuleCollider::SetHeight(height);
	delete _shape;
	_shape = new btCapsuleShape(_radius, height);
	_object->setCollisionShape(_shape);
}

void BulletCapsuleCollider::SetRadiusAndHeight(double radius, double height)
{
	CapsuleCollider::SetRadiusAndHeight(radius, height);
	delete _shape;
	_shape = new btCapsuleShape(radius, height);
	_object->setCollisionShape(_shape);
}

void BulletCapsuleCollider::SetPosition(const glm::vec3 &position)
{
	btVector3 vec(position.x, position.y, position.z);
	_object->getWorldTransform().setOrigin(vec);
}

void BulletCapsuleCollider::SetRotation(const glm::quat &rotation)
{
	btQuaternion quat(rotation.x, rotation.y, rotation.z, rotation.w);
	_object->getWorldTransform().setRotation(quat);
}

void BulletCapsuleCollider::SetScale(const glm::vec3 &scale)
{
	btVector3 vec(scale.x, scale.y, scale.z);
	_shape->setLocalScaling(vec);
}

BulletCapsuleCollider::~BulletCapsuleCollider()
{
	((BulletPhysics *)Physics::GetInstance())->RemoveCollisionObject(_object);

	delete _shape;
	delete _object;
}

/*********************
 * btStridingMeshInterface
 *********************/

class StaticMeshStrider : public btStridingMeshInterface
{
public:
	StaticMeshStrider(const StaticMesh *mesh) : _mesh(mesh) { }

	void SetMesh(const StaticMesh *mesh) { _mesh = mesh; }

	virtual int getNumSubParts() const override { return _mesh->GetGroupCount(); }
	virtual void getLockedVertexIndexBase(unsigned char **vertexbase, int &numverts, PHY_ScalarType &type, int &stride, unsigned char **indexbase, int &indexstride, int &numfaces, PHY_ScalarType &indicestype, int subpart = 0) override
	{
		//
	}
	virtual void getLockedReadOnlyVertexIndexBase(const unsigned char **vertexbase, int &numverts, PHY_ScalarType &type, int &stride, const unsigned char **indexbase, int &indexstride, int &numfaces, PHY_ScalarType &indicestype, int subpart = 0) const override
	{
		type = PHY_FLOAT;
		indicestype = PHY_INTEGER;
		stride = (int)sizeof(Vertex);
		indexstride = (int)(sizeof(uint32_t) * 3);
		numfaces = _mesh->GetGroupIndexCount(subpart) / 3;
		numverts = _mesh->GetGroupVertexCount(subpart);
		*indexbase = (const unsigned char *)_mesh->GetIndices().data() + _mesh->GetGroupIndexOffset(subpart);
		*vertexbase = (const unsigned char *)_mesh->GetVertices().data() + _mesh->GetGroupVertexOffset(subpart);
	}
	virtual void unLockVertexBase(int subpart) override { }
	virtual void unLockReadOnlyVertexBase(int subpart) const override { }
	virtual void preallocateVertices(int numverts) override { }
	virtual void preallocateIndices(int numindices) override { }

	virtual ~StaticMeshStrider() { }
private:
	const StaticMesh *_mesh;
};

/*********************
 * Mesh Collider
 *********************/

BulletMeshCollider::BulletMeshCollider(Object *parent, const StaticMesh *mesh) :
	MeshCollider(parent, mesh),
	_ivArray(nullptr),
	_shape(nullptr),
	_object(nullptr)
{
	_object = new btCollisionObject();	
	_object->setUserPointer(_parent);

	SetMesh(mesh);
}

void BulletMeshCollider::SetMesh(const StaticMesh *mesh)
{
	MeshCollider::SetMesh(mesh);

	delete _ivArray;
	delete _shape;

	btIndexedMesh indexedMesh{};
	indexedMesh.m_numTriangles = mesh->GetTriangleCount();
	indexedMesh.m_numVertices = mesh->GetVertexCount();
	indexedMesh.m_triangleIndexStride = sizeof(uint32_t) * 3;
	indexedMesh.m_vertexStride = sizeof(Vertex);
	indexedMesh.m_vertexType = PHY_FLOAT;
	indexedMesh.m_triangleIndexBase = (const unsigned char *)_mesh->GetIndices().data();
	indexedMesh.m_vertexBase = (const unsigned char *)_mesh->GetVertices().data();

	_ivArray = new btTriangleIndexVertexArray();
	_ivArray->addIndexedMesh(indexedMesh);

	_shape = new btConvexTriangleMeshShape(_ivArray, true);
	_object->setCollisionShape(_shape);
}

void BulletMeshCollider::SetPosition(const glm::vec3 &position)
{
	btVector3 vec(position.x, position.y, position.z);
	_object->getWorldTransform().setOrigin(vec);
}

void BulletMeshCollider::SetRotation(const glm::quat &rotation)
{
	btQuaternion quat(rotation.x, rotation.y, rotation.z, rotation.w);
	_object->getWorldTransform().setRotation(quat);
}

void BulletMeshCollider::SetScale(const glm::vec3 &scale)
{
	btVector3 vec(scale.x, scale.y, scale.z);
	_shape->setLocalScaling(vec);
}

BulletMeshCollider::~BulletMeshCollider()
{
	((BulletPhysics *)Physics::GetInstance())->RemoveCollisionObject(_object);

	delete _shape;
	delete _object;
}
