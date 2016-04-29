/* Neko Engine
 *
 * AnimationClip.h
 * Author: Alexandru Naiman
 *
 * AnimationClip class definition
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

#include <glm/glm.hpp>

#include <Engine/Vertex.h>
#include <Resource/Resource.h>
#include <Resource/AnimationClipResource.h>

struct AnimationNode
{
	std::string name;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> rotations;
	std::vector<glm::vec3> scalings;
};

class AnimationClip : public Resource
{
public:
	ENGINE_API AnimationClip(AnimationClipResource *res) noexcept;

	ENGINE_API AnimationClipResource* GetResourceInfo() noexcept { return (AnimationClipResource*)_resourceInfo; }
	ENGINE_API double GetDuration() noexcept { return _duration; }
	ENGINE_API double GetTicksPerSecond() noexcept { return _ticksPerSecond; }
	ENGINE_API std::vector<AnimationNode> &GetChannels();

	ENGINE_API virtual int Load() override;
	ENGINE_API void Release() noexcept;

	ENGINE_API virtual ~AnimationClip() noexcept;

protected:
	double _duration;
	double _ticksPerSecond;
	std::vector<AnimationNode> _channels;
};
