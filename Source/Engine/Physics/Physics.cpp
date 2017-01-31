/* NekoEngine
 *
 * Physics.cpp
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

#include <Scene/Object.h>
#include <System/Logger.h>
#include <Physics/Physics.h>
#include <Physics/Collider.h>
#include <Engine/CameraManager.h>

#define BLT_MODULE	"BulletPhysics"

using namespace glm;

static btCollisionConfiguration *_configuration{ nullptr };
static btCollisionDispatcher *_dispatcher{ nullptr };
static btBroadphaseInterface *_broadphase{ nullptr };
static btCollisionWorld *_world{ nullptr };

int Physics::Initialize()
{
	if ((_configuration = new btDefaultCollisionConfiguration()) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;

	if ((_dispatcher = new btCollisionDispatcher(_configuration)) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;
	
	Logger::Log(BLT_MODULE, LOG_INFORMATION, "Initialized");
	Logger::Log(BLT_MODULE, LOG_INFORMATION, "Using Bullet %d", BT_BULLET_VERSION);

	return ENGINE_OK;
}

int Physics::InitScene(BroadphaseType broadphase, double sceneSize, uint32_t maxObjects)
{
	btVector3 min(-sceneSize, -sceneSize, -sceneSize);
	btVector3 max(sceneSize, sceneSize, sceneSize);

	if ((_broadphase = new bt32BitAxisSweep3(min, max)) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;

	if ((_world = new btCollisionWorld(_dispatcher, _broadphase, _configuration)) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;
	
	return ENGINE_OK;
}

BoxCollider *Physics::CreateBoxCollider(Object *parent, const vec3 &halfExtents)
{
	BoxCollider *collider{ new BoxCollider(parent, halfExtents) };
	_world->addCollisionObject(collider->GetCollisionObject());
	return (BoxCollider *)collider;
}

SphereCollider *Physics::CreateSphereCollider(Object *parent, double radius)
{
	SphereCollider *collider{ new SphereCollider(parent, radius) };
	_world->addCollisionObject(collider->GetCollisionObject());
	return (SphereCollider *)collider;
}

CapsuleCollider *Physics::CreateCapsuleCollider(Object *parent, double radius, double height)
{
	CapsuleCollider *collider{ new CapsuleCollider(parent, radius, height) };
	_world->addCollisionObject(collider->GetCollisionObject());
	return (CapsuleCollider *)collider;
}

bool Physics::RayCast(Ray *ray)
{
	btVector3 rayStart{ ray->start.x, ray->start.y, ray->start.z };
	btVector3 rayEnd{ ray->end.x, ray->end.y, ray->end.z };

	btCollisionWorld::ClosestRayResultCallback callback(rayStart, rayEnd);

	_world->rayTest(rayStart, rayEnd, callback);

	if (!callback.hasHit())
		return false;

	ray->hitPoint = vec3(callback.m_hitPointWorld.x(), callback.m_hitPointWorld.y(), callback.m_hitPointWorld.z());
	ray->hitNormal = vec3(callback.m_hitNormalWorld.x(), callback.m_hitNormalWorld.y(), callback.m_hitNormalWorld.z());
	ray->hitObject = (Object *)callback.m_collisionObject->getUserPointer();

	return true;
}

bool Physics::ScreenRayCast(Ray *ray, vec2 &screenCoords, float distance)
{
	vec3 coords = vec3(screenCoords, 0.f);
	vec4 viewport = vec4(0.f, 0.f, Engine::GetScreenWidth(), Engine::GetScreenHeight());

	ray->start = unProject(coords, CameraManager::GetActiveCamera()->GetView(), CameraManager::GetActiveCamera()->GetProjectionMatrix(), viewport);
	ray->end = ray->start + distance * CameraManager::GetActiveCamera()->GetForward();
	
	return RayCast(ray);
}

void Physics::Update(double deltaTime)
{
	_world->performDiscreteCollisionDetection();

	int manifolds{ _dispatcher->getNumManifolds() };

	for (int i = 0; i < manifolds; ++i)
	{
		btPersistentManifold *manifold{ _dispatcher->getManifoldByIndexInternal(i) };
		const btCollisionObject *a{ manifold->getBody0() };
		const btCollisionObject *b{ manifold->getBody1() };
		manifold->refreshContactPoints(a->getWorldTransform(), b->getWorldTransform());

		int contacts{ manifold->getNumContacts() };

		for (int j = 0; j < contacts; ++j)
		{
			btManifoldPoint &contactPoint{ manifold->getContactPoint(j) };
			glm::vec3 hitPos{ contactPoint.getPositionWorldOnA().x(), contactPoint.getPositionWorldOnA().y(), contactPoint.getPositionWorldOnA().z() };
			((Object *)a->getUserPointer())->OnHit((Object *)b->getUserPointer(), hitPos);
			hitPos = { contactPoint.getPositionWorldOnB().x(), contactPoint.getPositionWorldOnB().y(), contactPoint.getPositionWorldOnB().z() };
			((Object *)b->getUserPointer())->OnHit((Object *)a->getUserPointer(), hitPos);
		}
	}
}

void Physics::RemoveCollisionObject(btCollisionObject *object)
{
	_world->removeCollisionObject(object);
}

void Physics::Release()
{
	delete _world;
	delete _dispatcher;
	delete _broadphase;
	delete _configuration;
}