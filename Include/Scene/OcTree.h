/* NekoEngine
 *
 * OcTree.h
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

#pragma once

#include <stdint.h>
#include <Engine/Defs.h>
#include <Runtime/NArray.h>
#include <Runtime/NBounds.h>
#include <Runtime/NFrustum.h>

#define OCT_CENTER			glm::vec3(0.f)
#define OCT_LOOSENESS		20.f
#define OCT_INITSIZE		200.f
#define OCT_MINSIZE			10.f
#define OCT_MAXOBJECTS		20

class Object;

class OcTreeNode
{
public:
	OcTreeNode(glm::vec3 center = OCT_CENTER, float length = OCT_INITSIZE, float minSize = OCT_MINSIZE, float looseness = OCT_LOOSENESS, uint8_t maxObjects = OCT_MAXOBJECTS) :
		_center(center),
		_children(nullptr),
		_count(0), _maxObjects(maxObjects),
		_length(length), _minSize(minSize), _looseness(looseness)
	{
		_looseLength = _length * _looseness;
		_bounds.InitBox(glm::vec3(-_looseLength), glm::vec3(_looseLength));
		_bounds.SetCenter(center);
	}

	OcTreeNode(const OcTreeNode &other)
	{
		_center = other._center;
		_count = other._count;
		_maxObjects = other._maxObjects;
		_length = other._length;
		_minSize = other._minSize;
		_looseness = other._looseness;
		_bounds = other._bounds;
		_objects = other._objects;

		if (!other._children)
			return;

		_children = new OcTreeNode[8];

		for (uint8_t i = 0; i < 8; ++i)
			_children[i] = other._children[i];
	}

	OcTreeNode(OcTreeNode &&other)
	{
		_center = other._center;
		_count = other._count;
		_maxObjects = other._maxObjects;
		_length = other._length;
		_minSize = other._minSize;
		_looseness = other._looseness;
		_bounds = other._bounds;
		_objects = std::move(other._objects);
		_children = other._children;
		other._children = nullptr;
	}

	float GetLength() { return _length; }
	const glm::vec3 &GetCenter() const { return _center; }
	void SetChildren(OcTreeNode *children)
	{
		if (_children)
			delete[] _children;

		_children = children;
	}

	bool Add(const Object *obj);
	bool Remove(const Object *obj);
	bool NeedsReposition(const Object *obj);

	void Split();
	void Merge();
	OcTreeNode *Shrink(float minLength);

	void GetVisible(const NFrustum &frustum, NArray<const Object *> &visibleObjects);
	void GetColliding(const NBounds &bounds, NArray<const Object *> &colligindObjects);

	bool HasObjects()
	{
		if (_objects.Count()) return true;

		for (uint8_t i = 0; i < 8; ++i)
			if (_children[i].HasObjects())
				return true;
	}

	virtual ~OcTreeNode()
	{
		delete[] _children;
	}

	OcTreeNode &operator =(const OcTreeNode &other)
	{
		_center = other._center;
		_count = other._count;
		_maxObjects = other._maxObjects;
		_length = other._length;
		_minSize = other._minSize;
		_looseness = other._looseness;
		_bounds = other._bounds;
		_objects = other._objects;
		
		_children = new OcTreeNode[8];

		for (uint8_t i = 0; i < 8; ++i)
			_children[i] = other._children[i];		

		return *this;
	}

private:
	glm::vec3 _center;
	NBounds _bounds;
	OcTreeNode *_children;
	size_t _count, _maxObjects;
	NArray<const Object *> _objects;
	float _length, _minSize, _looseness, _looseLength;

	uint32_t _GetBestFit(const NBounds &bounds) const { return (bounds.GetCenter().x <= _center.x ? 0 : 1) + (bounds.GetCenter().y >= _center.y ? 0 : 4) + (bounds.GetCenter().z <= _center.z ? 0 : 2); }
};

class OcTree
{
public:
	OcTree(glm::vec3 center = OCT_CENTER, float initialSize = OCT_INITSIZE, float looseness = OCT_LOOSENESS, float minNodeSize = OCT_MINSIZE);
	
	OcTreeNode *GetRootNode() noexcept { return _rootNode; }
	size_t Count() { return _count; }
	
	bool Add(Object *obj);
	bool Remove(Object *obj);
	void Grow(glm::vec3 direction);
	void Shrink();

	void GetVisible(const NFrustum &frustum, NArray<const Object *> &visibleObjects) { _rootNode->GetVisible(frustum, visibleObjects); }
	void GetColliding(const NBounds &bounds, NArray<const Object *> &collidingObjects) { _rootNode->GetColliding(bounds, collidingObjects); }
	
	virtual ~OcTree()
	{
		delete _rootNode;
		_rootNode = nullptr;
		_count = 0;
	}
	
private:
	OcTreeNode *_rootNode;
	size_t _count;
	float _initialSize, _looseness, _minNodeSize;
};
