/* Neko Engine
 *
 * Input.cpp
 * Author: Alexandru Naiman
 *
 * Input class implementation
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

#define ENGINE_INTERNAL

#include <Engine/Input.h>
#include <Engine/Engine.h>

using namespace std;

vector<uint8_t> Input::_pressedKeys;
unordered_map<int, uint8_t> Input::_keymap;
float Input::_screenHalfWidth = 0.f, Input::_screenHalfHeight = 0.f;
float Input::_mouseAxis[2] = { 0.f, 0.f };
unordered_map<std::string, uint8_t> Input::_buttonMap;
unordered_map<std::string, uint8_t> Input::_axisMap;
int Input::_connectedControllers = 0;
ControllerState Input::_controllerState[NE_MAX_CONTROLLERS];

int Input::Initialize()
{
	_InitializeKeymap();

	_screenHalfWidth = (float)Engine::GetScreenWidth() / 2.f;
	_screenHalfHeight = (float)Engine::GetScreenHeight() / 2.f;

	_connectedControllers = _GetControllerCount();

	return ENGINE_OK;
}

bool Input::GetKeyUp(uint8_t key) noexcept
{
	for (uint8_t a : _pressedKeys)
		if (a == key)
			return false;

	return true;
}

bool Input::GetKeyDown(uint8_t key) noexcept
{
	for (uint8_t a : _pressedKeys)
		if (a == key)
			return true;

	return false;
}

float Input::GetAxis(uint8_t axis) noexcept
{
	if (axis <= 0x01)
		return _mouseAxis[axis];

	uint8_t id = 0;
	while (axis >= 0x10)
	{
		axis -= 0x10;
		id++;
	}

	switch (axis)
	{
		case NE_GPAD0_LX: return _controllerState[id].left_x;
		case NE_GPAD0_LY: return _controllerState[id].left_y;
		case NE_GPAD0_RX: return _controllerState[id].right_x;
		case NE_GPAD0_RY: return _controllerState[id].right_y;
		case NE_GPAD0_TL: return _controllerState[id].left_trigger;
		case NE_GPAD0_TR: return _controllerState[id].right_trigger;
		default: return 0.f;
	}
}

void Input::Key(int key, bool isPressed) noexcept
{
	int code = _keymap[key];

	if (isPressed)
		_pressedKeys.push_back(code);
	else
		_pressedKeys.erase(remove(_pressedKeys.begin(), _pressedKeys.end(), code), _pressedKeys.end());
}

void Input::Update() noexcept
{
	// Controllers
	for (int i = 0; i < _connectedControllers; ++i)
		_GetControllerState(i, &_controllerState[i]);

	// Mouse
	long x = 0, y = 0;
	float xDelta = 0.f, yDelta = 0.f;

	if (Platform::GetPointerPosition(x, y))
	{
		xDelta = _screenHalfWidth - x;
		yDelta = _screenHalfHeight - y;

		Platform::SetPointerPosition((long)_screenHalfWidth, (long)_screenHalfHeight);
	}
	else if (!Platform::GetTouchMovementDelta(xDelta, yDelta))
		return;

	_mouseAxis[NE_MOUSE_X] = -(xDelta / _screenHalfWidth);
	_mouseAxis[NE_MOUSE_Y] = yDelta / _screenHalfHeight;
}

void Input::Release()
{
}
