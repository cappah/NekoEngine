/* NekoEngine
 *
 * AudioSource.h
 * Author: Alexandru Naiman
 *
 * Wrapper for OpenAL audio source
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
#include <Audio/AudioClip.h>

#define ASRC_NO_CLIP	-1

class AudioSource
{
public:
	AudioSource() noexcept { }

	ENGINE_API bool HasClip() noexcept { return _clip != nullptr; }

	virtual void SetPitch(float p) noexcept = 0;
	virtual void SetGain(float g) noexcept = 0;

	virtual void SetConeInnerAngle(float a) noexcept = 0;
	virtual void SetConeOuterAngle(float a) noexcept = 0;
	virtual void SetConeOuterGain(float g) noexcept = 0;
	
	virtual void SetDirection(glm::vec3 &dir) noexcept = 0;
	virtual void SetPosition(glm::vec3 &pos) noexcept = 0;
	virtual void SetVelocity(glm::vec3 &v) noexcept = 0;

	virtual void SetLooping(bool looping) noexcept = 0;

	virtual void SetMaxDistance(float maxDistance) noexcept = 0;
	virtual void SetReferenceDistance(float referenceDistance) noexcept = 0;

	virtual int SetClip(AudioClip *clip) noexcept = 0;

	virtual bool Play() noexcept = 0;
	virtual void Pause() noexcept = 0;
	virtual void Stop() noexcept = 0;
	virtual void Rewind() noexcept = 0;
	virtual bool IsPlaying() noexcept = 0;

	virtual ~AudioSource() {}

protected:
	AudioClip *_clip;
};

