/* NekoEngine
 *
 * FMODAudioSource.h
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

#include "FMODAudio.h"

class FMODAudioSource : public AudioSource
{
public:
	FMODAudioSource() noexcept;

	virtual void SetPitch(float p) noexcept override;
	virtual void SetGain(float g) noexcept override;

	virtual void SetConeInnerAngle(float a) noexcept override;
	virtual void SetConeOuterAngle(float a) noexcept override;
	virtual void SetConeOuterGain(float g) noexcept override;
	
	virtual void SetDirection(glm::vec3 &dir) noexcept override;
	virtual void SetPosition(glm::vec3 &pos) noexcept override;
	virtual void SetVelocity(glm::vec3 &v) noexcept override;

	virtual void SetLooping(bool looping) noexcept override;

	virtual void SetMaxDistance(float maxDistance) noexcept override;
	virtual void SetReferenceDistance(float referenceDistance) noexcept override;

	virtual int SetClip(AudioClip *clip) noexcept override;

	virtual bool Play() noexcept override;
	virtual void Pause() noexcept override;
	virtual void Stop() noexcept override;
	virtual void Rewind() noexcept override;
	virtual bool IsPlaying() noexcept override;

	virtual ~FMODAudioSource();

private:
	FMOD_CHANNEL *_channel;
	float _minDistance, _maxDistance;
	float _coneIn, _coneOut, _coneVol;
	FMOD_VECTOR _position, _velocity, _panPos, _coneDirection;
//	float 
};

