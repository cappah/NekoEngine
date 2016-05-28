/* Neko Engine
 *
 * Camera.h
 * Author: Alexandru Naiman
 *
 * Fly camera
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

#pragma once

#include <Engine/Engine.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class ProjectionType : unsigned short
{
	Perspective = 0,
	Ortographic = 1
};

#ifdef NE_PLATFORM_IOS
#define DEFAULT_VSENS	.75f
#define DEFAULT_HSENS	.75f
#define DEFAULT_TRANS	300.f
#define DEFAULT_TRANS_F	175.f
#define DEFAULT_ROTS	40.f
#else
#define DEFAULT_VSENS	.10f
#define DEFAULT_HSENS	.10f
#define DEFAULT_TRANS	250.f
#define DEFAULT_TRANS_F	370.f
#define DEFAULT_ROTS	40.f
#endif

class ENGINE_API Camera
{
public:
	Camera() noexcept :
		_position(glm::vec3(0, 0, 0)),
		_front(glm::vec3(0, 0, 1)),
		_up(glm::vec3(0, 1, 0)),
		_right(glm::vec3(0, 0, 0)),
		_worldUp(glm::vec3(0, 1, 0)),
		_translateSpeed(DEFAULT_TRANS),
		_fastTranslateSpeed(DEFAULT_TRANS_F),
		_rotateSpeed(DEFAULT_ROTS),
		_verticalSensivity(DEFAULT_VSENS), 
		_horizontalSensivity(DEFAULT_HSENS), 
		_xDelta(0.f),
		_yDelta(0.f),
		_fps(false),
		_zoom(0.f),
		_near(.2f),
		_far(1000.f),
		_fov(45.f),
		_viewDistance(1000.f),
		_fogDistance(1200.f),
		_projection(ProjectionType::Perspective),
		_id(0)
	{ _UpdateView(); }

	int GetId() noexcept { return _id; }
	glm::vec3& GetPosition() noexcept { return _position; }
	glm::vec3& GetFogColor() noexcept { return _fogColor; }
	glm::mat4& GetView() noexcept { return _view; }
	glm::mat4& GetProjectionMatrix() noexcept { return _projectionMatrix; }
	bool GetFPSCamera() { return _fps; }
	float GetNear() noexcept { return _near; }
	float GetFar() noexcept { return _far; }
	float GetFOV() noexcept { return _fov; }
	float GetViewDistance() noexcept { return _viewDistance; }
	float GetFogDistance() noexcept { return _fogDistance; }
	ProjectionType GetProjection() noexcept { return _projection; }

	void SetId(int id) noexcept { _id = id; }
	void SetPosition(glm::vec3& pos) { _position = pos; }
	void SetRotation(glm::vec3& rot) { _rotation = rot; }
	void SetFPSCamera(bool fps) { _fps = fps; }
	void SetNear(float Near) noexcept { _near = Near; }
	void SetFar(float Far) noexcept { _far = Far; }
	void SetFOV(float fov) noexcept { _fov = fov; }
	void SetFogColor(glm::vec3& color) noexcept { _fogColor = color; }
	void SetViewDistance(float distance) noexcept { _viewDistance = distance; }
	void SetFogDistance(float distance) noexcept { _fogDistance = distance; }
	void SetProjection(ProjectionType projection) noexcept { _projection = projection; }
	void SetHorizontalSensivity(float sensivity) noexcept { _horizontalSensivity = sensivity; }
	void SetVerticalSensivity(float sensivity) noexcept { _verticalSensivity = sensivity; }
	void SetRotationDelta(float x, float y) noexcept { _xDelta = x, _yDelta = y; }
	void UpdatePerspective() noexcept;

	void Initialize() noexcept;

	void Update(double deltaTime) noexcept;

	void MoveForward(float distance) noexcept { _position += _front * distance; }
	void MoveRight(float distance) noexcept { _position += _right * distance; }
	void MoveUp(float distance) noexcept { _position += _up * distance; }

	void RotateX(float angle) noexcept { _rotation.x += angle; }
	void RotateY(float angle) noexcept { _rotation.y += angle; }
	void RotateZ(float angle) noexcept { _rotation.z += angle; }

	~Camera() noexcept;

private:
	glm::vec3 _position;
	glm::vec3 _front;
	glm::vec3 _up;
	glm::vec3 _right;
	glm::vec3 _worldUp;
	glm::vec3 _rotation;
	
	float _translateSpeed, _fastTranslateSpeed;
	float _rotateSpeed;
	float _verticalSensivity;
	float _horizontalSensivity;
	float _zoom;

	float _near;
	float _far;
	float _fov;

	float _xDelta, _yDelta;

	float _viewDistance;
	float _fogDistance;
	glm::vec3 _fogColor;

	ProjectionType _projection;

	int _id;

	glm::mat4 _view;
	glm::mat4 _projectionMatrix;

	bool _fps;

	void _UpdateView() noexcept;
};

