/* NekoEngine
 *
 * RigidBody.h
 * Author: Alexandru Naiman
 *
 * RigidBody Interface
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

#include <Engine/Defs.h>

class RigidBody
{
public:
	RigidBody();

	virtual const glm::vec3 &GetGravity() const = 0;
	virtual const glm::vec3 &GetTotalForce() const = 0;
	virtual const glm::vec3 &GetTotalTorque() const = 0;
	virtual float GetLinearDamping() const = 0;
	virtual float GetAngularDamping() const = 0;
	virtual float GetLinearSleepingThreshold() const = 0;
	virtual float GetAngularSleepingThreshold() const = 0;
	virtual const glm::vec3 &GetLinearFactor() const = 0;
	virtual float GetInverseMass() const = 0;
	virtual const glm::mat3 &GetInverseInertiaTensorWorld() const = 0;

	virtual void SetGravity(const glm::vec3 &acceleration) = 0;
	virtual void SetSleepingThresholds(float linear, float angular) = 0;
	virtual void SetMass(float mass) = 0;
	virtual void SetLinearFactor(const glm::vec3 &factor) = 0;

	virtual void ApplyDamping() = 0;
	virtual void ApplyGravity() = 0;
	virtual void ApplyTorque(const glm::vec3 &torque) = 0;
	virtual void ApplyForce(const glm::vec3 &force) = 0;
	virtual void ApplyCentralImpulse(const glm::vec3 &impulse) = 0;
	virtual void ApplyTorqueImpulse(const glm::vec3 &torque) = 0;
	virtual void ApplyImpulse(const glm::vec3 &impulse, const glm::vec3 &torque) = 0;

	virtual void ClearForces() = 0;
	virtual void UpdateInertiaTensor() = 0;

	virtual ~RigidBody();
};