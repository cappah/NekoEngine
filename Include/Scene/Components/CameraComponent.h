/* NekoEngine
 *
 * CameraComponent.h
 * Author: Alexandru Naiman
 *
 * Camera component
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

#include <Engine/Engine.h>
#include <Scene/ObjectComponent.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class ProjectionType : unsigned short
{
	Perspective = 0,
	Ortographic = 1
};

class ENGINE_API CameraComponent : public ObjectComponent
{
public:
	CameraComponent(ComponentInitializer *initializer);

	glm::vec3& GetFogColor() noexcept { return _fogColor; }
	glm::vec3& GetForward() noexcept { return _front; }
	glm::vec3& GetRight() noexcept { return _right; }
	glm::mat4& GetView() noexcept { return _drawSkybox ? _skyboxView : _view; }
	glm::mat4& GetProjectionMatrix() noexcept { return _drawSkybox ? _skyboxProjectionMatrix : _projectionMatrix; }
	float GetNear() noexcept { return _near; }
	float GetFar() noexcept { return _far; }
	float GetFOV() noexcept { return _fov; }
	float GetViewDistance() noexcept { return _viewDistance; }
	float GetFogDistance() noexcept { return _fogDistance; }
	ProjectionType GetProjection() noexcept { return _projection; }

	void SetNear(float Near) noexcept { _near = Near; }
	void SetFar(float Far) noexcept { _far = Far; }
	void SetFOV(float fov) noexcept { _fov = fov; }
	void SetFogColor(glm::vec3& color) noexcept { _fogColor = color; }
	void SetViewDistance(float distance) noexcept { _viewDistance = distance; }
	void SetFogDistance(float distance) noexcept { _fogDistance = distance; }
	void SetProjection(ProjectionType projection) noexcept { _projection = projection; }

	void EnableSkybox(bool enable) { _drawSkybox = enable; }
	
	virtual void UpdatePosition() noexcept override { ObjectComponent::UpdatePosition(); _UpdateView(); }
	void UpdatePerspective() noexcept;

	virtual int Load() override;

	void MoveForward(float distance) noexcept { _localPosition += _front * distance; }
	void MoveRight(float distance) noexcept { _localPosition += _right * distance; }
	void MoveUp(float distance) noexcept { _localPosition += _up * distance; }

	void RotateX(float angle) noexcept { _localRotation.x += angle; }
	void RotateY(float angle) noexcept { _localRotation.y += angle; }
	void RotateZ(float angle) noexcept { _localRotation.z += angle; }

	~CameraComponent() noexcept { }

protected:
	glm::vec3 _front;
	glm::vec3 _up;
	glm::vec3 _right;
	glm::vec3 _worldUp;

	float _near;
	float _far;
	float _fov;

	float _viewDistance;
	float _fogDistance;
	glm::vec3 _fogColor;

	ProjectionType _projection;

	glm::mat4 _view, _skyboxView;
	glm::mat4 _projectionMatrix, _skyboxProjectionMatrix;

	bool _noRegister, _drawSkybox;

	void _UpdateView() noexcept;
};