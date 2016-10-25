/* NekoEngine
 *
 * PlatformInput.mm
 * Author: Alexandru Naiman
 *
 * macOS platform input support
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

#import <Carbon/Carbon.h>

using namespace std;

bool Input::SetControllerVibration(int n, float left, float right)
{
	return false;
}

void Input::_InitializeKeymap()
{
	_keymap.insert(make_pair(kVK_ANSI_0, NE_KEY_0));
	_keymap.insert(make_pair(kVK_ANSI_1, NE_KEY_1));
	_keymap.insert(make_pair(kVK_ANSI_2, NE_KEY_2));
	_keymap.insert(make_pair(kVK_ANSI_3, NE_KEY_3));
	_keymap.insert(make_pair(kVK_ANSI_4, NE_KEY_4));
	_keymap.insert(make_pair(kVK_ANSI_5, NE_KEY_5));
	_keymap.insert(make_pair(kVK_ANSI_6, NE_KEY_6));
	_keymap.insert(make_pair(kVK_ANSI_7, NE_KEY_7));
	_keymap.insert(make_pair(kVK_ANSI_8, NE_KEY_8));
	_keymap.insert(make_pair(kVK_ANSI_9, NE_KEY_9));

	_keymap.insert(make_pair(kVK_ANSI_A, NE_KEY_A));
	_keymap.insert(make_pair(kVK_ANSI_B, NE_KEY_B));
	_keymap.insert(make_pair(kVK_ANSI_C, NE_KEY_C));
	_keymap.insert(make_pair(kVK_ANSI_D, NE_KEY_D));
	_keymap.insert(make_pair(kVK_ANSI_E, NE_KEY_E));
	_keymap.insert(make_pair(kVK_ANSI_F, NE_KEY_F));
	_keymap.insert(make_pair(kVK_ANSI_G, NE_KEY_G));
	_keymap.insert(make_pair(kVK_ANSI_H, NE_KEY_H));
	_keymap.insert(make_pair(kVK_ANSI_I, NE_KEY_I));
	_keymap.insert(make_pair(kVK_ANSI_J, NE_KEY_J));
	_keymap.insert(make_pair(kVK_ANSI_K, NE_KEY_K));
	_keymap.insert(make_pair(kVK_ANSI_L, NE_KEY_L));
	_keymap.insert(make_pair(kVK_ANSI_M, NE_KEY_M));
	_keymap.insert(make_pair(kVK_ANSI_N, NE_KEY_N));
	_keymap.insert(make_pair(kVK_ANSI_O, NE_KEY_O));
	_keymap.insert(make_pair(kVK_ANSI_P, NE_KEY_P));
	_keymap.insert(make_pair(kVK_ANSI_Q, NE_KEY_Q));
	_keymap.insert(make_pair(kVK_ANSI_R, NE_KEY_R));
	_keymap.insert(make_pair(kVK_ANSI_S, NE_KEY_S));
	_keymap.insert(make_pair(kVK_ANSI_T, NE_KEY_T));
	_keymap.insert(make_pair(kVK_ANSI_U, NE_KEY_U));
	_keymap.insert(make_pair(kVK_ANSI_V, NE_KEY_V));
	_keymap.insert(make_pair(kVK_ANSI_W, NE_KEY_W));
	_keymap.insert(make_pair(kVK_ANSI_X, NE_KEY_X));
	_keymap.insert(make_pair(kVK_ANSI_Y, NE_KEY_Y));
	_keymap.insert(make_pair(kVK_ANSI_Z, NE_KEY_Z));

	_keymap.insert(make_pair(kVK_UpArrow, NE_KEY_UP));
	_keymap.insert(make_pair(kVK_DownArrow, NE_KEY_DOWN));
	_keymap.insert(make_pair(kVK_LeftArrow, NE_KEY_LEFT));
	_keymap.insert(make_pair(kVK_RightArrow, NE_KEY_RIGHT));
	_keymap.insert(make_pair(kVK_Space, NE_KEY_SPACE));
	_keymap.insert(make_pair(kVK_ANSI_Equal, NE_KEY_PLUS));
	_keymap.insert(make_pair(kVK_ANSI_Minus, NE_KEY_MINUS));
	_keymap.insert(make_pair(kVK_ANSI_Comma, NE_KEY_COMMA));
	_keymap.insert(make_pair(kVK_ANSI_Period, NE_KEY_PERIOD));
	//_keymap.insert(make_pair(XK_Scroll_Lock, NE_KEY_SCROLL));
	_keymap.insert(make_pair(kVK_Shift, NE_KEY_LSHIFT));
	_keymap.insert(make_pair(kVK_RightShift, NE_KEY_RSHIFT));
	_keymap.insert(make_pair(kVK_Option, NE_KEY_LALT));
	_keymap.insert(make_pair(kVK_RightOption, NE_KEY_RALT));
	_keymap.insert(make_pair(kVK_Command, NE_KEY_LSUPER));
	_keymap.insert(make_pair(kVK_Command, NE_KEY_RSUPER));
	_keymap.insert(make_pair(kVK_Control, NE_KEY_LCTRL));
	_keymap.insert(make_pair(kVK_RightControl, NE_KEY_RCTRL));
	_keymap.insert(make_pair(kVK_PageUp, NE_KEY_PGUP));
	_keymap.insert(make_pair(kVK_PageDown, NE_KEY_PGDN));
	_keymap.insert(make_pair(kVK_End, NE_KEY_END));
	_keymap.insert(make_pair(kVK_Home, NE_KEY_HOME));
	_keymap.insert(make_pair(kVK_Escape, NE_KEY_ESCAPE));
	//_keymap.insert(make_pair(XK_Insert, NE_KEY_INSERT));
	_keymap.insert(make_pair(kVK_Return, NE_KEY_RETURN));
	_keymap.insert(make_pair(kVK_CapsLock, NE_KEY_CAPS));
	_keymap.insert(make_pair(kVK_Delete, NE_KEY_DELETE));
	_keymap.insert(make_pair(kVK_ForwardDelete, NE_KEY_BKSPACE));
	_keymap.insert(make_pair(kVK_Tab, NE_KEY_TAB));
	//_keymap.insert(make_pair(XK_Print, NE_KEY_PRTSCRN));
	//_keymap.insert(make_pair(XK_Pause, NE_KEY_PAUSE));
	_keymap.insert(make_pair(kVK_ANSI_Semicolon, NE_KEY_SEMICOLON));
	_keymap.insert(make_pair(kVK_ANSI_Slash, NE_KEY_SLASH));
	_keymap.insert(make_pair(kVK_ANSI_Grave, NE_KEY_TILDE));
	_keymap.insert(make_pair(kVK_ANSI_LeftBracket, NE_KEY_LBRACKET));
	_keymap.insert(make_pair(kVK_ANSI_Backslash, NE_KEY_BKSLASH));
	_keymap.insert(make_pair(kVK_ANSI_RightBracket, NE_KEY_RBRACKET));
	_keymap.insert(make_pair(kVK_ANSI_Quote, NE_KEY_QUOTE));

	_keymap.insert(make_pair(kVK_F1, NE_KEY_F1));
	_keymap.insert(make_pair(kVK_F2, NE_KEY_F2));
	_keymap.insert(make_pair(kVK_F3, NE_KEY_F3));
	_keymap.insert(make_pair(kVK_F4, NE_KEY_F4));
	_keymap.insert(make_pair(kVK_F5, NE_KEY_F5));
	_keymap.insert(make_pair(kVK_F6, NE_KEY_F6));
	_keymap.insert(make_pair(kVK_F7, NE_KEY_F7));
	_keymap.insert(make_pair(kVK_F8, NE_KEY_F8));
	_keymap.insert(make_pair(kVK_F9, NE_KEY_F9));
	_keymap.insert(make_pair(kVK_F10, NE_KEY_F10));
	_keymap.insert(make_pair(kVK_F11, NE_KEY_F11));
	_keymap.insert(make_pair(kVK_F12, NE_KEY_F12));
	_keymap.insert(make_pair(kVK_F13, NE_KEY_F13));
	_keymap.insert(make_pair(kVK_F14, NE_KEY_F14));
	_keymap.insert(make_pair(kVK_F15, NE_KEY_F15));
	_keymap.insert(make_pair(kVK_F16, NE_KEY_F16));
	_keymap.insert(make_pair(kVK_F17, NE_KEY_F17));
	_keymap.insert(make_pair(kVK_F18, NE_KEY_F18));
	_keymap.insert(make_pair(kVK_F19, NE_KEY_F19));
	_keymap.insert(make_pair(kVK_F20, NE_KEY_F20));

	//_keymap.insert(make_pair(XK_Num_Lock, NE_KEY_NUMLOCK));
	_keymap.insert(make_pair(kVK_ANSI_Keypad0, NE_KEY_NUM_0));
	_keymap.insert(make_pair(kVK_ANSI_Keypad1, NE_KEY_NUM_1));
	_keymap.insert(make_pair(kVK_ANSI_Keypad2, NE_KEY_NUM_2));
	_keymap.insert(make_pair(kVK_ANSI_Keypad3, NE_KEY_NUM_3));
	_keymap.insert(make_pair(kVK_ANSI_Keypad4, NE_KEY_NUM_4));
	_keymap.insert(make_pair(kVK_ANSI_Keypad5, NE_KEY_NUM_5));
	_keymap.insert(make_pair(kVK_ANSI_Keypad6, NE_KEY_NUM_6));
	_keymap.insert(make_pair(kVK_ANSI_Keypad7, NE_KEY_NUM_7));
	_keymap.insert(make_pair(kVK_ANSI_Keypad8, NE_KEY_NUM_8));
	_keymap.insert(make_pair(kVK_ANSI_Keypad9, NE_KEY_NUM_9));
	_keymap.insert(make_pair(kVK_ANSI_KeypadPlus, NE_KEY_NUM_PLUS));
	_keymap.insert(make_pair(kVK_ANSI_KeypadMinus, NE_KEY_NUM_MINUS));
	_keymap.insert(make_pair(kVK_ANSI_KeypadDivide, NE_KEY_NUM_DIVIDE));
	_keymap.insert(make_pair(kVK_ANSI_KeypadMultiply, NE_KEY_NUM_MULT));
	_keymap.insert(make_pair(kVK_ANSI_KeypadDecimal, NE_KEY_NUM_DECIMAL));
}

int Input::_GetControllerCount()
{
	return 0;
}

bool Input::_GetControllerState(int n, ControllerState *state)
{
	return false;
}

bool Platform::CapturePointer()
{
    [NSCursor hide];
    return true;
}

void Platform::ReleasePointer()
{
    [NSCursor unhide];
}

bool Platform::GetPointerPosition(long& x, long& y)
{
    NSPoint position = [_activeWindow mouseLocationOutsideOfEventStream];
    
    x = position.x;
    y = _activeWindow.contentView.frame.size.height - position.y - 1;
    
    return true;
}

bool Platform::SetPointerPosition(long x, long y)
{
    NSRect globalPosition = [_activeWindow convertRectToScreen:NSMakeRect(x, _activeWindow.contentView.frame.size.height - y - 1, 0, 0)];
    
    CGAssociateMouseAndMouseCursorPosition(false);
    CGWarpMouseCursorPosition(CGPointMake(globalPosition.origin.x, CGDisplayBounds(CGMainDisplayID()).size.height - globalPosition.origin.y));
    CGAssociateMouseAndMouseCursorPosition(true);
    
    return true;
}

bool Platform::GetTouchMovementDelta(float &x, float &y)
{
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
