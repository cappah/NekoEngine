/* NekoEngine
 *
 * AudioSource.cpp
 * Author: Alexandru Naiman
 *
 * AudioSource class implementation
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

#include <Scene/Object.h>
#include <Audio/AudioSource.h>
#include <Engine/ResourceManager.h>

using namespace glm;

AudioSource::AudioSource() noexcept
{
	_clip = nullptr;

	alGenSources(1, &_src);
	alSourcef(_src, AL_PITCH, 1.f);
	alSourcef(_src, AL_GAIN, 1.f);
	alSource3f(_src, AL_POSITION, 0.f, 0.f, 0.f);
	alSource3f(_src, AL_VELOCITY, 0.f, 0.f, 0.f);
}

void AudioSource::SetPitch(float p) noexcept
{
	alSourcef(_src, AL_PITCH, p);
}

void AudioSource::SetGain(float g) noexcept
{
	alSourcef(_src, AL_GAIN, g);
}

void AudioSource::SetConeInnerAngle(float a) noexcept
{
	alSourcef(_src, AL_CONE_INNER_ANGLE, a);
}

void AudioSource::SetConeOuterAngle(float a) noexcept
{
	alSourcef(_src, AL_CONE_OUTER_ANGLE, a);
}

void AudioSource::SetConeOuterGain(float g) noexcept
{
	alSourcef(_src, AL_CONE_OUTER_GAIN, g);
}

void AudioSource::SetDirection(float x, float y, float z) noexcept
{
	alSource3f(_src, AL_DIRECTION, x, y, z);
}

void AudioSource::SetPosition(float x, float y, float z) noexcept
{
	alSource3f(_src, AL_POSITION, x, y, z);
}

void AudioSource::SetVelocity(float x, float y, float z) noexcept
{
	alSource3f(_src, AL_VELOCITY, x, y, z);
}

void AudioSource::SetDirection(glm::vec3 &dir) noexcept
{
	alSource3f(_src, AL_DIRECTION, dir.x, dir.y, dir.z);
}

void AudioSource::SetPosition(glm::vec3 &pos) noexcept
{
	alSource3f(_src, AL_POSITION, pos.x, pos.y, pos.z);
}

void AudioSource::SetVelocity(glm::vec3 &v) noexcept
{
	alSource3f(_src, AL_VELOCITY, v.x, v.y, v.z);
}

void AudioSource::SetLooping(bool looping) noexcept
{
	alSourcei(_src, AL_LOOPING, looping);
}

void AudioSource::SetMaxDistance(float maxDistance) noexcept
{
	alSourcef(_src, AL_MAX_DISTANCE, maxDistance);
}

void AudioSource::SetReferenceDistance(float referenceDistance) noexcept
{
	alSourcef(_src, AL_REFERENCE_DISTANCE, referenceDistance);
}

int AudioSource::SetClip(AudioClip *clip) noexcept
{
	if (!clip)
		return ENGINE_INVALID_ARGS;

	_clip = clip;

	alSourcei(_src, AL_BUFFER, _clip->GetBufferID());

	return ENGINE_OK;
}

bool AudioSource::Play() noexcept
{
	alSourcePlay(_src);
	return IsPlaying();
}

void AudioSource::Pause() noexcept
{
	alSourcePause(_src);
}

void AudioSource::Stop() noexcept
{
	alSourceStop(_src);
}

void AudioSource::Rewind() noexcept
{
	alSourceRewind(_src);
}

bool AudioSource::IsPlaying() noexcept
{
	ALenum state;
	alGetSourcei(_src, AL_SOURCE_STATE, &state);
	return state == AL_PLAYING;
}

AudioSource::~AudioSource()
{
	alSourceStop(_src);
	alSourcei(_src, AL_BUFFER, AL_NONE);
	alDeleteSources(1, &_src);

	/*if (_clip)
		ResourceManager::UnloadResource(_clip->GetResourceInfo()->id, ResourceType::RES_AUDIOCLIP);*/

	_clip = nullptr;
}
