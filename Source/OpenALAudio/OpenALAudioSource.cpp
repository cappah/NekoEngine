/* NekoEngine
 *
 * OpenALAudioSource.cpp
 * Author: Alexandru Naiman
 *
 * NekoEngine OpenAL Module
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

#include "OpenALAudioSource.h"
#include "OpenALAudioBuffer.h"

OpenALAudioSource::OpenALAudioSource() noexcept
{
	alGenSources(1, &_src);
	alSourcef(_src, AL_PITCH, 1.f);
	alSourcef(_src, AL_GAIN, 1.f);
	alSource3f(_src, AL_POSITION, 0.f, 0.f, 0.f);
	alSource3f(_src, AL_VELOCITY, 0.f, 0.f, 0.f);
}

void OpenALAudioSource::SetPitch(float p) noexcept
{
	alSourcef(_src, AL_PITCH, p);
}

void OpenALAudioSource::SetGain(float g) noexcept
{
	alSourcef(_src, AL_GAIN, g);
}

void OpenALAudioSource::SetConeInnerAngle(float a) noexcept
{
	alSourcef(_src, AL_CONE_INNER_ANGLE, a);
}

void OpenALAudioSource::SetConeOuterAngle(float a) noexcept
{
	alSourcef(_src, AL_CONE_OUTER_ANGLE, a);
}

void OpenALAudioSource::SetConeOuterGain(float g) noexcept
{
	alSourcef(_src, AL_CONE_OUTER_GAIN, g);
}

void OpenALAudioSource::SetDirection(glm::vec3 &dir) noexcept
{
	alSource3f(_src, AL_DIRECTION, dir.x, dir.y, dir.z);
}

void OpenALAudioSource::SetPosition(glm::vec3 &pos) noexcept
{
	alSource3f(_src, AL_POSITION, pos.x, pos.y, pos.z);
}

void OpenALAudioSource::SetVelocity(glm::vec3 &v) noexcept
{
	alSource3f(_src, AL_VELOCITY, v.x, v.y, v.z);
}

void OpenALAudioSource::SetLooping(bool looping) noexcept
{
	alSourcei(_src, AL_LOOPING, looping);
}

void OpenALAudioSource::SetMaxDistance(float maxDistance) noexcept
{
	alSourcef(_src, AL_MAX_DISTANCE, maxDistance);
}

void OpenALAudioSource::SetReferenceDistance(float referenceDistance) noexcept
{
	alSourcef(_src, AL_REFERENCE_DISTANCE, referenceDistance);
}

int OpenALAudioSource::SetClip(AudioClip *clip) noexcept
{
	if (!clip)
		return ENGINE_INVALID_ARGS;

	alSourceStop(_src);

	_clip = clip;

	alSourcei(_src, AL_BUFFER, ((OpenALAudioBuffer *)_clip->GetBuffer())->GetBufferId());

	return ENGINE_OK;
}

bool OpenALAudioSource::Play() noexcept
{
	alSourcePlay(_src);
	return IsPlaying();
}

void OpenALAudioSource::Pause() noexcept
{
	alSourcePause(_src);
}

void OpenALAudioSource::Stop() noexcept
{
	alSourceStop(_src);
}

void OpenALAudioSource::Rewind() noexcept
{
	alSourceRewind(_src);
}

bool OpenALAudioSource::IsPlaying() noexcept
{
	ALenum state;
	alGetSourcei(_src, AL_SOURCE_STATE, &state);
	return state == AL_PLAYING;
}

OpenALAudioSource::~OpenALAudioSource()
{
	alSourceStop(_src);
	alSourcei(_src, AL_BUFFER, AL_NONE);
	alDeleteSources(1, &_src);

	_clip = nullptr;
}
