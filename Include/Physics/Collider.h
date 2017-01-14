/* NekoEngine
 *
 * Collider.h
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

#include <Engine/Defs.h>

class Object;
class StaticMesh;

class Collider
{
public:
	Collider(class Object *parent) { _parent = parent; }

	Object *GetParent() { return _parent; }

	virtual void SetPosition(glm::vec3 &position) { (void)position; }
	virtual void SetRotation(glm::quat &rotation) { (void)rotation; }
	virtual void SetScale(glm::vec3 &scale) { (void)scale; }

	virtual ~Collider() { }

protected:
	Object *_parent;
};

class BoxCollider : public Collider
{
public:
	BoxCollider(class Object *parent, glm::vec3 &halfExtents) : Collider(parent) { _halfExtents = halfExtents; }

	glm::vec3 &GetHalfExtents() { return _halfExtents; }
	virtual void SetHalfExtents(glm::vec3 &halfExtents) { _halfExtents = halfExtents; }

	virtual ~BoxCollider() { }

protected:
	glm::vec3 _halfExtents;
};

class SphereCollider : public Collider
{
public:
	SphereCollider(Object *parent, double radius) : Collider(parent) { _radius = radius; }

	double GetRadius() { return _radius; }
	virtual void SetRadius(double radius) { _radius = radius; }

	virtual ~SphereCollider() { }

protected:
	double _radius;
};

class CapsuleCollider : public Collider
{
public:
	CapsuleCollider(Object *parent, double radius, double height) : Collider(parent) { _radius = radius; _height = height; }

	double GetRadius() { return _radius; }
	double GetHeight() { return _height; }
	virtual void SetRadius(double radius) { _radius = radius; }
	virtual void SetHeight(double height) { _height = height; }
	virtual void SetRadiusAndHeight(double radius, double height) { _radius = radius; _height = height; }

	virtual ~CapsuleCollider() { }

protected:
	double _radius, _height;
};

class MeshCollider : public Collider
{
public:
	MeshCollider(Object *parent, const StaticMesh *mesh) : Collider(parent) { _mesh = mesh; }

	const StaticMesh *GetMesh() const { return _mesh; }
	virtual void SetMesh(const StaticMesh *mesh) { _mesh = mesh; }

	virtual ~MeshCollider() { }

protected:
	const StaticMesh *_mesh;
};