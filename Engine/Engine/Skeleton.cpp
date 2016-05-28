/* Neko Engine
 *
 * Skeleton.cpp
 * Author: Alexandru Naiman
 *
 * Skeleton class implementation 
 *
 * ----------------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman <alexandru dot naiman at icloud dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ALEXANDRU NAIMAN
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define ENGINE_INTERNAL

#include <Engine/Skeleton.h>
#include <Engine/Vertex.h>
#include <Engine/Engine.h>
#include <Engine/EngineUtils.h>
#include <System/Logger.h>
#include <System/AssetLoader/AssetLoader.h>
#include <Engine/AnimationClip.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define SK_MESH_MODULE	"SkeletalMesh"

using namespace std;
using namespace glm;

Skeleton::Skeleton(vector<Bone> &bones, vector<TransformNode> &nodes, dmat4 &globalInverseTransform) noexcept
{
	_numBones = (unsigned int)bones.size();
	_numNodes = (unsigned int)nodes.size();
	
	if(_numBones > SH_MAX_BONES)
	{
		Logger::Log(SK_MESH_MODULE, LOG_WARNING, "Truncating skeleton");
		_numBones = SH_MAX_BONES;
	}
	
	for (unsigned int i = 0; i < _numBones; i++)
	{
		_bones[i] = bones[i];
		_boneMap.insert(make_pair(_bones[i].name, i));
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

void Skeleton::Bind(RShader *shader)
{
	shader->VSSetUniformBuffer(2, 0, sizeof(_transforms), _buffer);
}

int Skeleton::Load()
{
	if((_buffer = Engine::GetRenderer()->CreateBuffer(BufferType::Uniform, true, true)) == nullptr)
	{ DIE("Out of resources"); }
	
	_buffer->SetNumBuffers(3);
	
	TrMat t =
	{
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};
	
	for(int i = 0; i < SH_MAX_BONES; ++i)
		_transforms[i] = t;
	
	_buffer->SetStorage(sizeof(_transforms) * 3, nullptr);

	return ENGINE_OK;
}

void Skeleton::Draw(Renderer* r, size_t group)
{
	//
}

void Skeleton::TransformBones(double time)
{
	if(!_animationClip)
		return;
	
	dmat4 ident = dmat4();
	
	double ticks = _animationClip->GetTicksPerSecond() != 0 ? _animationClip->GetTicksPerSecond() : 25.f;
	double timeInTicks = time * ticks;
	double animTime = mod(timeInTicks, _animationClip->GetDuration());
	
	_TransformHierarchy(animTime, _rootNode, ident);

	_buffer->BeginUpdate();
	_buffer->UpdateData(0, sizeof(_transforms), _transforms);
	_buffer->EndUpdate();

	_buffer->NextBuffer();
}

void Skeleton::_CalculatePosition(dvec3 &out, double time, const AnimationNode *node)
{
	size_t numKeys = node->positionKeys.size();
	
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
	
	uint16_t nextPosIndex = posIndex + 1;
	
	double dt = node->positionKeys[nextPosIndex].time - node->positionKeys[posIndex].time;
	double factor = (time - node->positionKeys[posIndex].time) / dt;
	
	const dvec3 &start = node->positionKeys[posIndex].value;
	const dvec3 &end = node->positionKeys[nextPosIndex].value;
	
	dvec3 delta = end - start;
	out = start + factor * delta;
}

void Skeleton::_CalculateRotation(dquat &out, double time, const AnimationNode *node)
{
	size_t numKeys = node->rotationKeys.size();
	
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
	
	uint16_t nextRotIndex = rotIndex + 1;
	
	double dt = node->rotationKeys[nextRotIndex].time - node->rotationKeys[rotIndex].time;
	double factor = (time - node->rotationKeys[rotIndex].time) / dt;
	
	const dquat &start = node->rotationKeys[rotIndex].value;
	const dquat &end = node->rotationKeys[nextRotIndex].value;
	
	out = slerp(start, end, factor);
	out = normalize(out);
}

void Skeleton::_CalculateScaling(dvec3 &out, double time, const AnimationNode *node)
{
	size_t numKeys = node->scalingKeys.size();
	
	if(numKeys == 1)
	{
		out = node->scalingKeys[0].value;
		return;
	}
	
	uint16_t scaleIndex;
	
	for(uint16_t i = 0; i < numKeys - 1; ++i)
	{
		if(time < node->scalingKeys[i + 1].time)
		{
			scaleIndex = i;
			break;
		}
	}
	
	uint16_t nextScaleIndex = scaleIndex + 1;
	
	double dt = node->scalingKeys[nextScaleIndex].time - node->scalingKeys[scaleIndex].time;
	double factor = (time - node->scalingKeys[scaleIndex].time) / dt;
	
	const dvec3 &start = node->scalingKeys[scaleIndex].value;
	const dvec3 &end = node->scalingKeys[nextScaleIndex].value;
	
	dvec3 delta = end - start;
	out = start + factor * delta;
}

void Skeleton::_TransformHierarchy(double time, const TransformNode *node, dmat4 &parentTransform)
{
	const AnimationNode *animNode = nullptr;
	
	dmat4 nodeTransform = node->transform;
	
	for(uint i = 0; i < _animationClip->GetChannels().size(); ++i)
	{
		if(!_animationClip->GetChannels()[i].name.compare(node->name))
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
		
		dquat rotation = quat();
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
		uint16_t index = _boneMap[node->name];
		mat4 m = _globalInverseTransform * globalTransform * _bones[index].offset;
		memcpy(&_transforms[index], &m[0][0], sizeof(TrMat));
	}
	
	for(uint16_t i = 0; i < node->numChildren; ++i)
		_TransformHierarchy(time, node->children[i], globalTransform);
}

Skeleton::~Skeleton() noexcept
{
	//
}
