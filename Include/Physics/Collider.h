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

#include <Engine/Engine.h>

class Object;
class StaticMesh;

class Collider
{
public:
	Collider(class Object *parent);

	Object *GetParent() { return _parent; }
	btCollisionObject *GetCollisionObject() { return _object; }

	virtual void SetPosition(const glm::vec3 &position);
	virtual void SetRotation(const glm::quat &rotation);
	virtual void SetScale(const glm::vec3 &scale);

	virtual ~Collider();

protected:
	Object *_parent;
	btCollisionShape *_shape;
	btCollisionObject *_object;
};

class BoxCollider : public Collider
{
public:
	BoxCollider(class Object *parent, const glm::vec3 &halfExtents);

	const glm::vec3 &GetHalfExtents() { return _halfExtents; }
	virtual void SetHalfExtents(const glm::vec3 &halfExtents);

	virtual ~BoxCollider();

protected:
	glm::vec3 _halfExtents;
};

class SphereCollider : public Collider
{
public:
	SphereCollider(Object *parent, double radius);

	double GetRadius() { return _radius; }
	virtual void SetRadius(double radius);

	virtual ~SphereCollider();

protected:
	double _radius;
};

class CapsuleCollider : public Collider
{
public:
	CapsuleCollider(Object *parent, double radius, double height);

	double GetRadius() { return _radius; }
	double GetHeight() { return _height; }
	virtual void SetRadius(double radius);
	virtual void SetHeight(double height);
	virtual void SetRadiusAndHeight(double radius, double height);

	virtual ~CapsuleCollider();

protected:
	double _radius, _height;
};