/* NekoEngine
 *
 * FMODAudio.h
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

#pragma once

#include <fmod.h>
#include <fmod_common.h>

#include <Audio/AudioSystem.h>
#include <Audio/AudioBuffer.h>
#include <Audio/AudioSource.h>
#include <Platform/PlatformDetect.h>

extern FMOD_SYSTEM *_system;

class FMODAudio : public AudioSystem
{
public:
	virtual int Initialize() override;

	virtual const char *GetName() { return "FMOD"; }
	virtual const char *GetVersion();

	virtual AudioBuffer *CreateBuffer(size_t size) override;
	virtual AudioSource *CreateSource() override;

	virtual void SetDistanceModel(AudioDistanceModel model) override;

	virtual void SetListenerPosition(glm::vec3 &position) override;
	virtual void SetListenerVelocity(glm::vec3 &velocity) override;
	virtual void SetListenerOrientation(glm::vec3 &front, glm::vec3 &up) override;

	virtual void Update(double deltaTime) override;

	virtual void Release() override;

	virtual ~FMODAudio();

private:
	FMOD_VECTOR _position, _velocity, _forward, _up;
	char _versionString[100];
};