/* Neko Engine
 *
 * SoundManager.cpp
 * Author: Alexandru Naiman
 *
 * SoundManager class implementation
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

#include <Engine/SoundManager.h>
#include <Engine/ResourceManager.h>
#include <Engine/Engine.h>
#include <System/Logger.h>
#include <Audio/AudioClip.h>
#include <Audio/AudioSource.h>

#define SNDMGR_MODULE	"SoundManager"

using namespace std;

ALCdevice* SoundManager::_device;
ALCcontext* SoundManager::_context;
AudioSource* SoundManager::_bgMusicSource;
ALfloat listenerOrientation[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };

int SoundManager::Initialize()
{
	string configFile = Engine::GetConfiguration().Engine.DataDirectory;
	configFile.append("\\sounds.cfg");

	//return ReadConfigFile(configFile);

	Logger::Log(SNDMGR_MODULE, LOG_INFORMATION, "Initializing OpenAL...");

	if ((_device = alcOpenDevice(NULL)) == nullptr)
		return ENGINE_FAIL;

	if ((_context = alcCreateContext(_device, NULL)) == nullptr)
		return ENGINE_FAIL;

	if (!alcMakeContextCurrent(_context))
		return ENGINE_FAIL;

	AL_CHECK_RET(alListener3f(AL_POSITION, 0.f, 0.f, 0.f), ENGINE_FAIL);
	AL_CHECK_RET(alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f), ENGINE_FAIL);
	AL_CHECK_RET(alListenerfv(AL_ORIENTATION, listenerOrientation), ENGINE_FAIL);

	/*_bgMusicSource = new AudioSource();
	_bgMusicSource->SetLooping(true);*/

	Logger::Log(SNDMGR_MODULE, LOG_INFORMATION, "Initialized");

	return ENGINE_OK;
}

int SoundManager::SetBackgroundMusic(int resId) noexcept
{
	return _bgMusicSource->SetClipId(resId);
}

int SoundManager::PlayBackgroundMusic() noexcept
{
	return _bgMusicSource->Play();
}

int SoundManager::StopBackgroundMusic() noexcept
{
	return _bgMusicSource->Stop();
}

int SoundManager::SetBackgroundMusicVolume(float vol) noexcept
{
	return _bgMusicSource->SetGain(vol);
}

void SoundManager::SetListenerPosition(float x, float y, float z) noexcept
{
	AL_CHECK(alListener3f(AL_POSITION, x, y, z));
}

void SoundManager::_UnsetBackgroundMusic() noexcept
{
	_bgMusicSource->SetClipId(ASRC_NO_CLIP);
}

void SoundManager::Release() noexcept
{
	if (_bgMusicSource)
		delete _bgMusicSource;

	AL_CHECK(alcMakeContextCurrent(NULL));
	AL_CHECK(alcDestroyContext(_context));
	_context = nullptr;

	AL_CHECK(alcCloseDevice(_device));
	_device = nullptr;
	
	Logger::Log(SNDMGR_MODULE, LOG_INFORMATION, "Released");
}
