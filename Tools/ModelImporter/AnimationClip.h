/* NekoEngine - ModelImporter
 *
 * AnimationClip.h
 * Author: Alexandru Naiman
 *
 * AnimationClip class definition
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

#ifndef ANIMATIONCLIP_H
#define ANIMATIONCLIP_H

#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define ANIM_HEADER	"NANIM2 "
#define ANIM_FOOTER	"ENDANIM"

struct VectorKey
{
	glm::dvec3 value;
	double time;
};

struct QuatKey
{
	glm::dquat value;
	double time;
};

typedef struct ANIMATION_NODE
{
	std::string name;
	std::vector<VectorKey> positionKeys;
	std::vector<QuatKey> rotationKeys;
	std::vector<VectorKey> scalingKeys;
} AnimationNode;

class AnimationClip
{
public:
	AnimationClip(std::string name, double duration, double ticksPerSecond) :
		_name(name), _duration(duration), _ticksPerSecond(ticksPerSecond)
	{ }

	void AddChannel(AnimationNode &channel) { _channels.push_back(channel); }

	void Export(const char *file);

	virtual ~AnimationClip() { }

private:
	std::string _name;
	double _duration, _ticksPerSecond;
	std::vector<AnimationNode> _channels;
};

#endif // ANIMATIONCLIP_H
