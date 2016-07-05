/* Neko Engine
 *
 * AudioClip.cpp
 * Author: Alexandru Naiman
 *
 * AudioClip class implementation
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

#define ENGINE_INTERNAL

#include <string>
#include <cstring>

#include <Audio/AudioClip.h>
#include <Engine/Engine.h>
#include <System/AssetLoader/AssetLoader.h>

#define AC_MODULE	"AudioClip"

using namespace std;

int AudioClip::Load()
{
	ALsizei size = 0, freq = 0;
	ALenum format = 0;
	ALvoid *data = nullptr;
	int ret = ENGINE_FAIL;

	string path("/");
	path.append(GetResourceInfo()->filePath);

	if (path.substr(path.find_last_of(".") + 1) == "ogg")
		ret = AssetLoader::LoadOGG(path, &format, (unsigned char **)&data, &size, &freq);
	else if (path.substr(path.find_last_of(".") + 1) == "wav")
		ret = AssetLoader::LoadWAV(path, &format, &data, &size, &freq);

	if (ret != ENGINE_OK)
		return ENGINE_FAIL;

	AL_CHECK_FATAL(alGenBuffers(1, &_buffer));
	AL_CHECK_FATAL(alBufferData(_buffer, format, data, size, freq));

	free(data); data = nullptr;

	Logger::Log(AC_MODULE, LOG_DEBUG, "Loaded clip id %d from %s, size %ld kB, frequency %ld Hz, format 0x%x", _resourceInfo->id, path.c_str(), size / 1024, freq, format);

	return ENGINE_OK;
}

AudioClip::~AudioClip() noexcept
{
	alDeleteBuffers(1, &_buffer);
}
