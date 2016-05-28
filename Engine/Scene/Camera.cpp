/* Neko Engine
 *
 * Camera.cpp
 * Author: Alexandru Naiman
 *
 * Camera implementation
 *
 * ----------------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman <alexandru dot naiman at icloud dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ALEXANDRU NAIMAN
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#define ENGINE_INTERNAL

#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/glm.hpp>

#include <Scene/Camera.h>
#include <Engine/Engine.h>
#include <Engine/DeferredBuffer.h>
#include <Engine/EngineUtils.h>
#include <Engine/ResourceManager.h>
#include <Engine/SoundManager.h>

using namespace glm;

void Camera::Initialize() noexcept
{
	UpdatePerspective();
}

void Camera::UpdatePerspective() noexcept
{
	_projectionMatrix = perspective(_fov, (float)DeferredBuffer::GetWidth() / (float)DeferredBuffer::GetHeight(), _near, _far);
}

void Camera::Update(double deltaTime) noexcept
{
	float hAngle = _horizontalSensivity * _xDelta * deltaTime;
	float vAngle = _verticalSensivity * _yDelta * deltaTime;

	_xDelta = 0.f;
	_yDelta = 0.f;

	// rotate
	_rotation.x += RAD2DEG(vAngle);
	_rotation.y -= RAD2DEG(hAngle);

/*	if (Engine::GetKeyDown('x'))
		MoveUp(speed * _deltaTime);
	else if (Engine::GetKeyDown('z'))
		MoveUp(-speed * _deltaTime);

	if (Engine::GetKeyDown('e'))
		RotateZ(_rotateSpeed * _deltaTime);
	else if (Engine::GetKeyDown('q'))
		RotateZ(-_rotateSpeed * _deltaTime);*/

	float speed = _translateSpeed;

	if (Engine::GetKeyDown(KEY_LSHIFT))
		speed = _fastTranslateSpeed;

	float velocity = speed * deltaTime;

	if (Engine::GetKeyDown('w'))
		_position += _front * velocity * (_fps ? vec3(1.f, 0.f, 1.f) : vec3(1.f));
	else if (Engine::GetKeyDown('s'))
		_position -= _front * velocity * (_fps ? vec3(1.f, 0.f, 1.f) : vec3(1.f));

	if (Engine::GetKeyDown('d'))
		_position += _right * velocity;
	else if(Engine::GetKeyDown('a'))
		_position -= _right * velocity;

	if (Engine::GetKeyDown(KEY_RIGHTARROW))
		_rotation.y += _rotateSpeed * deltaTime;
	else if (Engine::GetKeyDown(KEY_LEFTARROW))
		_rotation.y -= _rotateSpeed * deltaTime;

	if (Engine::GetKeyDown(KEY_UPARROW))
		_rotation.x -= _rotateSpeed * deltaTime;
	else if (Engine::GetKeyDown(KEY_DOWNARROW))
		_rotation.x += _rotateSpeed * deltaTime;

	if (_fps)
		_rotation.x = clamp(_rotation.x, -60.f, 85.f);

	_UpdateView();

	SoundManager::SetListenerPosition(_position.x, _position.y, _position.z);
	SoundManager::SetListenerOrientation(_front.x, _front.y, _front.z);
}

void Camera::_UpdateView() noexcept
{
	vec3 front;

	front.x = cos(radians(_rotation.y)) * cos(radians(_rotation.x));
	front.y = sin(radians(_rotation.x));
	front.z = sin(radians(_rotation.y)) * cos(radians(_rotation.x));

	_front = normalize(front);
	_right = normalize(cross(_front, _worldUp));
	_up = normalize(cross(_right, _front));

	_view = lookAt(_position, _position + _front, _up);
}

Camera::~Camera() noexcept
{
}
