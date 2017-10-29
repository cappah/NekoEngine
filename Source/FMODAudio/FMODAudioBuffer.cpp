/* NekoEngine
 *
 * FMODAudioBuffer.cpp
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

#include <fmod_errors.h>
#include <System/Logger.h>

#include "FMODAudioBuffer.h"

#define FMOD_ABUFF	"FMODAudioBuffer"

struct FM_FORMAT
{
	FMOD_SOUND_FORMAT format;
	uint32_t channels;
};

FM_FORMAT _fmod_audioFormat[]
{
	{ FMOD_SOUND_FORMAT_PCM8, 1 },
	{ FMOD_SOUND_FORMAT_PCM16, 1 },
	{ FMOD_SOUND_FORMAT_PCM8, 2 },
	{ FMOD_SOUND_FORMAT_PCM16, 2 }
};

FMODAudioBuffer::FMODAudioBuffer(size_t size) :
	AudioBuffer(size)
{
	_sound = nullptr;
}

void FMODAudioBuffer::SetData(AudioFormat format, size_t frequency, size_t size, void *data)
{
	FM_FORMAT &fmt = _fmod_audioFormat[(int)format];

	FMOD_CREATESOUNDEXINFO exInfo{};
	exInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	exInfo.format = fmt.format;
	exInfo.defaultfrequency = frequency;
	exInfo.length = size;
	exInfo.numchannels = fmt.channels;
	exInfo.suggestedsoundtype = FMOD_SOUND_TYPE_RAW;
	
	FMOD_RESULT res{ FMOD_System_CreateSound(_system, (char *)data, FMOD_OPENRAW | FMOD_OPENMEMORY | FMOD_3D, &exInfo, &_sound) };
	if (res != FMOD_OK) Logger::Log(FMOD_ABUFF, LOG_WARNING, "Failed create sound: %s (%d)", FMOD_ErrorString(res), res);
}

FMODAudioBuffer::~FMODAudioBuffer()
{
	FMOD_Sound_Release(_sound);
}