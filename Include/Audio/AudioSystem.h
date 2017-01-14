/* NekoEngine
 *
 * AudioSystem.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Audio System Module Interface
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

#include <Engine/Defs.h>
#include <Audio/AudioSource.h>
#include <Audio/AudioBuffer.h>

enum class AudioDistanceModel : uint8_t
{
	Inverse,
	InverseClamped,
	Linear,
	LinearClamped,
	Exponent,
	ExponentClamped
};

class ENGINE_API AudioSystem
{
public:
	virtual int Initialize() = 0;

	virtual const char *GetName() = 0;
	virtual const char *GetVersion() = 0;

	virtual AudioBuffer *CreateBuffer(size_t size) = 0;
	virtual AudioSource *CreateSource() = 0;

	virtual void SetDistanceModel(AudioDistanceModel model) = 0;

	virtual void SetListenerPosition(glm::vec3 &position) = 0;
	virtual void SetListenerVelocity(glm::vec3 &velocity) = 0;
	virtual void SetListenerOrientation(glm::vec3 &front, glm::vec3 &up) = 0;

	virtual void Update(double deltaTime) = 0;

	virtual void Release() = 0;

	virtual ~AudioSystem();

	static int InitInstance(const char *module);
	static AudioSystem *GetInstance();
	static void ReleaseInstance();
};
