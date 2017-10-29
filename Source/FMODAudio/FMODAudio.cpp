/* NekoEngine
 *
 * FMODAudio.cpp
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

#include "version.h"
#include "FMODAudio.h"
#include "FMODAudioBuffer.h"
#include "FMODAudioSource.h"
#include <System/Logger.h>

#define FMOD_MODULE	"FMODAudio"

FMOD_SYSTEM *_system{ nullptr };

int FMODAudio::Initialize()
{
	uint32_t version{ 0 };

	if (FMOD_System_Create(&_system) != FMOD_OK)
	{
		Logger::Log(FMOD_MODULE, LOG_CRITICAL, "Failed to create FMOD system");
		return ENGINE_FAIL;
	}

	if (FMOD_System_GetVersion(_system, &version) != FMOD_OK)
	{
		Logger::Log(FMOD_MODULE, LOG_CRITICAL, "Failed to retrieve FMOD system version");
		return ENGINE_FAIL;
	}

	if (version < FMOD_VERSION)
	{
		Logger::Log(FMOD_MODULE, LOG_CRITICAL, "FMOD version mismatch");
		return ENGINE_FAIL;
	}
	snprintf(_versionString, 100, "%d", version);

	if (FMOD_System_Init(_system, 100, FMOD_INIT_3D_RIGHTHANDED, nullptr) != FMOD_OK)
	{
		Logger::Log(FMOD_MODULE, LOG_CRITICAL, "FMOD init failed");
		return ENGINE_FAIL;
	}

	if (FMOD_System_Set3DSettings(_system, 1.f, 1.f, 1.f) != FMOD_OK)
	{
		Logger::Log(FMOD_MODULE, LOG_CRITICAL, "Failed to set FMOD 3D settings");
		return ENGINE_FAIL;
	}

	_position = { 0.f, 0.f, 0.f };
	_velocity = { 0.f, 0.f, 0.f };
	_forward = { 0.f, 0.f, -1.f };
	_up = { 0.f, 1.f, 0.f };

	if (FMOD_System_Set3DListenerAttributes(_system, 0, &_position, &_velocity, &_forward, &_up) != FMOD_OK)
	{
		Logger::Log(FMOD_MODULE, LOG_CRITICAL, "Failed to set FMOD listener attributes");
		return ENGINE_FAIL;
	}

	Logger::Log(FMOD_MODULE, LOG_INFORMATION, "Initialized");
	Logger::Log(FMOD_MODULE, LOG_INFORMATION, "Module version: %s, using FMOD %d", FMOD_MODULE_VERSION_STRING, version);

	return ENGINE_OK;
}

const char *FMODAudio::GetVersion()
{
	return _versionString;
}

AudioBuffer *FMODAudio::CreateBuffer(size_t size)
{
	return (AudioBuffer *)new FMODAudioBuffer(size);
}

AudioSource *FMODAudio::CreateSource()
{
	return (AudioSource *)new FMODAudioSource();
}

void FMODAudio::SetDistanceModel(AudioDistanceModel model)
{
	//alDistanceModel(_oal_distanceModel[(int)model]);
}

void FMODAudio::SetListenerPosition(glm::vec3 &position)
{
	_position = { position.x, position.y, position.z };
	if (FMOD_System_Set3DListenerAttributes(_system, 0, &_position, &_velocity, &_forward, &_up) != FMOD_OK)
		Logger::Log(FMOD_MODULE, LOG_WARNING, "Failed to set FMOD listener attributes");
}

void FMODAudio::SetListenerVelocity(glm::vec3 &velocity)
{
	_velocity = { velocity.x, velocity.y, velocity.z };
	if (FMOD_System_Set3DListenerAttributes(_system, 0, &_position, &_velocity, &_forward, &_up) != FMOD_OK)
		Logger::Log(FMOD_MODULE, LOG_WARNING, "Failed to set FMOD listener attributes");
}

void FMODAudio::SetListenerOrientation(glm::vec3 &front, glm::vec3 &up)
{
	_forward = { front.x, front.y, front.z };
	_up = { up.x, up.y, up.z };
	if (FMOD_System_Set3DListenerAttributes(_system, 0, &_position, &_velocity, &_forward, &_up) != FMOD_OK)
		Logger::Log(FMOD_MODULE, LOG_WARNING, "Failed to set FMOD listener attributes");
}

void FMODAudio::Update(double deltaTime)
{
	(void)deltaTime;
	FMOD_System_Update(_system);
}

void FMODAudio::Release()
{
	if (FMOD_System_Close(_system) != FMOD_OK)
		Logger::Log(FMOD_MODULE, LOG_WARNING, "Failed close FMOD system");

	if (FMOD_System_Release(_system) != FMOD_OK)
		Logger::Log(FMOD_MODULE, LOG_WARNING, "Failed release FMOD system");
}

FMODAudio::~FMODAudio() { }