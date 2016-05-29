/* Neko Engine
 *
 * MovingObject.h
 * Author: Alexandru Naiman
 *
 * MovingObject class definition 
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

#include <glm/glm.hpp>

#include <Scene/Object.h>

enum class TrajectoryType : unsigned short
{
	NoTrajectory = 0,
	Linear = 1,
	Circular = 2
};

class MovingObject :
	public Object
{
public:
	TESTGAME_API MovingObject(ObjectInitializer *initializer) noexcept;

	void TESTGAME_API SetTrajectory(TrajectoryType trajectory) noexcept { _trajectory = trajectory; }
	void TESTGAME_API SetDestination(glm::vec3 &destination) noexcept { _endPosition = destination; }
	void TESTGAME_API SetMovementSpeed(float speed) noexcept { _speed = speed; }
	void TESTGAME_API SetMovementRadius(float radius) noexcept { _radius = radius; }

	virtual int TESTGAME_API Load() override;
	virtual void TESTGAME_API Update(double deltaTime) noexcept override;

	virtual TESTGAME_API ~MovingObject() noexcept {};

protected:
	glm::vec3 _startPosition, _endPosition;
	float _speed, _radius;
	TrajectoryType _trajectory;
	float _lastDistance, _circularCounter;
};