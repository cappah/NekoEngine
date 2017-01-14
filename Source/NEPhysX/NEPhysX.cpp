/* NekoEngine
 *
 * NEPhysX.cpp
 * Author: Alexandru Naiman
 *
 * NekoEngine PhysX Module
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

#include "NEPhysX.h"
#include "version.h"
#include <System/Logger.h>
#include <Platform/PlatformDetect.h>

#define NEHPYSX_MODULE	"NEPhysX"

using namespace physx;

class NEPhysXAllocatorCallback : public PxAllocatorCallback
{
public:
	virtual void *allocate(size_t size, const char *type, const char *file, int line)
	{
	#ifdef NE_PLATFORM_WINDOWS
		void *ptr = _aligned_malloc(size, 16);
	#else
		void *ptr = malloc(size);
	#endif
		if (!ptr)
			return nullptr;
		return ptr;
	}

	virtual void deallocate(void *ptr)
	{
		free(ptr);
	}
};

class NEPhysXErrorCallback : public PxErrorCallback
{
public:
	virtual void reportError(PxErrorCode::Enum code, const char *message, const char *file, int line)
	{
		Logger::Log(NEHPYSX_MODULE, LOG_CRITICAL, message);
	}
};

static NEPhysXAllocatorCallback _allocatorCallback{};
static NEPhysXErrorCallback _errorCallback{};

static PxFoundation *_foundation{ nullptr };
static PxPhysics *_physics{ nullptr };
static PxCooking *_cooking{ nullptr };

int NEPhysX::Initialize()
{
	if ((_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, _allocatorCallback, _errorCallback)) == nullptr)
	{
		Logger::Log(NEHPYSX_MODULE, LOG_CRITICAL, "Failed to create foundation");
		return ENGINE_FAIL;
	}

	if ((_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *_foundation, PxTolerancesScale())) == nullptr)
	{
		Logger::Log(NEHPYSX_MODULE, LOG_CRITICAL, "Failed to create physics object");
		return ENGINE_FAIL;
	}

	if ((_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *_foundation, PxCookingParams(PxTolerancesScale()))))
	{
		Logger::Log(NEHPYSX_MODULE, LOG_CRITICAL, "Failed to create cooking");
		return ENGINE_FAIL;
	}

	Logger::Log(NEHPYSX_MODULE, LOG_INFORMATION, "Initialized");
	Logger::Log(NEHPYSX_MODULE, LOG_INFORMATION, "Module version: %s, using PhysX %d.%d", NEPHYSX_VERSION_STRING, PX_PHYSICS_VERSION_MAJOR, PX_PHYSICS_VERSION_MINOR);

	return ENGINE_OK;
}

int InitScene(BroadphaseType broadphase, double sceneSize, uint32_t maxObjects)
{
	return ENGINE_OK;
}

BoxCollider *NEPhysX::CreateBoxCollider(Object *parent, glm::vec3 &halfExtents)
{
	return nullptr;
}

SphereCollider *NEPhysX::CreateSphereCollider(Object *parent, double radius)
{
	return nullptr;
}

CapsuleCollider *NEPhysX::CreateCapsuleCollider(Object *parent, double radius, double height)
{
	return nullptr;
}

MeshCollider *NEPhysX::CreateMeshCollider(Object *parent, const StaticMesh *mesh)
{
	return nullptr;
}

void NEPhysX::Update(double deltaTime)
{
	//
}

void NEPhysX::Release()
{
	//
}

NEPhysX::~NEPhysX()
{
	//
}