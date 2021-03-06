/* NekoEngine
 *
 * PlatformInput.cpp
 * Author: Alexandru Naiman
 *
 * Windows platform input support
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
#include <Platform/Platform.h>

#include <Xinput.h>

using namespace std;

extern UINT NWM_SHOWCURSOR;
extern UINT NWM_HIDECURSOR;

DWORD _xiLastPacket[4] = { 0, 0, 0, 0 };

static inline void _xiDeadzone(float &x, float &y, float max, float deadzone)
{
	float magnitude = sqrt(x*x + y*y);

	float normX = x / magnitude;
	float normY = y / magnitude;

	float normMagnitude = 0.f;

	if (magnitude > deadzone)
	{
		if (magnitude > max) magnitude = max;

		magnitude -= deadzone;

		normMagnitude = magnitude / (max - deadzone);
	}
	else
		normMagnitude = 0.f;

	x = normX * normMagnitude;
	y = normY * normMagnitude;
}

bool Input::SetControllerVibration(int n, float left, float right)
{
	XINPUT_VIBRATION vibration{};

	left = left > 1.f ? 1.f : left;
	right = right > 1.f ? 1.f : right;

	vibration.wLeftMotorSpeed = (WORD)(left * 65535);
	vibration.wRightMotorSpeed = (WORD)(right * 65535);

	return XInputSetState(n, &vibration) == ERROR_SUCCESS;
}

void Input::_InitializeKeymap()
{
	// Windows virtual keycode list:
	// https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx

	_keymap.insert(make_pair(0x30, NE_KEY_0));
	_keymap.insert(make_pair(0x31, NE_KEY_1));
	_keymap.insert(make_pair(0x32, NE_KEY_2));
	_keymap.insert(make_pair(0x33, NE_KEY_3));
	_keymap.insert(make_pair(0x34, NE_KEY_4));
	_keymap.insert(make_pair(0x35, NE_KEY_5));
	_keymap.insert(make_pair(0x36, NE_KEY_6));
	_keymap.insert(make_pair(0x37, NE_KEY_7));
	_keymap.insert(make_pair(0x38, NE_KEY_8));
	_keymap.insert(make_pair(0x39, NE_KEY_9));

	_keymap.insert(make_pair(0x41, NE_KEY_A));
	_keymap.insert(make_pair(0x42, NE_KEY_B));
	_keymap.insert(make_pair(0x43, NE_KEY_C));
	_keymap.insert(make_pair(0x44, NE_KEY_D));
	_keymap.insert(make_pair(0x45, NE_KEY_E));
	_keymap.insert(make_pair(0x46, NE_KEY_F));
	_keymap.insert(make_pair(0x47, NE_KEY_G));
	_keymap.insert(make_pair(0x48, NE_KEY_H));
	_keymap.insert(make_pair(0x49, NE_KEY_I));
	_keymap.insert(make_pair(0x4A, NE_KEY_J));
	_keymap.insert(make_pair(0x4B, NE_KEY_K));
	_keymap.insert(make_pair(0x4C, NE_KEY_L));
	_keymap.insert(make_pair(0x4D, NE_KEY_M));
	_keymap.insert(make_pair(0x4E, NE_KEY_N));
	_keymap.insert(make_pair(0x4F, NE_KEY_O));
	_keymap.insert(make_pair(0x50, NE_KEY_P));
	_keymap.insert(make_pair(0x51, NE_KEY_Q));
	_keymap.insert(make_pair(0x52, NE_KEY_R));
	_keymap.insert(make_pair(0x53, NE_KEY_S));
	_keymap.insert(make_pair(0x54, NE_KEY_T));
	_keymap.insert(make_pair(0x55, NE_KEY_U));
	_keymap.insert(make_pair(0x56, NE_KEY_V));
	_keymap.insert(make_pair(0x57, NE_KEY_W));
	_keymap.insert(make_pair(0x58, NE_KEY_X));
	_keymap.insert(make_pair(0x59, NE_KEY_Y));
	_keymap.insert(make_pair(0x5A, NE_KEY_Z));

	_keymap.insert(make_pair(VK_UP, NE_KEY_UP));
	_keymap.insert(make_pair(VK_DOWN, NE_KEY_DOWN));
	_keymap.insert(make_pair(VK_LEFT, NE_KEY_LEFT));
	_keymap.insert(make_pair(VK_RIGHT, NE_KEY_RIGHT));
	_keymap.insert(make_pair(VK_SPACE, NE_KEY_SPACE));
	_keymap.insert(make_pair(VK_OEM_PLUS, NE_KEY_PLUS));
	_keymap.insert(make_pair(VK_OEM_MINUS, NE_KEY_MINUS));
	_keymap.insert(make_pair(VK_OEM_COMMA, NE_KEY_COMMA));
	_keymap.insert(make_pair(VK_OEM_PERIOD, NE_KEY_PERIOD));
	_keymap.insert(make_pair(VK_SCROLL, NE_KEY_SCROLL));
	_keymap.insert(make_pair(VK_LSHIFT, NE_KEY_LSHIFT));
	_keymap.insert(make_pair(VK_RSHIFT, NE_KEY_RSHIFT));
	_keymap.insert(make_pair(VK_LMENU, NE_KEY_LALT));
	_keymap.insert(make_pair(VK_RMENU, NE_KEY_RALT));
	_keymap.insert(make_pair(VK_LWIN, NE_KEY_LSUPER));
	_keymap.insert(make_pair(VK_RWIN, NE_KEY_RSUPER));
	_keymap.insert(make_pair(VK_LCONTROL, NE_KEY_LCTRL));
	_keymap.insert(make_pair(VK_RCONTROL, NE_KEY_RCTRL));
	_keymap.insert(make_pair(VK_PRIOR, NE_KEY_PGUP));
	_keymap.insert(make_pair(VK_NEXT, NE_KEY_PGDN));
	_keymap.insert(make_pair(VK_END, NE_KEY_END));
	_keymap.insert(make_pair(VK_HOME, NE_KEY_HOME));
	_keymap.insert(make_pair(VK_ESCAPE, NE_KEY_ESCAPE));
	_keymap.insert(make_pair(VK_INSERT, NE_KEY_INSERT));
	_keymap.insert(make_pair(VK_RETURN, NE_KEY_RETURN));
	_keymap.insert(make_pair(VK_CAPITAL, NE_KEY_CAPS));
	_keymap.insert(make_pair(VK_DELETE, NE_KEY_DELETE));
	_keymap.insert(make_pair(VK_BACK, NE_KEY_BKSPACE));
	_keymap.insert(make_pair(VK_TAB, NE_KEY_TAB));
	_keymap.insert(make_pair(VK_SNAPSHOT, NE_KEY_PRTSCRN));
	_keymap.insert(make_pair(VK_PAUSE, NE_KEY_PAUSE));

	/* US standard keyboards only. Need a better solution */
	_keymap.insert(make_pair(VK_OEM_1, NE_KEY_SEMICOLON));
	_keymap.insert(make_pair(VK_OEM_2, NE_KEY_SLASH));
	_keymap.insert(make_pair(VK_OEM_3, NE_KEY_TILDE));
	_keymap.insert(make_pair(VK_OEM_4, NE_KEY_LBRACKET));
	_keymap.insert(make_pair(VK_OEM_5, NE_KEY_BKSLASH));
	_keymap.insert(make_pair(VK_OEM_6, NE_KEY_RBRACKET));
	_keymap.insert(make_pair(VK_OEM_7, NE_KEY_QUOTE));
	/* End */

	_keymap.insert(make_pair(VK_F1, NE_KEY_F1));
	_keymap.insert(make_pair(VK_F2, NE_KEY_F2));
	_keymap.insert(make_pair(VK_F3, NE_KEY_F3));
	_keymap.insert(make_pair(VK_F4, NE_KEY_F4));
	_keymap.insert(make_pair(VK_F5, NE_KEY_F5));
	_keymap.insert(make_pair(VK_F6, NE_KEY_F6));
	_keymap.insert(make_pair(VK_F7, NE_KEY_F7));
	_keymap.insert(make_pair(VK_F8, NE_KEY_F8));
	_keymap.insert(make_pair(VK_F9, NE_KEY_F9));
	_keymap.insert(make_pair(VK_F10, NE_KEY_F10));
	_keymap.insert(make_pair(VK_F11, NE_KEY_F11));
	_keymap.insert(make_pair(VK_F12, NE_KEY_F12));
	_keymap.insert(make_pair(VK_F13, NE_KEY_F13));
	_keymap.insert(make_pair(VK_F14, NE_KEY_F14));
	_keymap.insert(make_pair(VK_F15, NE_KEY_F15));
	_keymap.insert(make_pair(VK_F16, NE_KEY_F16));
	_keymap.insert(make_pair(VK_F17, NE_KEY_F17));
	_keymap.insert(make_pair(VK_F18, NE_KEY_F18));
	_keymap.insert(make_pair(VK_F19, NE_KEY_F19));
	_keymap.insert(make_pair(VK_F20, NE_KEY_F20));
	_keymap.insert(make_pair(VK_F21, NE_KEY_F21));
	_keymap.insert(make_pair(VK_F22, NE_KEY_F22));
	_keymap.insert(make_pair(VK_F23, NE_KEY_F23));
	_keymap.insert(make_pair(VK_F24, NE_KEY_F24));

	_keymap.insert(make_pair(VK_NUMLOCK, NE_KEY_NUMLOCK));
	_keymap.insert(make_pair(VK_NUMPAD0, NE_KEY_NUM_0));
	_keymap.insert(make_pair(VK_NUMPAD1, NE_KEY_NUM_1));
	_keymap.insert(make_pair(VK_NUMPAD2, NE_KEY_NUM_2));
	_keymap.insert(make_pair(VK_NUMPAD3, NE_KEY_NUM_3));
	_keymap.insert(make_pair(VK_NUMPAD4, NE_KEY_NUM_4));
	_keymap.insert(make_pair(VK_NUMPAD5, NE_KEY_NUM_5));
	_keymap.insert(make_pair(VK_NUMPAD6, NE_KEY_NUM_6));
	_keymap.insert(make_pair(VK_NUMPAD7, NE_KEY_NUM_7));
	_keymap.insert(make_pair(VK_NUMPAD8, NE_KEY_NUM_8));
	_keymap.insert(make_pair(VK_NUMPAD9, NE_KEY_NUM_9));
	_keymap.insert(make_pair(VK_ADD, NE_KEY_NUM_PLUS));
	_keymap.insert(make_pair(VK_SUBTRACT, NE_KEY_NUM_MINUS));
	_keymap.insert(make_pair(VK_DIVIDE, NE_KEY_NUM_DIVIDE));
	_keymap.insert(make_pair(VK_MULTIPLY, NE_KEY_NUM_MULT));
	_keymap.insert(make_pair(VK_DECIMAL, NE_KEY_NUM_DECIMAL));
}

