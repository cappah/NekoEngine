/* NekoEngine
 *
 * OpenALAudio.cpp
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

#include "version.h"
#include "OpenALAudio.h"
#include "OpenALAudioBuffer.h"
#include "OpenALAudioSource.h"
#include <System/Logger.h>

#define OAL_MODULE	"OpenALAudio"

ALenum _oal_distanceModel[]
{
	AL_INVERSE_DISTANCE,
	AL_INVERSE_DISTANCE_CLAMPED,
	AL_LINEAR_DISTANCE,
	AL_LINEAR_DISTANCE_CLAMPED,
	AL_EXPONENT_DISTANCE,
	AL_EXPONENT_DISTANCE_CLAMPED,
};

ALfloat listenerOrientation[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };

int OpenALAudio::Initialize()
{
	if ((_device = alcOpenDevice(NULL)) == nullptr)
	{
		Logger::Log(OAL_MODULE, LOG_CRITICAL, "Failed to open device.");
		return ENGINE_FAIL;
	}

	if ((_context = alcCreateContext(_device, NULL)) == nullptr)
		return ENGINE_FAIL;

	if (!alcMakeContextCurrent(_context))
		return ENGINE_FAIL;

	alListener3f(AL_POSITION, 0.f, 0.f, 0.f);
	alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f);
	alListenerfv(AL_ORIENTATION, listenerOrientation);
	
	alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);

	Logger::Log(OAL_MODULE, LOG_INFORMATION, "Initialized");
	Logger::Log(OAL_MODULE, LOG_INFORMATION, "Module version: %s, using OpenAL %s", OAL_MODULE_VERSION_STRING, GetVersion());

	return ENGINE_OK;
}

const char *OpenALAudio::GetVersion()
{
	/*NString alVersion();
	if (alVersion.Find("OpenAL") != NString::NotFound)
		alVersion = "[";
	else
		alVersion = "[OpenAL ";

	alVersion.Append(alVersionStr);
	alVersion = alVersion.Substring(0, alVersion.FindFirst('.') + 2);*/

	return (const char *)alGetString(AL_VERSION);
}

AudioBuffer *OpenALAudio::CreateBuffer(size_t size)
{
	return (AudioBuffer *)new OpenALAudioBuffer(size);
}

AudioSource *OpenALAudio::CreateSource()
{
	return (AudioSource *)new OpenALAudioSource();
}

void OpenALAudio::SetDistanceModel(AudioDistanceModel model)
{
	alDistanceModel(_oal_distanceModel[(int)model]);
}

void OpenALAudio::SetListenerPosition(glm::vec3 &position)
{
	alListener3f(AL_POSITION, position.x, position.y, position.z);
}

void OpenALAudio::SetListenerVelocity(glm::vec3 &velocity)
{
	alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

void OpenALAudio::SetListenerOrientation(glm::vec3 &front, glm::vec3 &up)
{
	float orientation[6]{ front.x, front.y, front.z, up.x, up.y, up.z };
	alListenerfv(AL_ORIENTATION, orientation);
}

void OpenALAudio::Update(double deltaTime)
{
	//
}

void OpenALAudio::Release()
{
	alcMakeContextCurrent(NULL);
	alcDestroyContext(_context);
	_context = nullptr;

	alcCloseDevice(_device);
	_device = nullptr;
}

OpenALAudio::~OpenALAudio() { }