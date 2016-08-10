/* NekoEngine
 *
 * MovingObject.cpp
 * Author: Alexandru Naiman
 *
 * MovingObject class implementation
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


#include <math.h>
#include <Engine/Engine.h>

#include "TestGame.h"
#include "MovingObject.h"

using namespace glm;

REGISTER_OBJECT_CLASS(MovingObject);

MovingObject::MovingObject(ObjectInitializer *initializer) noexcept : Object(initializer),
	_startPosition(vec3(0.f)), _endPosition(vec3(0.f)),
	_speed(50.f), _radius(10.f),
	_trajectory(TrajectoryType::NoTrajectory),
	_lastDistance(0.f),
	_circularCounter(0.f)
{
	ArgumentMapType::iterator it = initializer->arguments.find("trajectory");
	const char *ptr = nullptr;

	if (it != initializer->arguments.end())
	{
		if((ptr = it->second.c_str()))
		{
			size_t len = strlen(ptr);

			if (!strncmp(ptr, "none", len))
				_trajectory = TrajectoryType::NoTrajectory;
			else if (!strncmp(ptr, "linear", len))
				_trajectory = TrajectoryType::Linear;
			else if (!strncmp(ptr, "circular", len))
				_trajectory = TrajectoryType::Circular;
		}
	}

	if (((it = initializer->arguments.find("radius")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_radius = (float)atof(ptr);

	if (((it = initializer->arguments.find("speed")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_speed = (float)atof(ptr);

	if (((it = initializer->arguments.find("end")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		EngineUtils::ReadFloatArray(ptr, 3, &_endPosition.x);
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

		MoveForward(_speed * (float)deltaTime);
	}
	else if (_trajectory == TrajectoryType::Circular)
	{
		_circularCounter += (float)deltaTime;

		glm::vec3 pos = _position;
		pos.x += cosf(_speed * _circularCounter) * _radius;
		pos.z += sinf(_speed * _circularCounter) * _radius;

		SetPosition(pos);
	}
}
