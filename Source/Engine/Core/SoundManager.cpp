/* NekoEngine
 *
 * SoundManager.cpp
 * Author: Alexandru Naiman
 *
 * SoundManager class implementation
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

#include <Engine/SoundManager.h>
#include <Engine/ResourceManager.h>
#include <Engine/Engine.h>
#include <System/Logger.h>
#include <Audio/AudioClip.h>
#include <Audio/AudioSource.h>
#include <Audio/AudioSystem.h>

#define SNDMGR_MODULE	"SoundManager"

using namespace std;
using namespace glm;

AudioSource *SoundManager::_bgMusicSource{ nullptr };
AudioClip *SoundManager::_bgMusicClip{ nullptr };

int SoundManager::Initialize()
{
	Logger::Log(SNDMGR_MODULE, LOG_INFORMATION, "Initializing sound manager...");

	AudioSystem::GetInstance()->SetDistanceModel(AudioDistanceModel::LinearClamped);

	_bgMusicSource = AudioSystem::GetInstance()->CreateSource();
	_bgMusicSource->SetLooping(true);

	Logger::Log(SNDMGR_MODULE, LOG_INFORMATION, "Initialized");

	return ENGINE_OK;
}

int SoundManager::SetBackgroundMusic(int clipId) noexcept
{
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

int SoundManager::SetBackgroundMusic(const char *clipId) noexcept
{
	return SetBackgroundMusic(ResourceManager::GetResourceID(clipId, ResourceType::RES_AUDIOCLIP));
}

bool SoundManager::PlayBackgroundMusic() noexcept
{
	return _bgMusicSource ? _bgMusicSource->Play() : false;
}

void SoundManager::StopBackgroundMusic() noexcept
{
	if(_bgMusicSource) _bgMusicSource->Stop();
}

void SoundManager::SetBackgroundMusicVolume(float vol) noexcept
{
	if (_bgMusicSource) _bgMusicSource->SetGain(vol);
}

void SoundManager::_UnsetBackgroundMusic() noexcept
{
	StopBackgroundMusic();
	
	ResourceManager::UnloadResource(_bgMusicClip->GetResourceInfo()->id, ResourceType::RES_AUDIOCLIP);
	_bgMusicClip = nullptr;
}

void SoundManager::Release() noexcept
{
	delete _bgMusicSource;
	_bgMusicSource = nullptr;

	Logger::Log(SNDMGR_MODULE, LOG_INFORMATION, "Released");
}
