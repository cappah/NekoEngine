/* Neko Engine
 *
 * MovingObject.cpp
 * Author: Alexandru Naiman
 *
 * MovingObject class implementation 
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

#define TESTGAME_INTERNAL

#include <math.h>
#include <Engine/Engine.h>

#include "TestGame.h"
#include "MovingObject.h"

REGISTER_OBJECT_CLASS(MovingObject);

MovingObject::MovingObject(ObjectInitializer *initializer) noexcept : Object(initializer),
	_lastDistance(0.f),
	_circularCounter(0.f)
{
	_startPosition = glm::vec3(0.f, 0.f, 0.f);
	
	_speed = 50.f;
	
	const char *ptr = initializer->arguments.find("trajectory")->second.c_str();
	size_t len = strlen(ptr);
	
	if (!strncmp(ptr, "none", len))
		_trajectory = TrajectoryType::NoTrajectory;
	else if (!strncmp(ptr, "linear", len))
		_trajectory = TrajectoryType::Linear;
	else if (!strncmp(ptr, "circular", len))
		_trajectory = TrajectoryType::Circular;
	else
		_trajectory = TrajectoryType::NoTrajectory;
	
	ptr = initializer->arguments.find("radius")->second.c_str();
	_radius = ptr ? (float)atof(ptr) : 10.f;
	
	
	ptr = initializer->arguments.find("speed")->second.c_str();
	_speed = ptr ? (float)atof(ptr) : 50.f;
	
	ptr = initializer->arguments.find("end")->second.c_str();
	if(ptr)
		EngineUtils::ReadFloatArray(ptr, 3, &_endPosition.x);
	else
		_endPosition = glm::vec3(0.f, 0.f, 0.f);
}

int MovingObject::Load()
{
	int ret = Object::Load();

	if (ret != ENGINE_OK)
		return ret;

	if (_trajectory == TrajectoryType::Linear)
	{
		_startPosition = _position;
		LookAt(_endPosition);
	}
	else if (_trajectory == TrajectoryType::Circular)
		_radius /= 10.f;

	return ENGINE_OK;
}

void MovingObject::Update(double deltaTime) noexcept
{
	Object::Update(deltaTime);

	if (_trajectory == TrajectoryType::Linear)
	{
		float distance = glm::distance(_position, _endPosition);

		if (_lastDistance > 0.f && distance > _lastDistance)
		{
			glm::vec3 temp = _endPosition;
			_endPosition = _startPosition;
			_startPosition = temp;
			_lastDistance = 0.f;
			LookAt(_endPosition);
		}
		else
			_lastDistance = distance;

		MoveForward(_speed * deltaTime);
	}
	else if (_trajectory == TrajectoryType::Circular)
	{
		_circularCounter += deltaTime;

		glm::vec3 pos = _position;
		pos.x += cosf(_speed * _circularCounter) * _radius;
		pos.z += sinf(_speed * _circularCounter) * _radius;

		SetPosition(pos);
	}
}