/* NekoEngine
 *
 * CameraComponent.cpp
 * Author: Alexandru Naiman
 *
 * Camera implementation
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

#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/glm.hpp>

#include <Scene/Components/CameraComponent.h>
#include <Engine/Engine.h>
#include <Engine/DeferredBuffer.h>
#include <Engine/EngineUtils.h>
#include <Engine/ResourceManager.h>
#include <Engine/SoundManager.h>
#include <Engine/Input.h>
#include <Engine/CameraManager.h>

using namespace glm;

ENGINE_REGISTER_COMPONENT_CLASS(CameraComponent);

CameraComponent::CameraComponent(ComponentInitializer *initializer) : ObjectComponent(initializer),
	_cam(nullptr)
{
	vec3 tmp;

	if ((_cam = new Camera()) == nullptr)
	{ DIE("Out of resources"); }

	ArgumentMapType::iterator it;
	const char *ptr = nullptr;

	if (((it = initializer->arguments.find("fov")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_cam->SetFOV((float)atof(ptr));

	if (((it = initializer->arguments.find("near")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_cam->SetNear((float)atof(ptr));

	if (((it = initializer->arguments.find("far")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_cam->SetFar((float)atof(ptr));

	if (((it = initializer->arguments.find("view_distance")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_cam->SetViewDistance((float)atof(ptr));

	if (((it = initializer->arguments.find("fog_distance")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_cam->SetFogDistance((float)atof(ptr));

	if (((it = initializer->arguments.find("fog_color")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
	{
		EngineUtils::ReadFloatArray(ptr, 3, &tmp.x);
		_cam->SetFogColor(tmp);
	}

	if (((it = initializer->arguments.find("projection")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
	{
		size_t len = strlen(ptr);

		if (!strncmp(ptr, "perspective", len))
			_cam->SetProjection(ProjectionType::Perspective);
		else if (!strncmp(ptr, "ortographic", len))
			_cam->SetProjection(ProjectionType::Ortographic);
	}

	_UpdateView();
}

int CameraComponent::Load()
{
	int ret = ObjectComponent::Load();
	if (ret != ENGINE_OK)
		return ret;

	_cam->UpdateProjection();

	return ENGINE_OK;
}

void CameraComponent::_UpdateView() noexcept
{
	vec3 tmp = _parent->GetPosition() + _localPosition;
	_cam->SetPosition(tmp, false);

	tmp = _parent->GetRotation() + _localRotation;
	_cam->SetRotation(tmp, false);

	_cam->UpdateView();

	/*SoundManager::SetListenerPosition(pos.x, pos.y, pos.z);
	SoundManager::SetListenerOrientation(_front.x, _front.y, _front.z);*/
}
