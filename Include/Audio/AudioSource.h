/* Neko Engine
 *
 * AudioSource.h
 * Author: Alexandru Naiman
 *
 * Wrapper for OpenAL audio source
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

#pragma once

#include <Platform/Platform.h>

#ifdef NE_PLATFORM_MAC
	#include <OpenAL/al.h>
#else
	#include <AL/al.h>
#endif

#include <glm/glm.hpp>

#include <Engine/Engine.h>
#include <Audio/AudioClip.h>
#include <Scene/ObjectComponent.h>

#define ASRC_NO_CLIP	-1

class AudioSource :
	public ObjectComponent
{
public:
	AudioSource() noexcept;

	ENGINE_API bool HasClip() noexcept { return _clip != nullptr; }

	ENGINE_API int SetPitch(float p) noexcept;
	ENGINE_API int SetGain(float g) noexcept;

	ENGINE_API int SetConeInnerAngle(float a) noexcept;
	ENGINE_API int SetConeOuterAngle(float a) noexcept;
	ENGINE_API int SetConeOuterGain(float g) noexcept;

	ENGINE_API int SetDirection(glm::vec3 &dir) noexcept;
	ENGINE_API int SetPosition(glm::vec3 &pos) noexcept;
	ENGINE_API int SetVelocity(glm::vec3 &v) noexcept;

	ENGINE_API int SetLooping(bool looping) noexcept;

	ENGINE_API int SetClipId(int id) noexcept;

	ENGINE_API int Play() noexcept;
	ENGINE_API int Pause() noexcept;
	ENGINE_API int Stop() noexcept;
	ENGINE_API int Rewind() noexcept;

	ENGINE_API virtual void Update(float deltaTime) noexcept override;

	virtual ~AudioSource();

private:
	ALuint _src;
	AudioClip *_clip;
};

