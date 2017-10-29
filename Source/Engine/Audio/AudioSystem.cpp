/* NekoEngine
 *
 * AudioSystem.cpp
 * Author: Alexandru Naiman
 *
 * NekoEngine Audio System
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

#include <System/Logger.h>
#include <Engine/Version.h>
#include <Audio/AudioSystem.h>
#include <Platform/Platform.h>

#define ASYS_MODULE	"AudioSystem"

typedef AudioSystem *(*createAudioSystemProc)(void);

static AudioSystem *_instance{ nullptr };
static PlatformModuleType _audioSystemModule{ nullptr };

AudioSystem::~AudioSystem() { }

int AudioSystem::InitInstance(const char *module)
{
	if ((_audioSystemModule = Platform::LoadModule(module)) == nullptr)
	{
		Logger::Log(ASYS_MODULE, LOG_CRITICAL, "Failed to load module: %s", module);
		return ENGINE_FAIL;
	}

	createAudioSystemProc createAudioSystem = (createAudioSystemProc)Platform::GetProcAddress(_audioSystemModule, "createAudioSystem");
	if (!createAudioSystem)
	{
		Logger::Log(ASYS_MODULE, LOG_CRITICAL, "Invalid audio system module: %s", module);
		return ENGINE_FAIL;
	}

	if ((_instance = createAudioSystem()) == nullptr)
	{
		Logger::Log(ASYS_MODULE, LOG_CRITICAL, "Failed to create audio system instance from module: %s", module);
		return ENGINE_FAIL;
	}

	return _instance->Initialize();
}

AudioSystem *AudioSystem::GetInstance()
{
	return _instance;
}

void AudioSystem::ReleaseInstance()
{
	if (!_instance)
		return;

	_instance->Release();
	delete _instance;
	_instance = nullptr;

	Logger::Log(ASYS_MODULE, LOG_INFORMATION, "Released");
}