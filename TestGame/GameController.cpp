/* Neko Engine
 *
 * GameController.cpp
 * Author: Alexandru Naiman
 *
 * GameController class implementation
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

#define TESTGAME_INTERNAL

#include <math.h>
#include <Engine/Engine.h>
#include <Engine/Input.h>
#include <Engine/SceneManager.h>

#include "TestGame.h"
#include "GameController.h"

REGISTER_OBJECT_CLASS(GameController);

GameController::GameController(ObjectInitializer *initializer) noexcept : Object(initializer)
{
}

int GameController::Load()
{
	int ret = Object::Load();

	if (ret != ENGINE_OK)
		return ret;

	return ENGINE_OK;
}

void GameController::Update(double deltaTime) noexcept
{
	Object::Update(deltaTime);

	if (Input::GetKeyDown(NE_KEY_ESCAPE))
		Engine::Exit();

	if (Input::GetKeyDown(NE_KEY_I))
		Engine::DrawStats(true);

	if (Input::GetKeyDown(NE_KEY_O))
		Engine::DrawStats(false);

	if (Input::GetKeyDown(NE_KEY_M))
		SceneManager::LoadNextScene();

	if (Input::GetKeyDown(NE_KEY_L))
		SceneManager::GetActiveScene()->SetDrawLights(!SceneManager::GetActiveScene()->GetDrawLights());

	if (Input::GetKeyDown(NE_KEY_P))
		Engine::SaveScreenshot();
}