int Input::_GetControllerCount()
{
	XINPUT_STATE xistate{};
	int num = 0;

	for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i)
		if (XInputGetState(i, &xistate) == ERROR_SUCCESS)
			num++;

	return num;
}

bool Input::_GetControllerState(int n, ControllerState *state)
{
	XINPUT_STATE xistate;

	if (XInputGetState(n, &xistate) != ERROR_SUCCESS)
		return false;

	// Controller state has not changed
	if (_xiLastPacket[n] == xistate.dwPacketNumber)
		return true;

	if ((xistate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) == XINPUT_GAMEPAD_DPAD_UP)
		_pressedKeys.push_back(NE_GPAD_D_UP);
	else
		_pressedKeys.erase(remove(_pressedKeys.begin(), _pressedKeys.end(), NE_GPAD_D_UP), _pressedKeys.end());
	
	if ((xistate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) == XINPUT_GAMEPAD_DPAD_DOWN)
		_pressedKeys.push_back(NE_GPAD_D_DOWN);
	else
		_pressedKeys.erase(remove(_pressedKeys.begin(), _pressedKeys.end(), NE_GPAD_D_DOWN), _pressedKeys.end());

	if ((xistate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) == XINPUT_GAMEPAD_DPAD_LEFT)
		_pressedKeys.push_back(NE_GPAD_D_LEFT);
	else
		_pressedKeys.erase(remove(_pressedKeys.begin(), _pressedKeys.end(), NE_GPAD_D_LEFT), _pressedKeys.end());

	if ((xistate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) == XINPUT_GAMEPAD_DPAD_RIGHT)
		_pressedKeys.push_back(NE_GPAD_D_RIGHT);
	else
		_pressedKeys.erase(remove(_pressedKeys.begin(), _pressedKeys.end(), NE_GPAD_D_RIGHT), _pressedKeys.end());

	if ((xistate.Gamepad.wButtons & XINPUT_GAMEPAD_START) == XINPUT_GAMEPAD_START)
		_pressedKeys.push_back(NE_GPAD_START);
	else
		_pressedKeys.erase(remove(_pressedKeys.begin(), _pressedKeys.end(), NE_GPAD_START), _pressedKeys.end());

	if ((xistate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) == XINPUT_GAMEPAD_BACK)
		_pressedKeys.push_back(NE_GPAD_BACK);
	else
		_pressedKeys.erase(remove(_pressedKeys.begin(), _pressedKeys.end(), NE_GPAD_BACK), _pressedKeys.end());

	if ((xistate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) == XINPUT_GAMEPAD_LEFT_THUMB)
		_pressedKeys.push_back(NE_GPAD_LTHUMB);
	else
		_pressedKeys.erase(remove(_pressedKeys.begin(), _pressedKeys.end(), NE_GPAD_LTHUMB), _pressedKeys.end());

	if ((xistate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) == XINPUT_GAMEPAD_RIGHT_THUMB)
		_pressedKeys.push_back(NE_GPAD_RTHUMB);
	else
		_pressedKeys.erase(remove(_pressedKeys.begin(), _pressedKeys.end(), NE_GPAD_RTHUMB), _pressedKeys.end());

	if ((xistate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) == XINPUT_GAMEPAD_LEFT_SHOULDER)
		_pressedKeys.push_back(NE_GPAD_LSHOULDER);
	else
		_pressedKeys.erase(remove(_pressedKeys.begin(), _pressedKeys.end(), NE_GPAD_LSHOULDER), _pressedKeys.end());

	if ((xistate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) == XINPUT_GAMEPAD_RIGHT_SHOULDER)
		_pressedKeys.push_back(NE_GPAD_RSHOULDER);
	else
		_pressedKeys.erase(remove(_pressedKeys.begin(), _pressedKeys.end(), NE_GPAD_RSHOULDER), _pressedKeys.end());

	if ((xistate.Gamepad.wButtons & XINPUT_GAMEPAD_A) == XINPUT_GAMEPAD_A)
		_pressedKeys.push_back(NE_GPAD_A);
	else
		_pressedKeys.erase(remove(_pressedKeys.begin(), _pressedKeys.end(), NE_GPAD_A), _pressedKeys.end());

	if ((xistate.Gamepad.wButtons & XINPUT_GAMEPAD_B) == XINPUT_GAMEPAD_B)
		_pressedKeys.push_back(NE_GPAD_B);
	else
		_pressedKeys.erase(remove(_pressedKeys.begin(), _pressedKeys.end(), NE_GPAD_B), _pressedKeys.end());

	if ((xistate.Gamepad.wButtons & XINPUT_GAMEPAD_X) == XINPUT_GAMEPAD_X)
		_pressedKeys.push_back(NE_GPAD_X);
	else
		_pressedKeys.erase(remove(_pressedKeys.begin(), _pressedKeys.end(), NE_GPAD_X), _pressedKeys.end());

	if ((xistate.Gamepad.wButtons & XINPUT_GAMEPAD_Y) == XINPUT_GAMEPAD_Y)
		_pressedKeys.push_back(NE_GPAD_Y);
	else
		_pressedKeys.erase(remove(_pressedKeys.begin(), _pressedKeys.end(), NE_GPAD_Y), _pressedKeys.end());

	state->left_x = (float)xistate.Gamepad.sThumbLX;
	state->left_y = (float)xistate.Gamepad.sThumbLY;
	state->left_trigger = (float)xistate.Gamepad.bLeftTrigger;
	state->right_x = (float)xistate.Gamepad.sThumbRX;
	state->right_y = (float)xistate.Gamepad.sThumbRY;
	state->right_trigger = (float)xistate.Gamepad.bRightTrigger;

	_xiDeadzone(state->left_x, state->left_y, 32767.f, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	_xiDeadzone(state->right_x, state->right_y, 32767.f, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
	_xiDeadzone(state->left_trigger, state->right_trigger, 255.f, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

/*	_deadzone(state->left_x, state->left_y, .1f);
	_deadzone(state->right_x, state->right_y, .1f);
	_deadzone(state->left_trigger, state->right_trigger, .1f);*/

	_xiLastPacket[n] = xistate.dwPacketNumber;

	return true;
}

bool Input::CapturePointer()
{
	PostMessage(Platform::GetActiveWindow(), NWM_HIDECURSOR, (WPARAM)0, (LPARAM)0);
	SetCapture(Platform::GetActiveWindow());

	_pointerCaptured = true;

	return true;
}

void Input::ReleasePointer()
{
	ReleaseCapture();
	PostMessage(Platform::GetActiveWindow(), NWM_SHOWCURSOR, (WPARAM)0, (LPARAM)0);

	_pointerCaptured = false;
}

bool Input::GetPointerPosition(long &x, long &y)
{
	POINT pt{};
	bool ret = false;

	ret = GetCursorPos(&pt) == TRUE ? true : false;
	ret &= ScreenToClient(Platform::GetActiveWindow(), &pt) == TRUE ? true : false;

	if (ret)
	{
		x = pt.x;
		y = pt.y;
	}

	return ret;
}

bool Input::SetPointerPosition(long x, long y)
{
	POINT pt{};
	bool ret = false;

	pt.x = x;
	pt.y = y;

	ret = ClientToScreen(Platform::GetActiveWindow(), &pt) == TRUE ? true : false;
	ret &= SetCursorPos(pt.x, pt.y) == TRUE ? true : false;

	return ret;
}