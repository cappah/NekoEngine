/* NekoEngine
 *
 * PlatformInput.cpp
 * Author: Alexandru Naiman
 *
 * Android platform input support
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

#include <Engine/Engine.h>
#include <Engine/Input.h>
#include <Platform/Platform.h>

#include <android/sensor.h>
#include <android/keycodes.h>

#include <glm/glm.hpp>

using namespace std;
using namespace glm;

bool _dragging{ false };
float _lastX{ 0.f }, _lastY{ 0.f };
float _xDelta{ 0.f }, _yDelta{ 0.f };
float _mouseX( 0.f ), _mouseY{ 0.f };
vec3 _acceleration;
vec3 _gyroscope;
float _lightValue{ 0.f };

int32_t _android_handle_input_event(struct android_app* app, AInputEvent* event)
{
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
	{
		int32_t action = AMotionEvent_getAction(event);
		int32_t source = AInputEvent_getSource(event);

		if (source == AINPUT_SOURCE_TOUCHSCREEN) {
			if (action == AMOTION_EVENT_ACTION_DOWN) {
				_dragging = true;

				_lastX = AMotionEvent_getX(event, 0);
				_lastY = AMotionEvent_getY(event, 0);
			}
			else if (action == AMOTION_EVENT_ACTION_MOVE) {
				if (!_dragging)
					return 0;

				_xDelta = _lastX - AMotionEvent_getX(event, 0);
				_yDelta = _lastY - AMotionEvent_getY(event, 0);
			}
			else if (action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_CANCEL)
				_dragging = false;
		}
		else if (source == AINPUT_SOURCE_MOUSE)
		{
			_mouseX = AMotionEvent_getX(event, 0);
			_mouseY = AMotionEvent_getY(event, 0);
		}
	}
	else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
		Input::Key(AKeyEvent_getKeyCode(event), AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN ? true : false);

	return 0;
}

void _android_handle_sensor_event(ASensorEventQueue *queue)
{
	ASensorEvent event;
	while (ASensorEventQueue_getEvents(queue, &event, 1))
	{
		switch (event.type)
		{
			case ASENSOR_TYPE_ACCELEROMETER: _acceleration = { event.acceleration.x, event.acceleration.y, event.acceleration.z }; break;
			case ASENSOR_TYPE_GYROSCOPE: _gyroscope = { event.vector.x, event.vector.y, event.vector.z }; break;
			case ASENSOR_TYPE_LIGHT: _lightValue = event.light; break;
		}
	}
}

bool Input::SetControllerVibration(int n, float left, float right)
{
	(void)n;
	(void)left;
	(void)right;

	return false;
}

void Input::_InitializeKeymap()
{
	_keymap.insert(make_pair(AKEYCODE_0, NE_KEY_0));
	_keymap.insert(make_pair(AKEYCODE_1, NE_KEY_1));
	_keymap.insert(make_pair(AKEYCODE_2, NE_KEY_2));
	_keymap.insert(make_pair(AKEYCODE_3, NE_KEY_3));
	_keymap.insert(make_pair(AKEYCODE_4, NE_KEY_4));
	_keymap.insert(make_pair(AKEYCODE_5, NE_KEY_5));
	_keymap.insert(make_pair(AKEYCODE_6, NE_KEY_6));
	_keymap.insert(make_pair(AKEYCODE_7, NE_KEY_7));
	_keymap.insert(make_pair(AKEYCODE_8, NE_KEY_8));
	_keymap.insert(make_pair(AKEYCODE_9, NE_KEY_9));

	_keymap.insert(make_pair(AKEYCODE_A, NE_KEY_A));
	_keymap.insert(make_pair(AKEYCODE_B, NE_KEY_B));
	_keymap.insert(make_pair(AKEYCODE_C, NE_KEY_C));
	_keymap.insert(make_pair(AKEYCODE_D, NE_KEY_D));
	_keymap.insert(make_pair(AKEYCODE_E, NE_KEY_E));
	_keymap.insert(make_pair(AKEYCODE_F, NE_KEY_F));
	_keymap.insert(make_pair(AKEYCODE_G, NE_KEY_G));
	_keymap.insert(make_pair(AKEYCODE_H, NE_KEY_H));
	_keymap.insert(make_pair(AKEYCODE_I, NE_KEY_I));
	_keymap.insert(make_pair(AKEYCODE_J, NE_KEY_J));
	_keymap.insert(make_pair(AKEYCODE_K, NE_KEY_K));
	_keymap.insert(make_pair(AKEYCODE_L, NE_KEY_L));
	_keymap.insert(make_pair(AKEYCODE_M, NE_KEY_M));
	_keymap.insert(make_pair(AKEYCODE_N, NE_KEY_N));
	_keymap.insert(make_pair(AKEYCODE_O, NE_KEY_O));
	_keymap.insert(make_pair(AKEYCODE_P, NE_KEY_P));
	_keymap.insert(make_pair(AKEYCODE_Q, NE_KEY_Q));
	_keymap.insert(make_pair(AKEYCODE_R, NE_KEY_R));
	_keymap.insert(make_pair(AKEYCODE_S, NE_KEY_S));
	_keymap.insert(make_pair(AKEYCODE_T, NE_KEY_T));
	_keymap.insert(make_pair(AKEYCODE_U, NE_KEY_U));
	_keymap.insert(make_pair(AKEYCODE_V, NE_KEY_V));
	_keymap.insert(make_pair(AKEYCODE_W, NE_KEY_W));
	_keymap.insert(make_pair(AKEYCODE_X, NE_KEY_X));
	_keymap.insert(make_pair(AKEYCODE_Y, NE_KEY_Y));
	_keymap.insert(make_pair(AKEYCODE_Z, NE_KEY_Z));

	_keymap.insert(make_pair(AKEYCODE_DPAD_UP, NE_KEY_UP));
	_keymap.insert(make_pair(AKEYCODE_DPAD_DOWN, NE_KEY_DOWN));
	_keymap.insert(make_pair(AKEYCODE_DPAD_LEFT, NE_KEY_LEFT));
	_keymap.insert(make_pair(AKEYCODE_DPAD_RIGHT, NE_KEY_RIGHT));
	_keymap.insert(make_pair(AKEYCODE_SPACE, NE_KEY_SPACE));
	_keymap.insert(make_pair(AKEYCODE_PLUS, NE_KEY_PLUS));
	_keymap.insert(make_pair(AKEYCODE_MINUS, NE_KEY_MINUS));
	_keymap.insert(make_pair(AKEYCODE_COMMA, NE_KEY_COMMA));
	_keymap.insert(make_pair(AKEYCODE_PERIOD, NE_KEY_PERIOD));
	_keymap.insert(make_pair(AKEYCODE_SCROLL_LOCK, NE_KEY_SCROLL));
	_keymap.insert(make_pair(AKEYCODE_SHIFT_LEFT, NE_KEY_LSHIFT));
	_keymap.insert(make_pair(AKEYCODE_SHIFT_RIGHT, NE_KEY_RSHIFT));
	_keymap.insert(make_pair(AKEYCODE_ALT_LEFT, NE_KEY_LALT));
	_keymap.insert(make_pair(AKEYCODE_ALT_RIGHT, NE_KEY_RALT));
	_keymap.insert(make_pair(AKEYCODE_META_LEFT, NE_KEY_LSUPER));
	_keymap.insert(make_pair(AKEYCODE_META_RIGHT, NE_KEY_RSUPER));
	_keymap.insert(make_pair(AKEYCODE_CTRL_LEFT, NE_KEY_LCTRL));
	_keymap.insert(make_pair(AKEYCODE_CTRL_RIGHT, NE_KEY_RCTRL));
	_keymap.insert(make_pair(AKEYCODE_PAGE_UP, NE_KEY_PGUP));
	_keymap.insert(make_pair(AKEYCODE_PAGE_DOWN, NE_KEY_PGDN));
	_keymap.insert(make_pair(AKEYCODE_MOVE_END, NE_KEY_END));
	_keymap.insert(make_pair(AKEYCODE_MOVE_HOME, NE_KEY_HOME));
	_keymap.insert(make_pair(AKEYCODE_ESCAPE, NE_KEY_ESCAPE));
	_keymap.insert(make_pair(AKEYCODE_INSERT, NE_KEY_INSERT));
	_keymap.insert(make_pair(AKEYCODE_ENTER, NE_KEY_RETURN));
	_keymap.insert(make_pair(AKEYCODE_CAPS_LOCK, NE_KEY_CAPS));
	_keymap.insert(make_pair(AKEYCODE_DEL, NE_KEY_DELETE));
	_keymap.insert(make_pair(AKEYCODE_BACK, NE_KEY_BKSPACE));
	_keymap.insert(make_pair(AKEYCODE_TAB, NE_KEY_TAB));
	//_keymap.insert(make_pair(AKEYCODE_SC, NE_KEY_PRTSCRN));
	_keymap.insert(make_pair(AKEYCODE_MEDIA_PAUSE, NE_KEY_PAUSE));

	_keymap.insert(make_pair(AKEYCODE_SEMICOLON, NE_KEY_SEMICOLON));
	_keymap.insert(make_pair(AKEYCODE_SLASH, NE_KEY_SLASH));
	//_keymap.insert(make_pair(AKEYCODE_, NE_KEY_TILDE));
	_keymap.insert(make_pair(AKEYCODE_LEFT_BRACKET, NE_KEY_LBRACKET));
	_keymap.insert(make_pair(AKEYCODE_BACKSLASH, NE_KEY_BKSLASH));
	_keymap.insert(make_pair(AKEYCODE_RIGHT_BRACKET, NE_KEY_RBRACKET));
	_keymap.insert(make_pair(AKEYCODE_APOSTROPHE, NE_KEY_QUOTE));

	_keymap.insert(make_pair(AKEYCODE_F1, NE_KEY_F1));
	_keymap.insert(make_pair(AKEYCODE_F2, NE_KEY_F2));
	_keymap.insert(make_pair(AKEYCODE_F3, NE_KEY_F3));
	_keymap.insert(make_pair(AKEYCODE_F4, NE_KEY_F4));
	_keymap.insert(make_pair(AKEYCODE_F5, NE_KEY_F5));
	_keymap.insert(make_pair(AKEYCODE_F6, NE_KEY_F6));
	_keymap.insert(make_pair(AKEYCODE_F7, NE_KEY_F7));
	_keymap.insert(make_pair(AKEYCODE_F8, NE_KEY_F8));
	_keymap.insert(make_pair(AKEYCODE_F9, NE_KEY_F9));
	_keymap.insert(make_pair(AKEYCODE_F10, NE_KEY_F10));
	_keymap.insert(make_pair(AKEYCODE_F11, NE_KEY_F11));
	_keymap.insert(make_pair(AKEYCODE_F12, NE_KEY_F12));

	_keymap.insert(make_pair(AKEYCODE_NUM_LOCK, NE_KEY_NUMLOCK));
	_keymap.insert(make_pair(AKEYCODE_NUMPAD_0, NE_KEY_NUM_0));
	_keymap.insert(make_pair(AKEYCODE_NUMPAD_1, NE_KEY_NUM_1));
	_keymap.insert(make_pair(AKEYCODE_NUMPAD_2, NE_KEY_NUM_2));
	_keymap.insert(make_pair(AKEYCODE_NUMPAD_3, NE_KEY_NUM_3));
	_keymap.insert(make_pair(AKEYCODE_NUMPAD_4, NE_KEY_NUM_4));
	_keymap.insert(make_pair(AKEYCODE_NUMPAD_5, NE_KEY_NUM_5));
	_keymap.insert(make_pair(AKEYCODE_NUMPAD_6, NE_KEY_NUM_6));
	_keymap.insert(make_pair(AKEYCODE_NUMPAD_7, NE_KEY_NUM_7));
	_keymap.insert(make_pair(AKEYCODE_NUMPAD_8, NE_KEY_NUM_8));
	_keymap.insert(make_pair(AKEYCODE_NUMPAD_9, NE_KEY_NUM_9));
	_keymap.insert(make_pair(AKEYCODE_NUMPAD_ADD, NE_KEY_NUM_PLUS));
	_keymap.insert(make_pair(AKEYCODE_NUMPAD_SUBTRACT, NE_KEY_NUM_MINUS));
	_keymap.insert(make_pair(AKEYCODE_NUMPAD_DIVIDE, NE_KEY_NUM_DIVIDE));
	_keymap.insert(make_pair(AKEYCODE_NUMPAD_MULTIPLY, NE_KEY_NUM_MULT));
	_keymap.insert(make_pair(AKEYCODE_NUMPAD_DOT, NE_KEY_NUM_DECIMAL));
}

int Input::_GetControllerCount()
{
	return 0;
}

bool Input::_GetControllerState(int n, ControllerState *state)
{
	(void)n;
	(void)state;

	return false;
}

bool Platform::CapturePointer()
{
	return false;
}

void Platform::ReleasePointer()
{
}

bool Platform::GetPointerPosition(long& x, long& y)
{
	x = _mouseX;
	y = _mouseY;

	_mouseX = 0.f;
	_mouseY = 0.f;

	return false;
}

bool Platform::SetPointerPosition(long x, long y)
{
	(void)x;
	(void)y;

	return false;
}

bool Platform::GetTouchMovementDelta(float &x, float &y)
{
	x = _xDelta;
	y = _yDelta;

	_xDelta = 0.f;
	_yDelta = 0.f;

	return false;
}

bool Input::GetAccelerometerAxis(glm::vec3 &axis)
{
	(void)axis;
	return false;
}

bool Input::GetGyroscopeAxis(glm::vec3 &axis)
{
	(void)axis;
	return false;
}

bool Input::GetLightIntensity(float &intensity)
{
	(void)intensity;
	return false;
}
