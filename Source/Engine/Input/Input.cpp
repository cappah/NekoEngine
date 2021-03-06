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

#include <algorithm>

#include <Input/Input.h>
#include <Engine/Engine.h>
#include <Engine/Console.h>
#include <System/Logger.h>

using namespace std;

#define INPUT_MODULE	"Input"

vector<uint8_t> Input::_pressedKeys;
vector<uint8_t> Input::_keyDown;
vector<uint8_t> Input::_keyUp;
NArray<VirtualAxis> Input::_virtualAxes;
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
bool Input::_enableMouseAxis = false;
bool Input::_pointerCaptured = false;

int Input::Initialize(bool enableMouseAxis)
{
	_InitializeKeymap();

	_screenHalfWidth = (float)Engine::GetConfiguration().Engine.ScreenWidth / 2.f;
	_screenHalfHeight = (float)Engine::GetConfiguration().Engine.ScreenHeight / 2.f;

	_connectedControllers = _GetControllerCount();

	EnableMouseAxis(enableMouseAxis);
	
	Logger::Log(INPUT_MODULE, LOG_INFORMATION, "Initialized");

	return ENGINE_OK;
}

bool Input::EnableMouseAxis(bool enable)
{
	if (enable == _enableMouseAxis) return true;
	else if (!enable)
	{
		_enableMouseAxis = false;
		if (_pointerCaptured) ReleasePointer();
		return true;
	}

	if (!_pointerCaptured)
	{
		if (!CapturePointer())
			return false;
	
		SetPointerPosition(Engine::GetScreenWidth() / 2, Engine::GetScreenHeight() / 2);
	}

	_enableMouseAxis = true;

	return true;
}

void Input::SetAxisSensivity(uint8_t axis, float sensivity)
{
	if (axis >= NE_VIRT_AXIS)
		return;
	else if (axis <= 0x07)
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

	int axisId = axis + id * 6;
	if (axisId > 25)
		return;

	_sensivity[axisId] = sensivity;
}

bool Input::GetButton(uint8_t key) noexcept
{
	for (const uint8_t a : _pressedKeys)
		if (a == key)
			return true;

	return false;
}

bool Input::GetButtonUp(uint8_t key) noexcept
{
	for (const uint8_t a : _keyUp)
		if (a == key)
			return true;

	return false;
}

bool Input::GetButtonDown(uint8_t key) noexcept
{
	for (const uint8_t a : _keyDown)
		if (a == key)
			return true;

	return false;
}

float Input::GetAxis(uint8_t axis) noexcept
{
	if (axis <= 0x01)
		return _mouseAxis[axis] * _sensivity[axis];
	else if (axis >= NE_VIRT_AXIS)
		return _virtualAxes[axis - NE_VIRT_AXIS].val;

	uint8_t id = 0;
	while (axis >= 0x10)
	{
		axis -= 0x10;
		id++;
	}
	
	const int sensivityOffset =  id * 6;

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


float Input::GetAxisSensivity(uint8_t axis) noexcept
{
	if (axis >= NE_VIRT_AXIS)
		return 0.f;
	else if (axis <= 0x07)
		return _sensivity[axis];

	uint8_t id = 0;
	while (axis >= 0x10)
	{
		axis -= 0x10;
		id++;
	}

	int axisId = axis + id * 6;
	if (axisId > 25)
		return 0.f;

	return _sensivity[axisId];
}

void Input::Key(int key, bool isPressed) noexcept
{
	int code = key;
	
	switch (key)
	{
		case NE_MOUSE_LMB:
		case NE_MOUSE_RMB:
		case NE_MOUSE_MMB:
		case NE_MOUSE_BTN4:
		case NE_MOUSE_BTN5:
		{
			if (isPressed && !GetButton(key))
				_keyDown.push_back(key);
			else if (!isPressed)
				_keyUp.push_back(key);
			return;
		}
		break;
	}

	code = _keymap[key];

	if (isPressed && !GetButton(code))
	{
		if (Console::IsOpen())
			Console::HandleKeyDown(code);
		else if ((code == NE_KEY_TILDE) && Engine::GetConfiguration().Engine.EnableConsole)
			Console::OpenConsole();
		else
		{
			_keyDown.push_back(code);
			for (VirtualAxis &v : _virtualAxes)
			{
				if (v.min == code)
				{
					v.val = -.5f;
					v.active = true;
				}
				else if (v.max == code)
				{
					v.val = .5f;
					v.active = true;
				}
			}
		}
	}
	else if (!isPressed)
	{
		if (Console::IsOpen())
			Console::HandleKeyUp(code);
		else
		{
			_keyUp.push_back(code);
			for (VirtualAxis &v : _virtualAxes)
			{
				if (v.min == code)
				{
					if (GetButton(v.max))
						v.val = 1.f;
					else
					{
						v.val = -.5f;
						v.active = false;
					}
				}
				else if (v.max == code)
				{
					if (GetButton(v.min))
						v.val = -1.f;
					else
					{
						v.val = .5f;
						v.active = false;
					}
				}
			}
		}
	}
}

void Input::Update() noexcept
{
	// Controllers
	for (int i = 0; i < _connectedControllers; ++i)
		_GetControllerState(i, &_controllerState[i]);

	// Mouse
	long x = 0, y = 0;
	float xDelta = 0.f, yDelta = 0.f;

	if (_enableMouseAxis && _pointerCaptured && GetPointerPosition(x, y))
	{
		xDelta = _screenHalfWidth - x;
		yDelta = _screenHalfHeight - y;

		SetPointerPosition((long)_screenHalfWidth, (long)_screenHalfHeight);
	}

	_mouseAxis[NE_MOUSE_X] = -(xDelta / _screenHalfWidth);
	_mouseAxis[NE_MOUSE_Y] = yDelta / _screenHalfHeight;
}

void Input::ClearKeyState() noexcept
{
	for (uint8_t code : _keyDown)
		_pressedKeys.push_back(code);

	for (uint8_t code : _keyUp)
		_pressedKeys.erase(remove(_pressedKeys.begin(), _pressedKeys.end(), code), _pressedKeys.end());

	for (VirtualAxis &v : _virtualAxes)
	{
		if (v.active && v.val > 0.f) v.val = 1.f;
		else if (v.active) v.val = -1.f;
		else v.val = 0.f;
	}

	_keyDown.clear();
	_keyUp.clear();
}

char Input::KeycodeToChar(uint8_t keyCode, bool shift) noexcept
{
	uint8_t ch = keycodeToChar[keyCode];

	if (shift)
	{
		if (ch > 0x60 && ch < 0x7B) ch -= 0x20;
		else if (ch > 0x5A && ch < 0x5E) ch += 0x20;
		else if (shiftedChars.find(ch) != shiftedChars.end())
			ch = shiftedChars[ch];
	}

	return ch;
}

void Input::Release()
{
	ReleasePointer();
	Logger::Log(INPUT_MODULE, LOG_INFORMATION, "Released");
}

map<uint8_t, char> keycodeToChar
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

map<char, char> shiftedChars
{
	{ 0x2C, 0x3C },
	{ 0x2D, 0x5F },
	{ 0x2E, 0x3E },
	{ 0x2F, 0x3F },
	{ 0x30, 0x29 },
	{ 0x31, 0x21 },
	{ 0x32, 0x40 },
	{ 0x33, 0x23 },
	{ 0x34, 0x24 },
	{ 0x35, 0x25 },
	{ 0x36, 0x5E },
	{ 0x37, 0x26 },
	{ 0x38, 0x2A },
	{ 0x39, 0x28 },
	{ 0x2B, 0x3D },
	{ '\'', '"' }
};
