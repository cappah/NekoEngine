/* NekoEngine
 *
 * SoundManager.cpp
 * Author: Alexandru Naiman
 *
 * SoundManager class implementation
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

#include <Engine/SoundManager.h>
#include <Engine/ResourceManager.h>
#include <Engine/Engine.h>
#include <System/Logger.h>
#include <Audio/AudioClip.h>
#include <Audio/AudioSource.h>

#include <glm/glm.hpp>

#define SNDMGR_MODULE	"SoundManager"

using namespace std;
using namespace glm;

ALCdevice *SoundManager::_device;
ALCcontext *SoundManager::_context;
AudioSource *SoundManager::_bgMusicSource;
AudioClip *SoundManager::_bgMusicClip;
ALfloat listenerOrientation[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
static bool _initialized = false;

int SoundManager::Initialize()
{
	string configFile = Engine::GetConfiguration().Engine.DataDirectory;
	configFile.append("\\sounds.cfg");

	//return ReadConfigFile(configFile);

	Logger::Log(SNDMGR_MODULE, LOG_INFORMATION, "Initializing OpenAL...");

	if ((_device = alcOpenDevice(NULL)) == nullptr)
	{
		Logger::Log(SNDMGR_MODULE, LOG_WARNING, "Failed to initialize OpenAL, sound disabled.");
		return ENGINE_OK;
	}

	if ((_context = alcCreateContext(_device, NULL)) == nullptr)
	{
		alcCloseDevice(_device);
		Logger::Log(SNDMGR_MODULE, LOG_WARNING, "Failed to initialize OpenAL, sound disabled.");
		return ENGINE_OK;
	}

	if (!alcMakeContextCurrent(_context))
		return ENGINE_FAIL;

	AL_CHECK_RET(alListener3f(AL_POSITION, 0.f, 0.f, 0.f), ENGINE_FAIL);
	AL_CHECK_RET(alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f), ENGINE_FAIL);
	AL_CHECK_RET(alListenerfv(AL_ORIENTATION, listenerOrientation), ENGINE_FAIL);

	AL_CHECK_RET(alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED), ENGINE_FAIL);

	_bgMusicSource = new AudioSource();
	_bgMusicSource->SetLooping(true);

	_initialized = true;

	Logger::Log(SNDMGR_MODULE, LOG_INFORMATION, "Initialized");

	return ENGINE_OK;
}

bool SoundManager::Enabled() { return _initialized; }

int SoundManager::SetBackgroundMusic(int clipId) noexcept
{
	if (!_initialized) return ENGINE_OK;

	StopBackgroundMusic();
	
	if (_bgMusicClip)
	{
		ResourceManager::UnloadResource(_bgMusicClip->GetResourceInfo()->id, ResourceType::RES_AUDIOCLIP);
		_bgMusicClip = nullptr;
	}

	if ((_bgMusicClip = (AudioClip*)ResourceManager::GetResource(clipId, ResourceType::RES_AUDIOCLIP)) == nullptr)
		return ENGINE_INVALID_RES;

	return _bgMusicSource->SetClip(_bgMusicClip);
}

int SoundManager::PlayBackgroundMusic() noexcept
{
	if (!_initialized) return ENGINE_OK;
	return _bgMusicSource->Play();
}

int SoundManager::StopBackgroundMusic() noexcept
{
	if (!_initialized) return ENGINE_OK;
	return _bgMusicSource->Stop();
}

int SoundManager::SetBackgroundMusicVolume(float vol) noexcept
{
	if (!_initialized) return ENGINE_OK;
	return _bgMusicSource->SetGain(vol);
}

void SoundManager::SetListenerPosition(float x, float y, float z) noexcept
{
	if (!_initialized) return;
	_bgMusicSource->SetPosition(x, y, z);
	AL_CHECK(alListener3f(AL_POSITION, x, y, z));
}

void SoundManager::SetListenerOrientation(float x, float y, float z) noexcept
{
	if (!_initialized) return;
//	AL_CHECK(alListener3f(AL_ORIENTATION, x, y, z));
}

void SoundManager::_UnsetBackgroundMusic() noexcept
{
	if (!_initialized) return;
	StopBackgroundMusic();
	
	ResourceManager::UnloadResource(_bgMusicClip->GetResourceInfo()->id, ResourceType::RES_AUDIOCLIP);
	_bgMusicClip = nullptr;
}

void SoundManager::Release() noexcept
{
	if (!_initialized) return;
	if (_bgMusicSource)
		delete _bgMusicSource;

	AL_CHECK(alcMakeContextCurrent(NULL));
	AL_CHECK(alcDestroyContext(_context));
	_context = nullptr;

	AL_CHECK(alcCloseDevice(_device));
	_device = nullptr;
	
	Logger::Log(SNDMGR_MODULE, LOG_INFORMATION, "Released");
}
