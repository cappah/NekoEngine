/* Neko Engine
 *
 * TestGame.cpp
 * Author: Alexandru Naiman
 *
 * Minimal game module implementation 
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

#include <vector>

#include <glm/glm.hpp>

#include "TestGame.h"
#include "MovingObject.h"

using namespace std;
using namespace glm;

NEKO_GAME_MODULE_IMPL(TestGame);

int TestGame::Initialize()
{
	//
	return ENGINE_OK;
}

void TestGame::LoadObjectOptionalArguments(Object *obj, const vector<char*> &args)
{
	MovingObject *mObj = dynamic_cast<MovingObject *>(obj);

	if (mObj)
	{
		for (char* str : args)
		{
			if (strstr(str, "tr_none"))
				mObj->SetTrajectory(TrajectoryType::NoTrajectory);
			else if (strstr(str, "tr_linear"))
				mObj->SetTrajectory(TrajectoryType::Linear);
			else if (strstr(str, "tr_circular"))
				mObj->SetTrajectory(TrajectoryType::Circular);
			else if (strstr(str, "tr_radius"))
			{
				const char *pch = strchr(str, ':');
				mObj->SetMovementRadius((float)atof(pch + 1));
			}
			else if (strstr(str, "tr_speed"))
			{
				const char *pch = strchr(str, ':');
				mObj->SetMovementSpeed((float)atof(pch + 1));
			}
			else if (strstr(str, "tr_end"))
			{
				const char *pch = strchr(str, ':');
				vec3 vec;

				EngineUtils::ReadFloatArray(pch + 1, 3, &vec.x);
				mObj->SetDestination(vec);
			}
		}
	}
}

void TestGame::CleanUp()
{
	//
}
