/* NekoEngine
 *
 * OcTree.cpp
 * Author: Alexandru Naiman
 *
 * NekoEngine
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

#include <Scene/OcTree.h>
#include <Scene/Object.h>
#include <Engine/Engine.h>
#include <Engine/EventManager.h>
#include <System/Logger.h>

#define NOC_MODULE			"NOcTree"
#define NOC_GROW_MAX_ITER	40

using namespace glm;

bool OcTreeNode::Add(const Object *obj)
{
	if (!obj->GetTransformedBounds().IsValid())
		return true;

	if (!_bounds.Contains(obj->GetTransformedBounds()))
		return false;

	if (_count < _maxObjects || (_length / 2) < _minSize)
	{
		_objects.Add(obj);
		++_count;
		return true;
	}

	if (!_children)
	{
		Split();

		NArray<const Object *> removedObjects;

		for (const Object *obj : _objects)
			if (_children[_GetBestFit(obj->GetTransformedBounds())].Add(obj))
				removedObjects.Add(obj);

		for (const Object *obj : removedObjects)
			Remove(obj);
	}

	if (!_children[_GetBestFit(obj->GetTransformedBounds())].Add(obj))
	{
		_objects.Add(obj);
		++_count;
	}

	return true;
}

bool OcTreeNode::Remove(const Object *obj)
{
	size_t id{ _objects.Find(obj) };

	if (id != NArray<const Object *>::NotFound)
	{
		_objects.Remove((uint32_t)id);

		if (_children)
			Merge();

		return true;
	}

	if (!_children)
		return false;

	for (uint8_t i = 0; i < 8; ++i)
	{
		if (_children[i].Remove(obj))
		{
			Merge();
			return true;
		}
	}

	return false;
}

bool OcTreeNode::NeedsReposition(const Object *obj)
{
	size_t id{ _objects.Find(obj) };

	if (id != NArray<const Object *>::NotFound)
	{
		if (!_bounds.Contains(obj->GetTransformedBounds()))
			return true;
		return false;
	}

	if (!_children)
		return false;

	for (uint8_t i = 0; i < 8; ++i)
		if (_children[i].NeedsReposition(obj))
			return true;

	return false;
}

void OcTreeNode::Split()
{
	if (_children)
		return;

	float quat{ _length / 4.f };
	float half{ _length / 2.f };

	if ((_children = new OcTreeNode[8]) == nullptr)
	{ DIE("Out of resources"); }

	_children[0] = OcTreeNode(glm::vec3(-quat,  quat, -quat), half, _minSize, _looseness);
	_children[1] = OcTreeNode(glm::vec3( quat,  quat, -quat), half, _minSize, _looseness);
	_children[2] = OcTreeNode(glm::vec3(-quat,  quat,  quat), half, _minSize, _looseness);
	_children[3] = OcTreeNode(glm::vec3( quat,  quat,  quat), half, _minSize, _looseness);
	_children[4] = OcTreeNode(glm::vec3(-quat, -quat, -quat), half, _minSize, _looseness);
	_children[5] = OcTreeNode(glm::vec3( quat, -quat, -quat), half, _minSize, _looseness);
	_children[6] = OcTreeNode(glm::vec3(-quat, -quat,  quat), half, _minSize, _looseness);
	_children[7] = OcTreeNode(glm::vec3( quat, -quat,  quat), half, _minSize, _looseness);
}

void OcTreeNode::Merge()
{
	if (!_children)
		return;

	size_t count{ _objects.Count() };

	for (uint8_t i = 0; i < 8; ++i)
	{
		if (_children[i]._children)
			return;
		count += _children[i]._objects.Count();
	}

	if (count >= _maxObjects)
		return;

	for (uint8_t i = 0; i < 8; ++i)
		_objects.Add(_children[i]._objects);

	delete[] _children;
	_children = nullptr;
}

OcTreeNode *OcTreeNode::Shrink(float minLength)
{
	// TODO

	return this;
}

void OcTreeNode::GetVisible(const NFrustum &frustum, NArray<const Object *> &visibleObjects)
{
	if (!frustum.ContainsBounds(_bounds))
		return;

	for (const Object *obj : _objects)
	{
		if (obj->GetNoCull() || !obj->IsVisible() || !frustum.ContainsBounds(obj->GetTransformedBounds()))
			continue;

		visibleObjects.Add(obj);
	}

	if (!_children)
		return;

	for (uint8_t i = 0; i < 8; ++i)
		_children[i].GetVisible(frustum, visibleObjects);
}

void OcTreeNode::GetColliding(const NBounds &bounds, NArray<const Object *> &collidingObjects)
{
	if (!_bounds.Intersects(bounds))
		return;

	for (const Object *obj : _objects)
		if (obj->GetTransformedBounds().Intersects(bounds))
			collidingObjects.Add(obj);

	if (!_children)
		return;

	for (uint8_t i = 0; i < 8; ++i)
		_children[i].GetColliding(bounds, collidingObjects);
}

OcTree::OcTree(vec3 center, float initialSize, float looseness, float minNodeSize) :
	_rootNode(new OcTreeNode(center, initialSize, minNodeSize, looseness, OCT_MAXOBJECTS)),
	_count(0),
	//_initialSize(initialSize),
	_looseness(looseness),
	_minNodeSize(minNodeSize)
{
}

bool OcTree::Add(Object *obj)
{
	int i{ 0 };

	while (!_rootNode->Add(obj))
	{
		Grow(obj->GetTransformedBounds().GetCenter() - _rootNode->GetCenter());

		if (++i > NOC_GROW_MAX_ITER)
		{
			Logger::Log(NOC_MODULE, LOG_WARNING, "Failed to add object %s. Maximum number of Grow() iterations reached (%d).",
						*obj->GetName(), NOC_GROW_MAX_ITER);
			return false;
		}
	}

	++_count;

	return true;
}

bool OcTree::Remove(Object *obj)
{
	if (_rootNode->Remove(obj))
	{
		--_count;
		Shrink();
		return true;
	}

	return false;
}
	
void OcTree::Grow(vec3 direction)
{
	int xDir{ direction.x >= 0 ? 1 : -1 };
	int yDir{ direction.y >= 0 ? 1 : -1 };
	int zDir{ direction.z >= 0 ? 1 : -1 };
	OcTreeNode *oldRoot{ _rootNode };
	float half{ oldRoot->GetLength() / 2.f };
	float len{ oldRoot->GetLength() * 2.f };
	vec3 center{ oldRoot->GetCenter() + vec3(xDir * half, yDir * half, zDir * half) };

	_rootNode = new OcTreeNode(center, len, _minNodeSize, _looseness);

	int rootPos{ xDir > 0 ? 1 : 0 };
	if (yDir < 0) rootPos += 4;
	if (zDir > 0) rootPos += 2;

	OcTreeNode *children{ new OcTreeNode[8] };
	if (!children) { DIE("Out of resources"); }

	for (uint8_t i = 0; i < 8; ++i)
	{
		if (i == rootPos)
		{
			children[i] = std::move(*oldRoot);
			continue;
		}

		xDir = i % 2 == 0 ? -1 : 1;
		yDir = i > 3 ? -1 : 1;
		zDir = (i < 2 || (i > 3 && i < 6)) ? -1 : 1;
		children[i] = OcTreeNode(center + glm::vec3(xDir * half, yDir * half, zDir * half), len, _minNodeSize, _looseness);
	}

	_rootNode->SetChildren(children);
	delete oldRoot;
}

void OcTree::Shrink()
{
	_rootNode = _rootNode->Shrink(_minNodeSize);
}
