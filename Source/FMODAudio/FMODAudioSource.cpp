/* NekoEngine
 *
 * FMODAudioSource.cpp
 * Author: Alexandru Naiman
 *
 * NekoEngine FMOD Studio Module
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

#include "FMODAudioSource.h"
#include "FMODAudioBuffer.h"

#include <System/Logger.h>

#define FMOD_ASRC	"FMODAudioSource"

FMODAudioSource::FMODAudioSource() noexcept
{
	_channel = nullptr;
	_minDistance = 0.f;
	_maxDistance = 1000.f;
	_coneIn = 500.f;
	_coneOut = 1000.f;
	_coneVol = 1.f;

	memset(&_position, 0x0, sizeof(FMOD_VECTOR));
	memset(&_velocity, 0x0, sizeof(FMOD_VECTOR));
	memset(&_panPos, 0x0, sizeof(FMOD_VECTOR));
	memset(&_coneDirection, 0x0, sizeof(FMOD_VECTOR));
}

void FMODAudioSource::SetPitch(float p) noexcept
{
	if (FMOD_Channel_SetPitch(_channel, p) != FMOD_OK)
		Logger::Log(FMOD_ASRC, LOG_WARNING, "Failed to set pitch");
}

void FMODAudioSource::SetGain(float g) noexcept
{
	if (FMOD_Channel_SetVolume(_channel, g) != FMOD_OK)
		Logger::Log(FMOD_ASRC, LOG_WARNING, "Failed to set volume");
}

void FMODAudioSource::SetConeInnerAngle(float a) noexcept
{
	_coneIn = a;
	if (FMOD_Channel_Set3DConeSettings(_channel, _coneIn, _coneOut, _coneVol) != FMOD_OK)
		Logger::Log(FMOD_ASRC, LOG_WARNING, "Failed to set 3D cone settings");
}

void FMODAudioSource::SetConeOuterAngle(float a) noexcept
{
	_coneOut = a;
	if (FMOD_Channel_Set3DConeSettings(_channel, _coneIn, _coneOut, _coneVol) != FMOD_OK)
		Logger::Log(FMOD_ASRC, LOG_WARNING, "Failed to set 3D cone settings");
}

void FMODAudioSource::SetConeOuterGain(float g) noexcept
{
	_coneVol = g;
	if (FMOD_Channel_Set3DConeSettings(_channel, _coneIn, _coneOut, _coneVol) != FMOD_OK)
		Logger::Log(FMOD_ASRC, LOG_WARNING, "Failed to set 3D cone settings");
}

void FMODAudioSource::SetDirection(glm::vec3 &dir) noexcept
{
	_coneDirection = { dir.x, dir.y, dir.z };
	if (FMOD_Channel_Set3DConeOrientation(_channel, &_coneDirection) != FMOD_OK)
		Logger::Log(FMOD_ASRC, LOG_WARNING, "Failed to set 3D cone orientation");
}

void FMODAudioSource::SetPosition(glm::vec3 &pos) noexcept
{
	_position = { pos.x, pos.y, pos.z };
	if (FMOD_Channel_Set3DAttributes(_channel, &_position, &_velocity, &_panPos) != FMOD_OK)
		Logger::Log(FMOD_ASRC, LOG_WARNING, "Failed to set 3D attributes");
}

void FMODAudioSource::SetVelocity(glm::vec3 &v) noexcept
{
	_velocity = { v.x, v.y, v.z };
	if (FMOD_Channel_Set3DAttributes(_channel, &_position, &_velocity, &_panPos) != FMOD_OK)
		Logger::Log(FMOD_ASRC, LOG_WARNING, "Failed to set 3D attributes");
}

void FMODAudioSource::SetLooping(bool looping) noexcept
{
	//FMOD_Channel_Loop
	//alSourcei(_src, AL_LOOPING, looping);
}

void FMODAudioSource::SetMaxDistance(float maxDistance) noexcept
{
	_maxDistance = maxDistance;
	if (FMOD_Channel_Set3DMinMaxDistance(_channel, _minDistance, _maxDistance) != FMOD_OK)
		Logger::Log(FMOD_ASRC, LOG_WARNING, "Failed to set 3D min/max distance");
}

void FMODAudioSource::SetReferenceDistance(float referenceDistance) noexcept
{
	_minDistance = referenceDistance;
	if (FMOD_Channel_Set3DMinMaxDistance(_channel, _minDistance, _maxDistance) != FMOD_OK)
		Logger::Log(FMOD_ASRC, LOG_WARNING, "Failed to set 3D min/max distance");
}

int FMODAudioSource::SetClip(AudioClip *clip) noexcept
{
	if (!clip)
		return ENGINE_INVALID_ARGS;
	_clip = clip;	
	return ENGINE_OK;
}

bool FMODAudioSource::Play() noexcept
{
	FMOD_RESULT res{ _channel ? FMOD_Channel_SetPaused(_channel, false) : FMOD_System_PlaySound(_system, ((FMODAudioBuffer *)_clip->GetBuffer())->GetSound(), nullptr, true, &_channel) };
	if (res != FMOD_OK)
		Logger::Log(FMOD_ASRC, LOG_WARNING, "Failed to play sound: %d", res);
	return res == FMOD_OK;
}

void FMODAudioSource::Pause() noexcept
{
	if (FMOD_Channel_SetPaused(_channel, true) != FMOD_OK)
		Logger::Log(FMOD_ASRC, LOG_WARNING, "Failed to pause sound");
}

void FMODAudioSource::Stop() noexcept
{
	if (FMOD_Channel_Stop(_channel) != FMOD_OK)
		Logger::Log(FMOD_ASRC, LOG_WARNING, "Failed to stop sound");
	_channel = nullptr;
}

void FMODAudioSource::Rewind() noexcept
{
	if (FMOD_Channel_SetPosition(_channel, 0, FMOD_TIMEUNIT_MS) != FMOD_OK)
		Logger::Log(FMOD_ASRC, LOG_WARNING, "Failed to rewind sound");
}

bool FMODAudioSource::IsPlaying() noexcept
{
	FMOD_BOOL isPlaying{};
	FMOD_Channel_IsPlaying(_channel, &isPlaying);
	return isPlaying;
}

FMODAudioSource::~FMODAudioSource()
{
	FMOD_Channel_Stop(_channel);
	_clip = nullptr;
}
