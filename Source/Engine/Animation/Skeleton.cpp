/* NekoEngine
 *
 * Skeleton.cpp
 * Author: Alexandru Naiman
 *
 * Skeleton class implementation
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

#include <Animation/Skeleton.h>
#include <Animation/AnimationClip.h>
#include <Engine/Vertex.h>
#include <Engine/Engine.h>
#include <System/Logger.h>
#include <System/AssetLoader/AssetLoader.h>

#define SKEL_MODULE		"Skeleton"

using namespace std;
using namespace glm;

Skeleton::Skeleton(vector<Bone> &bones, vector<TransformNode> &nodes, dmat4 &globalInverseTransform) noexcept
{
	_buffer = nullptr;
	_animationClip = nullptr;
	_rootNode = nullptr;
	_bufferSize = 0;

	_numBones = (unsigned int)bones.size();
	_numNodes = (unsigned int)nodes.size();

	_bones.resize(_numBones);
	for (unsigned int i = 0; i < _numBones; i++)
	{
		_bones[i] = bones[i];
		_boneMap.insert(make_pair(_bones[i].name.c_str(), i));
	}

	_nodes.reserve(_numNodes);
	for (unsigned int i = 0; i < _numNodes; ++i)
	{
		_nodes.push_back(nodes[i]);
		_nodes[i].parent = _nodes[i].parentId == -1 ? nullptr : &_nodes[_nodes[i].parentId];

		if(!_nodes[i].parent)
			_rootNode = &_nodes[i];
	}

	for (TransformNode &t : _nodes)
	{
		if(!t.numChildren)
			continue;

		for(int i = 0; i < t.numChildren; ++i)
			t.children.push_back(&_nodes[t.childrenIds[i]]);
	}

	_globalInverseTransform = globalInverseTransform;
}

int Skeleton::Load()
{
	_bufferSize = sizeof(mat4) * _numBones;
	if((_buffer = new Buffer(_bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) == nullptr)
	{ DIE("Out of resources"); }

	constexpr TrMat t =
	{ {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	} };

	_transforms.resize(_numBones);
	_prevTransforms.resize(_numBones);

	for(int i = 0; i < _numBones; ++i)
	{
		_transforms[i] = t;
		_prevTransforms[i] = t;
	}
	
	return ENGINE_OK;
}

void Skeleton::TransformBones(double time)
{
	if(!_animationClip)
		return;

	dmat4 ident = dmat4();

	const double ticks = _animationClip->GetTicksPerSecond() != 0 ? _animationClip->GetTicksPerSecond() : 25.f;
	const double timeInTicks = time * ticks;
	const double animTime = mod(timeInTicks, _animationClip->GetDuration());

	_TransformHierarchy(animTime, _rootNode, ident);
}

void Skeleton::UpdateData(VkCommandBuffer commandBuffer) noexcept
{
	_buffer->UpdateData((uint8_t *)_transforms.data(), 0, _bufferSize, commandBuffer);
}

void Skeleton::_CalculatePosition(dvec3 &out, double time, const AnimationNode *node)
{
	const size_t numKeys = node->positionKeys.size();

	if(numKeys == 1)
	{
		out = node->positionKeys[0].value;
		return;
	}

	uint16_t posIndex = 0;

	for(uint16_t i = 0; i < numKeys - 1; ++i)
	{
		if(time < node->positionKeys[i + 1].time)
		{
			posIndex = i;
			break;
		}
	}

	const uint16_t nextPosIndex = posIndex + 1;

	const double dt = node->positionKeys[nextPosIndex].time - node->positionKeys[posIndex].time;
	const double factor = (time - node->positionKeys[posIndex].time) / dt;

	const dvec3 &start = node->positionKeys[posIndex].value;
	const dvec3 &end = node->positionKeys[nextPosIndex].value;

	const dvec3 delta = end - start;
	out = start + factor * delta;
}

void Skeleton::_CalculateRotation(dquat &out, double time, const AnimationNode *node)
{
	const size_t numKeys = node->rotationKeys.size();

	if(numKeys == 1)
	{
		out = node->rotationKeys[0].value;
		return;
	}

	uint16_t rotIndex = 0;

	for(uint16_t i = 0; i < numKeys - 1; ++i)
	{
		if(time < node->rotationKeys[i + 1].time)
		{
			rotIndex = i;
			break;
		}
	}

	const uint16_t nextRotIndex = rotIndex + 1;

	const double dt = node->rotationKeys[nextRotIndex].time - node->rotationKeys[rotIndex].time;
	const double factor = (time - node->rotationKeys[rotIndex].time) / dt;

	const dquat &start = node->rotationKeys[rotIndex].value;
	const dquat &end = node->rotationKeys[nextRotIndex].value;

	out = slerp(start, end, factor);
	out = normalize(out);
}

void Skeleton::_CalculateScaling(dvec3 &out, double time, const AnimationNode *node)
{
	const size_t numKeys = node->scalingKeys.size();

	if(numKeys == 1)
	{
		out = node->scalingKeys[0].value;
		return;
	}

	uint16_t scaleIndex = 0;

	for(uint16_t i = 0; i < numKeys - 1; ++i)
	{
		if(time < node->scalingKeys[i + 1].time)
		{
			scaleIndex = i;
			break;
		}
	}

	const uint16_t nextScaleIndex = scaleIndex + 1;

	const double dt = node->scalingKeys[nextScaleIndex].time - node->scalingKeys[scaleIndex].time;
	const double factor = (time - node->scalingKeys[scaleIndex].time) / dt;

	const dvec3 &start = node->scalingKeys[scaleIndex].value;
	const dvec3 &end = node->scalingKeys[nextScaleIndex].value;

	const dvec3 delta = end - start;
	out = start + factor * delta;
}

void Skeleton::_TransformHierarchy(double time, const TransformNode *node, dmat4 &parentTransform)
{
	const AnimationNode *animNode = nullptr;

	dmat4 nodeTransform = node->transform;

	for(uint i = 0; i < _animationClip->GetChannels().size(); ++i)
	{
		if(*_animationClip->GetChannels()[i].name && !strncmp(*_animationClip->GetChannels()[i].name, node->name.c_str(), node->name.length()))
		{
			animNode = &_animationClip->GetChannels()[i];
			break;
		}
	}

	if(animNode)
	{
		dvec3 scaling;
		_CalculateScaling(scaling, time, animNode);
		dmat4 scaleMatrix = scale(dmat4(1), scaling);

		dquat rotation;
		_CalculateRotation(rotation, time, animNode);
		dmat4 rotationMatrix = mat4_cast(rotation);

		dvec3 position;
		_CalculatePosition(position, time, animNode);
		dmat4 translationMatirx = translate(dmat4(1), position);

		nodeTransform = (translationMatirx * rotationMatrix) * scaleMatrix;
	}

	dmat4 globalTransform = parentTransform * nodeTransform;

	if(_boneMap.find(node->name) != _boneMap.end())
	{
		const uint16_t index = _boneMap[node->name];
		mat4 m = (mat4)(_globalInverseTransform * globalTransform * _bones[index].offset);
		memcpy(&_transforms[index], &m[0][0], sizeof(TrMat));
	}

	for(uint16_t i = 0; i < node->numChildren; ++i)
		_TransformHierarchy(time, node->children[i], globalTransform);
}

Skeleton::~Skeleton() noexcept
{
	delete _buffer;
}
