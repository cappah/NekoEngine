/* Neko Engine
 *
 * Skeleton.h
 * Author: Alexandru Naiman
 *
 * Skeleton class definition
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

class Skeleton
{
public:
	ENGINE_API Skeleton(std::vector<Bone> bones) noexcept;
	
	ENGINE_API void Bind(RShader *shader);
	
	
	
	ENGINE_API int Load();
	ENGINE_API void Update(float deltaTime);
	ENGINE_API void Draw(Renderer* r, size_t group);

	ENGINE_API virtual ~Skeleton() noexcept;
	
private:
	Bone *_rootBone;
	Bone _bones[SH_MAX_BONES];
	uint16_t _numBones;
	RBuffer *_buffer;
	glm::mat4 _globalInverseTransform;
	glm::mat4 _transforms[SH_MAX_BONES];
	std::map<std::string, uint16_t> _boneMap;

	void _PrepareTransforms();
	glm::mat4 _BoneTransform(double time, vector<glm::mat4> &transforms);
	
	void _CalculatePosition(glm::vec3 &out, double time, const AnimationNode *node);
	void _CalculateRotation(glm::quat &out, double time, const AnimationNode *node);
	void _CalculateScaling(glm::vec3 &out, double time, const AnimationNode *node);
	void _TransformHierarchy(double time, const Bone *bone, glm::mat4 &parentTransform);
};
