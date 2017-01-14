/* NekoEngine
 *
 * Camera.h
 * Author: Alexandru Naiman
 *
 * Camera
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

#pragma once

#include <Engine/Engine.h>
#include <Runtime/Runtime.h>

enum class ProjectionType : unsigned short
{
	Perspective = 0,
	Ortographic = 1
};

class ENGINE_API Camera
{
public:
	Camera(bool noRegister = false);

	glm::vec3 &GetFogColor() noexcept { return _fogColor; }
	glm::vec3 &GetForward() noexcept { return _front; }
	glm::vec3 &GetRight() noexcept { return _right; }
	glm::vec3 &GetPosition() noexcept { return _position; }
	glm::vec3 &GetRotation() noexcept { return _rotation; }
	glm::mat4 &GetView() noexcept { return _drawSkybox ? _skyboxView : _view; }
	glm::mat4 &GetProjectionMatrix() noexcept { return _drawSkybox ? _skyboxProjectionMatrix : _projectionMatrix; }
	float GetNear() noexcept { return _near; }
	float GetFar() noexcept { return _far; }
	float GetFOV() noexcept { return _fov; }
	float GetViewDistance() noexcept { return _viewDistance; }
	float GetFogDistance() noexcept { return _fogDistance; }
	ProjectionType GetProjection() noexcept { return _projection; }
	NFrustum &GetFrustum() noexcept { return _frustum; }

	void SetPosition(glm::vec3 &position, bool update = true) noexcept { _position = position; if(update) UpdateView(); }
	void SetRotation(glm::vec3 &rotation, bool update = true) noexcept { _rotation = rotation; if(update) UpdateView(); }
	void SetNear(float Near) noexcept { _near = Near; }
	void SetFar(float Far) noexcept { _far = Far; }
	void SetFOV(float fov) noexcept { _fov = fov; }
	void SetFogColor(glm::vec3 &color) noexcept { _fogColor = color; }
	void SetViewDistance(float distance) noexcept { _viewDistance = distance; }
	void SetFogDistance(float distance) noexcept { _fogDistance = distance; }
	void SetProjection(ProjectionType projection) noexcept { _projection = projection; }
	void SetProjectionMatrix(glm::mat4 &matrix) noexcept { _projectionMatrix = matrix; }
	void SetView(glm::mat4 &matrix) noexcept { _view = matrix; }

	void EnableSkybox(bool enable) { _drawSkybox = enable; }

	void Ortho(float left, float right, float top, float bottom, float zNear, float zFar);
	void LookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up) noexcept;
	void UpdateProjection() noexcept;
	void UpdateView() noexcept;

	void MoveForward(float distance) noexcept { _position += _front * distance; }
	void MoveRight(float distance) noexcept { _position += _right * distance; }
	void MoveUp(float distance) noexcept { _position += _up * distance; }

	void RotateX(float angle) noexcept { _rotation.x += angle; }
	void RotateY(float angle) noexcept { _rotation.y += angle; }
	void RotateZ(float angle) noexcept { _rotation.z += angle; }

	~Camera() noexcept { }

protected:
	glm::vec3 _front;
	glm::vec3 _up;
	glm::vec3 _right;
	glm::vec3 _worldUp;

	glm::vec3 _position;
	glm::vec3 _rotation;

	float _near;
	float _far;
	float _fov;

	float _viewDistance;
	float _fogDistance;
	glm::vec3 _fogColor;

	ProjectionType _projection;

	glm::mat4 _view, _skyboxView;
	glm::mat4 _projectionMatrix, _skyboxProjectionMatrix;
	NFrustum _frustum;

	bool _drawSkybox;
};
