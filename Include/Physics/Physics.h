/* NekoEngine
 *
 * Physics.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Physics Module Interface
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

#include <stdint.h>
#include <Engine/Defs.h>

#include <Physics/Ray.h>
#include <Physics/Collider.h>

class Object;

enum BroadphaseType
{
	SAP,
	MBP
};

class ENGINE_API Physics
{
public:
	virtual int Initialize() = 0;

	virtual int InitScene(BroadphaseType broadphase, double sceneSize, uint32_t maxObjects) = 0;

	virtual BoxCollider *CreateBoxCollider(Object *parent, const glm::vec3 &halfExtents) = 0;
	virtual SphereCollider *CreateSphereCollider(Object *parent, double radius) = 0;
	virtual CapsuleCollider *CreateCapsuleCollider(Object *parent, double radius, double height) = 0;
	virtual MeshCollider *CreateMeshCollider(Object *parent, const StaticMesh *mesh) = 0;
	
	virtual bool RayCast(Ray *ray) = 0;
	virtual bool ScreenRayCast(Ray *ray, glm::vec2 &screenCoords, float distance) = 0;

	virtual void Update(double deltaTime) = 0;

	virtual void Release() = 0;

	virtual ~Physics();

	static int InitInstance(const char *module);
	static Physics *GetInstance();
	static void ReleaseInstance();
};
