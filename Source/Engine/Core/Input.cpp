/* NekoEngine
 *
 * Input.cpp
 * Author: Alexandru Naiman
 *
 * Input class implementation
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

#include <Engine/Input.h>
#include <Engine/Engine.h>

using namespace std;

#define INPUT_MODULE	"Input"

vector<uint8_t> Input::_pressedKeys;
vector<uint8_t> Input::_keyDown;
vector<uint8_t> Input::_keyUp;
unordered_map<int, uint8_t> Input::_keymap;
float Input::_screenHalfWidth = 0.f, Input::_screenHalfHeight = 0.f;
float Input::_mouseAxis[2] = { 0.f, 0.f };
float Input::_sensivity[26] = 
{
	0.f, 0.f,
	0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
	0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
	0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
	0.f, 0.f, 0.f, 0.f, 0.f, 0.f
};
unordered_map<std::string, uint8_t> Input::_buttonMap;
unordered_map<std::string, uint8_t> Input::_axisMap;
int Input::_connectedControllers = 0;
ControllerState Input::_controllerState[NE_MAX_CONTROLLERS];
bool Input::_captureMouse = false;

int Input::Initialize(bool captureMouse)
{
	_InitializeKeymap();

	_screenHalfWidth = (float)Engine::GetScreenWidth() / 2.f;
	_screenHalfHeight = (float)Engine::GetScreenHeight() / 2.f;

	_connectedControllers = _GetControllerCount();

	_captureMouse = captureMouse;
	
	Logger::Log(INPUT_MODULE, LOG_INFORMATION, "Initialized");

	return ENGINE_OK;
}

void Input::SetAxisSensivity(uint8_t axis, float sensivity)
{
	if (axis <= 0x07)
	{
		_sensivity[axis] = sensivity;
		return;
	}
	
	uint8_t id = 0;
	while (axis >= 0x10)
	{
		axis -= 0x10;
		id++;
	}
	
	_sensivity[axis + id * 6] = sensivity;
}

bool Input::GetButton(uint8_t key) noexcept
{
	for (uint8_t a : _pressedKeys)
		if (a == key)
			return true;

	return false;
}

bool Input::GetButtonUp(uint8_t key) noexcept
{
	for (uint8_t a : _keyUp)
		if (a == key)
			return true;

	return false;
}

bool Input::GetButtonDown(uint8_t key) noexcept
{
	for (uint8_t a : _keyDown)
		if (a == key)
			return true;

	return false;
}

float Input::GetAxis(uint8_t axis) noexcept
{
	if (axis <= 0x01)
		return _mouseAxis[axis] * _sensivity[axis];

	uint8_t id = 0;
	while (axis >= 0x10)
	{
		axis -= 0x10;
		id++;
	}
	
	int sensivityOffset =  id * 6;

	switch (axis)
	{
		case NE_GPAD0_LX: return _controllerState[id].left_x * _sensivity[2 + sensivityOffset];
		case NE_GPAD0_LY: return _controllerState[id].left_y * _sensivity[3 + sensivityOffset];
		case NE_GPAD0_RX: return _controllerState[id].right_x * _sensivity[4 + sensivityOffset];
		case NE_GPAD0_RY: return _controllerState[id].right_y * _sensivity[5 + sensivityOffset];
		case NE_GPAD0_TL: return _controllerState[id].left_trigger * _sensivity[6 + sensivityOffset];
		case NE_GPAD0_TR: return _controllerState[id].right_trigger * _sensivity[7 + sensivityOffset];
		default: return 0.f;
	}
}

void Input::Key(int key, bool isPressed) noexcept
{
	int code = _keymap[key];

	if (isPressed && !GetButton(code))
	{
		if (Console::IsOpen())
			Console::HandleKeyDown(code);
		else
			_keyDown.push_back(code);
	}
	else if(!isPressed)
		_keyUp.push_back(code);
}

void Input::Update() noexcept
{
	// Controllers
	for (int i = 0; i < _connectedControllers; ++i)
		_GetControllerState(i, &_controllerState[i]);

	// Mouse
	long x = 0, y = 0;
	float xDelta = 0.f, yDelta = 0.f;

	if (_captureMouse && Platform::GetPointerPosition(x, y))
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

void Input::ClearKeyState() noexcept
{
	for (uint8_t code : _keyDown)
		_pressedKeys.push_back(code);

	for (uint8_t code : _keyUp)
		_pressedKeys.erase(remove(_pressedKeys.begin(), _pressedKeys.end(), code), _pressedKeys.end());

	_keyDown.clear();
	_keyUp.clear();
}

void Input::Release()
{
	Logger::Log(INPUT_MODULE, LOG_INFORMATION, "Released");
}

map<uint8_t, char> keycodeToChar =
{
	{ NE_KEY_A, 'a' },
	{ NE_KEY_B, 'b' },
	{ NE_KEY_C, 'c' },
	{ NE_KEY_D, 'd' },
	{ NE_KEY_E, 'e' },
	{ NE_KEY_F, 'f' },
	{ NE_KEY_G, 'g' },
	{ NE_KEY_H, 'h' },
	{ NE_KEY_I, 'i' },
	{ NE_KEY_J, 'j' },
	{ NE_KEY_K, 'k' },
	{ NE_KEY_L, 'l' },
	{ NE_KEY_M, 'm' },
	{ NE_KEY_N, 'n' },
	{ NE_KEY_O, 'o' },
	{ NE_KEY_P, 'p' },
	{ NE_KEY_Q, 'q' },
	{ NE_KEY_R, 'r' },
	{ NE_KEY_S, 's' },
	{ NE_KEY_T, 't' },
	{ NE_KEY_U, 'u' },
	{ NE_KEY_V, 'v' },
	{ NE_KEY_W, 'w' },
	{ NE_KEY_X, 'x' },
	{ NE_KEY_Y, 'y' },
	{ NE_KEY_Z, 'z' },
	{ NE_KEY_0, '0' },
	{ NE_KEY_1, '1' },
	{ NE_KEY_2, '2' },
	{ NE_KEY_3, '3' },
	{ NE_KEY_4, '4' },
	{ NE_KEY_5, '5' },
	{ NE_KEY_6, '6' },
	{ NE_KEY_7, '7' },
	{ NE_KEY_8, '8' },
	{ NE_KEY_9, '9' },
	{ NE_KEY_SPACE, ' ' },
	{ NE_KEY_COMMA, ',' },
	{ NE_KEY_PERIOD, '.' },
	{ NE_KEY_SEMICOLON, ';' },
	{ NE_KEY_QUOTE, '\'' },
	{ NE_KEY_PLUS, '+' },
	{ NE_KEY_MINUS, '-' },
	{ NE_KEY_SLASH, '/' },
	{ NE_KEY_BKSLASH, '\\' },
	{ NE_KEY_LBRACKET, '[' },
	{ NE_KEY_RBRACKET, ']' },
	{ NE_KEY_NUM_MULT, '*' },
	{ NE_KEY_NUM_DIVIDE, '/' },
	{ NE_KEY_NUM_DECIMAL, '.' },
	{ NE_KEY_NUM_MINUS, '-' },
	{ NE_KEY_NUM_PLUS, '+' },
	{ NE_KEY_NUM_0, '0' },
	{ NE_KEY_NUM_1, '1' },
	{ NE_KEY_NUM_2, '2' },
	{ NE_KEY_NUM_3, '3' },
	{ NE_KEY_NUM_4, '4' },
	{ NE_KEY_NUM_5, '5' },
	{ NE_KEY_NUM_6, '6' },
	{ NE_KEY_NUM_7, '7' },
	{ NE_KEY_NUM_8, '8' },
	{ NE_KEY_NUM_9, '9' }
};
