/* NekoEngine
 *
 * FPSControllerComponent.cpp
 * Author: Alexandru Naiman
 *
 * FPSControllerComponent component class implementation
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

#include <Input/Input.h>

#include <Scene/Components/FPSControllerComponent.h>
#include <Scene/Components/CameraComponent.h>
#include <Engine/ResourceManager.h>
#include <Scene/CameraManager.h>
#include <Scene/Object.h>

using namespace glm;
using namespace std;

ENGINE_REGISTER_COMPONENT_CLASS(FPSControllerComponent);

FPSControllerComponent::FPSControllerComponent(ComponentInitializer *initializer)
	: ObjectComponent(initializer),
	_cameraComponent(nullptr),
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

	ComponentInitializer init;
	init.parent = _parent;
	init.arguments.insert(make_pair("near", "0.1"));
	init.arguments.insert(make_pair("far", "1000.0"));
	init.arguments.insert(make_pair("fov", "90"));
	init.arguments.insert(make_pair("projection", "perspective"));
	init.arguments.insert(make_pair("position", "-40.0, 30.0, 0.0"));
	init.arguments.insert(make_pair("rotation", "0.0, 0.0, 0.0"));
	init.arguments.insert(make_pair("fog_color", "0.207, 0.255, 0.349"));
	init.arguments.insert(make_pair("view_distance", "300"));
	init.arguments.insert(make_pair("fog_distance", "500"));

	_cameraComponent = (CameraComponent *)Engine::NewComponent("CameraComponent", &init);
	_cameraComponent->Load();
	_parent->AddComponent("FPSCamera", _cameraComponent);

	CameraManager::SetActiveCamera(_cameraComponent->GetCamera());
}

void FPSControllerComponent::Update(double deltaTime) noexcept
{
	ObjectComponent::Update(deltaTime);

	if (!_cameraComponent) _cameraComponent = (CameraComponent *)_parent->GetComponent("FPSCamera");
	Camera *cam = _cameraComponent->GetCamera();
	
	float vAngle = _verticalSensivity * Input::GetAxis("vertical");
	float hAngle = _horizontalSensivity * Input::GetAxis("horizontal");

	vec3 pos = _parent->GetPosition();
	vec3 rot = _parent->GetRotationAngles();

	rot.x += radians(vAngle);
	rot.y += radians(hAngle);

	float speed = _moveSpeed;

	if (Input::GetButton("sprint"))
		speed = _sprintSpeed;

	float velocity = speed * (float)deltaTime;

	if (Input::GetButton("forward"))
		pos += cam->GetForward() * velocity * vec3(1.f, 0.f, 1.f);
	else if (Input::GetButton("back"))
		pos -= cam->GetForward() * velocity * vec3(1.f, 0.f, 1.f);

	if (Input::GetButton("right"))
		pos += cam->GetRight() * velocity;
	else if (Input::GetButton("left"))
		pos -= cam->GetRight() * velocity;

	if (Input::GetButton("rot_right"))
		rot.y += _rotateSpeed * (float)deltaTime;
	else if (Input::GetButton("rot_left"))
		rot.y -= _rotateSpeed * (float)deltaTime;

	if (Input::GetButton("rot_up"))
		rot.x -= _rotateSpeed * (float)deltaTime;
	else if (Input::GetButton("rot_down"))
		rot.x += _rotateSpeed * (float)deltaTime;

	rot.x = glm::clamp(rot.x, -60.f, 85.f);

	pos *= _allowedMovement;

	_parent->SetPosition(pos);
	_parent->SetRotation(rot);

	_allowedMovement = vec3(1.f);
}

void FPSControllerComponent::OnHit(Object *other, glm::vec3 &position)
{
	vec3 localHitPos = _parent->GetPosition() - position;
	
	float x{ abs(localHitPos.x) };
	float z{ abs(localHitPos.z) };

	if (x > z)
		_allowedMovement = vec3(0.f, 1.f, 1.f);
	else
		_allowedMovement = vec3(1.f, 1.f, 0.f);
}