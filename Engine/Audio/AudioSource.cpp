/* Neko Engine
 *
 * AudioSource.cpp
 * Author: Alexandru Naiman
 *
 * AudioSource class implementation
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

#include <glm/glm.hpp>

#include <Scene/Object.h>
#include <Audio/AudioSource.h>
#include <Engine/ResourceManager.h>

using namespace glm;

AudioSource::AudioSource() noexcept
{
	_clip = nullptr;

	AL_CHECK_FATAL(alGenSources(1, &_src));
	AL_CHECK_FATAL(alSourcef(_src, AL_PITCH, 1.f));
	AL_CHECK_FATAL(alSourcef(_src, AL_GAIN, 1.f));
	AL_CHECK_FATAL(alSource3f(_src, AL_POSITION, 0.f, 0.f, 0.f));
	AL_CHECK_FATAL(alSource3f(_src, AL_VELOCITY, 0.f, 0.f, 0.f));
}

int AudioSource::SetPitch(float p) noexcept
{
	AL_CHECK_RET(alSourcef(_src, AL_PITCH, p), ENGINE_FAIL);
	return ENGINE_OK;
}

int AudioSource::SetGain(float g) noexcept
{
	AL_CHECK_RET(alSourcef(_src, AL_GAIN, g), ENGINE_FAIL);
	return ENGINE_OK;
}

int AudioSource::SetConeInnerAngle(float a) noexcept
{
	AL_CHECK_RET(alSourcef(_src, AL_CONE_INNER_ANGLE, a), ENGINE_FAIL);
	return ENGINE_OK;
}

int AudioSource::SetConeOuterAngle(float a) noexcept
{
	AL_CHECK_RET(alSourcef(_src, AL_CONE_OUTER_ANGLE, a), ENGINE_FAIL);
	return ENGINE_OK;
}

int AudioSource::SetConeOuterGain(float g) noexcept
{
	AL_CHECK_RET(alSourcef(_src, AL_CONE_OUTER_GAIN, g), ENGINE_FAIL);
	return ENGINE_OK;
}

int AudioSource::SetDirection(float x, float y, float z) noexcept
{
	AL_CHECK_RET(alSource3f(_src, AL_DIRECTION, x, y, z), ENGINE_FAIL);
	return ENGINE_OK;
}

int AudioSource::SetPosition(float x, float y, float z) noexcept
{
	AL_CHECK_RET(alSource3f(_src, AL_POSITION, x, y, z), ENGINE_FAIL);
	return ENGINE_OK;
}

int AudioSource::SetVelocity(float x, float y, float z) noexcept
{
	AL_CHECK_RET(alSource3f(_src, AL_VELOCITY, x, y, z), ENGINE_FAIL);
	return ENGINE_OK;
}

int AudioSource::SetDirection(glm::vec3 &dir) noexcept
{
	AL_CHECK_RET(alSource3f(_src, AL_DIRECTION, dir.x, dir.y, dir.z), ENGINE_FAIL);
	return ENGINE_OK;
}

int AudioSource::SetPosition(glm::vec3 &pos) noexcept
{
	AL_CHECK_RET(alSource3f(_src, AL_POSITION, pos.x, pos.y, pos.z), ENGINE_FAIL);
	return ENGINE_OK;
}

int AudioSource::SetVelocity(glm::vec3 &v) noexcept
{
	AL_CHECK_RET(alSource3f(_src, AL_VELOCITY, v.x, v.y, v.z), ENGINE_FAIL);
	return ENGINE_OK;
}

int AudioSource::SetLooping(bool looping) noexcept
{
	AL_CHECK_RET(alSourcei(_src, AL_LOOPING, looping), ENGINE_FAIL);
	return ENGINE_OK;
}

int AudioSource::SetMaxDistance(float maxDistance) noexcept
{
	AL_CHECK_RET(alSourcef(_src, AL_MAX_DISTANCE, maxDistance), ENGINE_FAIL);
	return ENGINE_OK;
}

int AudioSource::SetReferenceDistance(float referenceDistance) noexcept
{
	AL_CHECK_RET(alSourcef(_src, AL_REFERENCE_DISTANCE, referenceDistance), ENGINE_FAIL);
	return ENGINE_OK;
}

int AudioSource::SetClip(AudioClip *clip) noexcept
{
	if (!clip)
		return ENGINE_INVALID_ARGS;

	_clip = clip;

	AL_CHECK_RET(alSourcei(_src, AL_BUFFER, _clip->GetBufferID()), ENGINE_FAIL);

	return ENGINE_OK;
}

int AudioSource::Play() noexcept
{
	AL_CHECK_RET(alSourcePlay(_src), ENGINE_FAIL);
	return ENGINE_OK;
}

int AudioSource::Pause() noexcept
{
	AL_CHECK_RET(alSourcePause(_src), ENGINE_FAIL);
	return ENGINE_OK;
}

int AudioSource::Stop() noexcept
{
	AL_CHECK_RET(alSourceStop(_src), ENGINE_FAIL);
	return ENGINE_OK;
}

int AudioSource::Rewind() noexcept
{
	AL_CHECK_RET(alSourceRewind(_src), ENGINE_FAIL);
	return ENGINE_OK;
}

AudioSource::~AudioSource()
{
	AL_CHECK(alSourceStop(_src));
	AL_CHECK(alSourcei(_src, AL_BUFFER, AL_NONE));
	AL_CHECK(alDeleteSources(1, &_src));

	/*if (_clip)
		ResourceManager::UnloadResource(_clip->GetResourceInfo()->id, ResourceType::RES_AUDIOCLIP);*/

	_clip = nullptr;
}
