/* NekoEngine
 *
 * NullPhysics.cpp
 * Author: Alexandru Naiman
 *
 * NekoEngine NullPhysics Module
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

#include "NullPhysics.h"

int NullPhysics::Initialize() { return ENGINE_OK; }
int NullPhysics::InitScene(BroadphaseType broadphase, double sceneSize, uint32_t maxObjects) { return ENGINE_OK; }
BoxCollider *NullPhysics::CreateBoxCollider(Object *parent, const glm::vec3 &halfExtents) { return new BoxCollider(parent, halfExtents); }
SphereCollider *NullPhysics::CreateSphereCollider(Object *parent, double radius) { return new SphereCollider(parent, radius); }
CapsuleCollider *NullPhysics::CreateCapsuleCollider(Object *parent, double radius, double height) { return new CapsuleCollider(parent, radius, height); }
MeshCollider *NullPhysics::CreateMeshCollider(Object *parent, const StaticMesh *mesh) { return new MeshCollider(parent, mesh); }
bool NullPhysics::RayCast(Ray *ray) { (void)ray; return false; }
bool NullPhysics::ScreenRayCast(Ray *ray, glm::vec2 &screenCoords, float distance) { (void)ray; (void)screenCoords; (void)distance; return false; }
void NullPhysics::Update(double deltaTime) { (void)deltaTime; }
void NullPhysics::Release() { }
NullPhysics::~NullPhysics() { }

#if defined(_WIN32) || defined(_WIN64)
	#define EXPORT __declspec(dllexport)
#else
	#define EXPORT
#endif

extern "C" EXPORT Physics *createPhysics() { return (Physics *)new NullPhysics(); }