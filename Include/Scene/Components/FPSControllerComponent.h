/* NekoEngine
 *
 * FPSControllerComponent.h
 * Author: Alexandru Naiman
 *
 * First Person Shooter controls
 * The object you attach this component to must have a CameraComponent named
 * FPSCamera.
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
#include <Scene/ObjectComponent.h>
#include <Scene/Components/CameraComponent.h>

#define DEFAULT_VSENS		2500.f
#define DEFAULT_HSENS		2500.f
#define DEFAULT_MOVE_SPD	40.f
#define DEFAULT_SPRINT_SPD	60.f
#define DEFAULT_ROTS		40.f

class FPSControllerComponent : public ObjectComponent
{
public:
	ENGINE_API FPSControllerComponent(ComponentInitializer *initializer);

	ENGINE_API void SetHorizontalSensivity(float sensivity) noexcept { _horizontalSensivity = sensivity; }
	ENGINE_API void SetVerticalSensivity(float sensivity) noexcept { _verticalSensivity = sensivity; }

	ENGINE_API virtual void Update(double deltaTime) noexcept override;

	ENGINE_API virtual void OnHit(Object *other, glm::vec3 &position) override;

	ENGINE_API virtual ~FPSControllerComponent() { Unload(); }

protected:
	CameraComponent *_cameraComponent;
	float _moveSpeed, _sprintSpeed;
	float _rotateSpeed;
	float _verticalSensivity;
	float _horizontalSensivity;
	glm::vec3 _allowedMovement;
};