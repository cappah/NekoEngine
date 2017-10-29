/* NekoEngine
 *
 * GameController.cpp
 * Author: Alexandru Naiman
 *
 * GameController class implementation
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

#include <math.h>
#include <Input/Input.h>
#include <Engine/Console.h>
#include <Physics/Physics.h>
#include <Scene/SceneManager.h>

#include "TestGame.h"
#include "GameController.h"

using namespace glm;

REGISTER_OBJECT_CLASS(GameController);

static bool _menuVisible = false;

GameController::GameController(ObjectInitializer *initializer) noexcept : Object(initializer)
{
}

int GameController::Load()
{
	int ret = Object::Load();

	if (ret != ENGINE_OK)
		return ret;

	uint32_t top = (Engine::GetScreenHeight() - 102) / 3;

	_menuBox = new Box((Engine::GetScreenWidth() - 100) / 2, top, 100, 102);
	_menuBox->SetVisible(false);
	top += 10;

	vec3 labelColor = vec3(1.f);
	_menuLabel = new Label((Engine::GetScreenWidth() - 80) / 2, top, 80, 24);
	_menuLabel->SetText("Paused");
	_menuLabel->SetTextColor(labelColor);
	_menuLabel->SetVisible(false);
	top += 29;

	_resumeButton = new Button((Engine::GetScreenWidth() - 80) / 2, top, 80, 24);
	_resumeButton->SetText("Resume");
	_resumeButton->SetClickHandler([this](){
		_menuVisible = false;
		this->_ShowMenu(_menuVisible);
		Engine::TogglePause();
		Input::CapturePointer();
	});
	_resumeButton->SetVisible(false);
	top += 29;

	_exitButton = new Button((Engine::GetScreenWidth() - 80) / 2, top, 80, 24);
	_exitButton->SetText("Exit");
	_exitButton->SetClickHandler([]() { Console::ExecuteCommand("E_Exit()", false); });
	_exitButton->SetVisible(false);

	/*Slider *slider = new Slider(100, 100, 100, 24);
	slider->SetVisible(true);
	GUIManager::RegisterControl(slider);*/

	/*TextBox *tbx = new TextBox(100, 200, 100);
	tbx->SetVisible(true);
	GUIManager::RegisterControl(tbx);*/

	GUIManager::RegisterControl(_menuBox);
	GUIManager::RegisterControl(_menuLabel);
	GUIManager::RegisterControl(_resumeButton);
	GUIManager::RegisterControl(_exitButton);

	_updateWhilePaused = true;

	return ENGINE_OK;
}

void GameController::Update(double deltaTime) noexcept
{
	Object::Update(deltaTime);

	if (Input::GetButtonDown("pause"))
	{
		_menuVisible = !_menuVisible;

		if (_menuVisible)
			Input::ReleasePointer();
		else
			Input::CapturePointer();

		Engine::TogglePause();
		_ShowMenu(_menuVisible);
	}

	if (Input::GetButtonDown("show_stats"))
		Console::ExecuteCommand("E_ToggleStats()", false);

	if (Input::GetButtonDown("next_scene"))
		SceneManager::LoadNextScene();

	if (Input::GetButtonDown("screenshot"))
		Console::ExecuteCommand("screenshot", false);

	Ray r;
	vec2 coords{ .5f, .5f };
	if (Physics::GetInstance()->ScreenRayCast(&r, coords, 100.f))
		Logger::Log("GameController", LOG_INFORMATION, "Hit something");
}

bool GameController::Unload() noexcept
{
	if (!Object::Unload())
		return false;

	delete _exitButton;
	delete _menuBox;
	delete _menuLabel;
	delete _resumeButton;

	return true;
}

void GameController::_ShowMenu(bool show)
{
	_menuBox->SetVisible(show);
	_menuLabel->SetVisible(show);
	_resumeButton->SetVisible(show);
	_exitButton->SetVisible(show);
}

GameController::~GameController() noexcept
{
}
