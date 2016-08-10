/* NekoEngine
 *
 * Skeleton.h
 * Author: Alexandru Naiman
 *
 * Skeleton class definition
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman
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

#include <string>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Engine/Bone.h>
#include <Engine/Engine.h>
#include <Engine/Shader.h>
#include <Renderer/Renderer.h>
#include <Engine/AnimationNode.h>
#include <Engine/TransformNode.h>

class AnimationClip;

typedef struct TR_MAT
{
	float m[16];
} TrMat;

class Skeleton
{
public:
	ENGINE_API Skeleton(std::vector<Bone> &bones, std::vector<TransformNode> &nodes, glm::dmat4 &globalInverseTransform) noexcept;
	
	ENGINE_API void Bind(RShader *shader);
	
	ENGINE_API void SetAnimationClip(AnimationClip *clip) noexcept { _animationClip = clip; };
	
	ENGINE_API int Load();
	ENGINE_API void Update(float deltaTime);

	ENGINE_API void TransformBones(double time);

	ENGINE_API virtual ~Skeleton() noexcept;
	
private:
	Bone _bones[SH_MAX_BONES];
	TransformNode *_rootNode;
	std::vector<TransformNode> _nodes;
	uint16_t _numBones;
	uint16_t _numNodes;
	RBuffer *_buffer;
	glm::dmat4 _globalInverseTransform;
	TrMat _transforms[SH_MAX_BONES], _prevTransforms[SH_MAX_BONES];
	std::map<std::string, uint16_t> _boneMap;
	AnimationClip *_animationClip;
	
	void _CalculatePosition(glm::dvec3 &out, double time, const AnimationNode *node);
	void _CalculateRotation(glm::dquat &out, double time, const AnimationNode *node);
	void _CalculateScaling(glm::dvec3 &out, double time, const AnimationNode *node);
	void _TransformHierarchy(double time, const TransformNode *node, glm::dmat4 &parentTransform);
};