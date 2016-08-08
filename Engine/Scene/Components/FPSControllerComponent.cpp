/* NekoEngine
 *
 * FPSControllerComponent.cpp
 * Author: Alexandru Naiman
 *
 * FPSControllerComponent component class implementation
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

#include <Engine/Input.h>

#include <Scene/Components/FPSControllerComponent.h>
#include <Scene/Components/CameraComponent.h>
#include <Engine/ResourceManager.h>
#include <Scene/Object.h>

#include <glm/glm.hpp>

using namespace glm;

ENGINE_REGISTER_COMPONENT_CLASS(FPSControllerComponent);

FPSControllerComponent::FPSControllerComponent(ComponentInitializer *initializer)
	: ObjectComponent(initializer),
	_moveSpeed(DEFAULT_MOVE_SPD),
	_sprintSpeed(DEFAULT_SPRINT_SPD),
	_rotateSpeed(DEFAULT_ROTS),
	_verticalSensivity(DEFAULT_VSENS),
	_horizontalSensivity(DEFAULT_HSENS)
{
	ArgumentMapType::iterator it;
	const char *ptr = nullptr;

	if (((it = initializer->arguments.find("move_speed")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_moveSpeed = (float)atof(ptr);

	if (((it = initializer->arguments.find("sprint_speed")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_sprintSpeed = (float)atof(ptr);

	if (((it = initializer->arguments.find("rotate_speed")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_rotateSpeed = (float)atof(ptr);

	if (((it = initializer->arguments.find("horizontal_sensivity")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_horizontalSensivity = (float)atof(ptr);

	if (((it = initializer->arguments.find("vertical_sensivity")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_verticalSensivity = (float)atof(ptr);
}

void FPSControllerComponent::Update(double deltaTime) noexcept
{
	ObjectComponent::Update(deltaTime);

	float vAngle = _verticalSensivity * Input::GetAxis("vertical") * (float)deltaTime;
	float hAngle = _horizontalSensivity * Input::GetAxis("horizontal") * (float)deltaTime;

	vec3 pos = _parent->GetPosition();
	vec3 rot = _parent->GetRotation();

	rot.x += RAD2DEG(vAngle);
	rot.y += RAD2DEG(hAngle);

	float speed = _moveSpeed;

	if (Input::GetKeyDown("sprint"))
		speed = _sprintSpeed;

	float velocity = speed * (float)deltaTime;

	CameraComponent *cam = (CameraComponent*)_parent->GetComponent("FPSCamera");

	if (Input::GetKeyDown("forward"))
		pos += cam->GetForward() * velocity * vec3(1.f, 0.f, 1.f);
	else if (Input::GetKeyDown("back"))
		pos -= cam->GetForward() * velocity * vec3(1.f, 0.f, 1.f);

	if (Input::GetKeyDown("right"))
		pos += cam->GetRight() * velocity;
	else if (Input::GetKeyDown("left"))
		pos -= cam->GetRight() * velocity;

	if (Input::GetKeyDown("rot_right"))
		rot.y += _rotateSpeed * (float)deltaTime;
	else if (Input::GetKeyDown("rot_left"))
		rot.y -= _rotateSpeed * (float)deltaTime;

	if (Input::GetKeyDown("rot_up"))
		rot.x -= _rotateSpeed * (float)deltaTime;
	else if (Input::GetKeyDown("rot_down"))
		rot.x += _rotateSpeed * (float)deltaTime;

	rot.x = clamp(rot.x, -60.f, 85.f);

	_parent->SetPosition(pos);
	_parent->SetRotation(rot);
}
