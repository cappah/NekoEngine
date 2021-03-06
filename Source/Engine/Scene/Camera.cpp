/* NekoEngine
 *
 * Camera.cpp
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

#include <Scene/Camera.h>
#include <Scene/CameraManager.h>
#include <System/Logger.h>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace glm;

Camera::Camera(bool noRegister) :
	_front(glm::vec3(0, 0, 1)),
	_up(glm::vec3(0, 1, 0)),
	_right(glm::vec3(0, 0, 0)),
	_worldUp(glm::vec3(0, 1, 0)),
	_position(glm::vec3(0, 0, 0)),
	_rotation(glm::vec3(0, 0, 0)),
	_near(.2f),
	_far(1000.f),
	_fov(45.f),
	_viewDistance(1000.f),
	_fogDistance(1200.f),
	_fogColor(glm::vec3(0, 0, 0)),
	_projection(ProjectionType::Perspective),
	_view(glm::mat4(0.f)),
	_skyboxView(glm::mat4(0.f)),
	_projectionMatrix(glm::mat4(0.f)),
	_skyboxProjectionMatrix(glm::mat4(0.f)),
	_drawSkybox(false)
{
	UpdateView();
	UpdateProjection();

	if(!noRegister) CameraManager::AddCamera(this);
}

void Camera::Ortho(float left, float right, float top, float bottom, float zNear, float zFar)
{
	_projectionMatrix = ortho(left, right, top, bottom, zNear, zFar);
	_projectionMatrix[1][1] *= -1;
	_projection = ProjectionType::Ortographic;
}

void Camera::LookAt(vec3 eye, vec3 center, vec3 up) noexcept
{
	_view = lookAt(eye, center, up);
	_projection = ProjectionType::Perspective;
}

void Camera::UpdateProjection() noexcept
{
	if (_projection == ProjectionType::Perspective)
	{
		_projectionMatrix = perspective(radians(_fov / 2.f), (float)Engine::GetScreenWidth() / (float)Engine::GetScreenHeight(), _near, _far);
		_skyboxProjectionMatrix = infinitePerspective(radians(_fov / 2.f), (float)Engine::GetScreenWidth() / (float)Engine::GetScreenHeight(), _near);
	}
	else
		_projectionMatrix = ortho(0.f, (float)Engine::GetScreenWidth(), (float)Engine::GetScreenHeight(), 0.f, _near, _far);

	_projectionMatrix[1][1] *= -1;
	_skyboxProjectionMatrix[1][1] *= -1;
}

void Camera::UpdateView() noexcept
{
	vec3 front;

	front.x = cos(radians(_rotation.y)) * cos(radians(_rotation.x));
	front.y = sin(radians(_rotation.x));
	front.z = sin(radians(_rotation.y)) * cos(radians(_rotation.x));

	_front = normalize(front);
	_right = normalize(cross(_front, _worldUp));
	_up = normalize(cross(_right, _front));

	_view = lookAt(_position, _position + _front, _up);
	_skyboxView = mat4(mat3(_view));

	mat4 viewProjection = _projectionMatrix * _view;
	_frustum.FromViewProjection(viewProjection);
}