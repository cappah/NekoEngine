/* NekoEngine
 *
 * CameraManager.h
 * Author: Alexandru Naiman
 *
 * CameraManager class definition
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

#pragma once

#include <vector>
#include <string>

#include <Engine/Engine.h>
#include <Scene/Components/CameraComponent.h>

class CameraManager
{
public:
	ENGINE_API static CameraComponent* GetActiveCamera() noexcept
	{
		if (!_activeCamera)
			_activeCamera = _cameras[0];

		return _activeCamera;
	}

	ENGINE_API static void SetActiveCamera(CameraComponent *cam) noexcept { _activeCamera = cam; }
	ENGINE_API static void SetActiveCameraId(int id) noexcept { _activeCamera = _cameras[id]; }
	ENGINE_API static void AddCamera(CameraComponent *cam) noexcept { _cameras.push_back(cam); }
	ENGINE_API static size_t Count() noexcept { return _cameras.size(); }

private:
	static std::vector<CameraComponent *> _cameras;
	static CameraComponent *_activeCamera;

	CameraManager() { _cameras.clear(); }
};
