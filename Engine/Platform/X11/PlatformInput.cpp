/* Neko Engine
 *
 * PlatformInput.cpp
 * Author: Alexandru Naiman
 *
 * X11 platform input support
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
#define PLATFORM_INTERNAL

#include <Engine/Engine.h>
#include <Engine/Input.h>
#include <Platform/Platform.h>

#include <X11/keysym.h>

using namespace std;

bool Input::SetControllerVibration(int n, float left, float right)
{
	return false;
}

void Input::_InitializeKeymap()
{
	setlocale(LC_CTYPE, "");

	_keymap.insert(make_pair(XK_0, NE_KEY_0));
	_keymap.insert(make_pair(XK_1, NE_KEY_1));
	_keymap.insert(make_pair(XK_2, NE_KEY_2));
	_keymap.insert(make_pair(XK_3, NE_KEY_3));
	_keymap.insert(make_pair(XK_4, NE_KEY_4));
	_keymap.insert(make_pair(XK_5, NE_KEY_5));
	_keymap.insert(make_pair(XK_6, NE_KEY_6));
	_keymap.insert(make_pair(XK_7, NE_KEY_7));
	_keymap.insert(make_pair(XK_8, NE_KEY_8));
	_keymap.insert(make_pair(XK_9, NE_KEY_9));

	_keymap.insert(make_pair(XK_a, NE_KEY_A));
	_keymap.insert(make_pair(XK_b, NE_KEY_B));
	_keymap.insert(make_pair(XK_c, NE_KEY_C));
	_keymap.insert(make_pair(XK_d, NE_KEY_D));
	_keymap.insert(make_pair(XK_e, NE_KEY_E));
	_keymap.insert(make_pair(XK_f, NE_KEY_F));
	_keymap.insert(make_pair(XK_g, NE_KEY_G));
	_keymap.insert(make_pair(XK_h, NE_KEY_H));
	_keymap.insert(make_pair(XK_i, NE_KEY_I));
	_keymap.insert(make_pair(XK_j, NE_KEY_J));
	_keymap.insert(make_pair(XK_k, NE_KEY_K));
	_keymap.insert(make_pair(XK_l, NE_KEY_L));
	_keymap.insert(make_pair(XK_m, NE_KEY_M));
	_keymap.insert(make_pair(XK_n, NE_KEY_N));
	_keymap.insert(make_pair(XK_o, NE_KEY_O));
	_keymap.insert(make_pair(XK_p, NE_KEY_P));
	_keymap.insert(make_pair(XK_q, NE_KEY_Q));
	_keymap.insert(make_pair(XK_r, NE_KEY_R));
	_keymap.insert(make_pair(XK_s, NE_KEY_S));
	_keymap.insert(make_pair(XK_t, NE_KEY_T));
	_keymap.insert(make_pair(XK_u, NE_KEY_U));
	_keymap.insert(make_pair(XK_v, NE_KEY_V));
	_keymap.insert(make_pair(XK_w, NE_KEY_W));
	_keymap.insert(make_pair(XK_x, NE_KEY_X));
	_keymap.insert(make_pair(XK_y, NE_KEY_Y));
	_keymap.insert(make_pair(XK_z, NE_KEY_Z));

	_keymap.insert(make_pair(XK_Up, NE_KEY_UP));
	_keymap.insert(make_pair(XK_Down, NE_KEY_DOWN));
	_keymap.insert(make_pair(XK_Left, NE_KEY_LEFT));
	_keymap.insert(make_pair(XK_Right, NE_KEY_RIGHT));
	_keymap.insert(make_pair(XK_space, NE_KEY_SPACE));
	_keymap.insert(make_pair(XK_plus, NE_KEY_PLUS));
	_keymap.insert(make_pair(XK_minus, NE_KEY_MINUS));
	_keymap.insert(make_pair(XK_comma, NE_KEY_COMMA));
	_keymap.insert(make_pair(XK_period, NE_KEY_PERIOD));
	_keymap.insert(make_pair(XK_Scroll_Lock, NE_KEY_SCROLL));
	_keymap.insert(make_pair(XK_Shift_L, NE_KEY_LSHIFT));
	_keymap.insert(make_pair(XK_Shift_R, NE_KEY_RSHIFT));
	_keymap.insert(make_pair(XK_Alt_L, NE_KEY_LALT));
	_keymap.insert(make_pair(XK_Alt_R, NE_KEY_RALT));
	_keymap.insert(make_pair(XK_Super_L, NE_KEY_LSUPER));
	_keymap.insert(make_pair(XK_Super_R, NE_KEY_RSUPER));
	_keymap.insert(make_pair(XK_Control_L, NE_KEY_LCTRL));
	_keymap.insert(make_pair(XK_Control_R, NE_KEY_RCTRL));
	_keymap.insert(make_pair(XK_Page_Up, NE_KEY_PGUP));
	_keymap.insert(make_pair(XK_Page_Down, NE_KEY_PGDN));
	_keymap.insert(make_pair(XK_End, NE_KEY_END));
	_keymap.insert(make_pair(XK_Home, NE_KEY_HOME));
	_keymap.insert(make_pair(XK_Escape, NE_KEY_ESCAPE));
	_keymap.insert(make_pair(XK_Insert, NE_KEY_INSERT));
	_keymap.insert(make_pair(XK_Return, NE_KEY_RETURN));
	_keymap.insert(make_pair(XK_Caps_Lock, NE_KEY_CAPS));
	_keymap.insert(make_pair(XK_Delete, NE_KEY_DELETE));
	_keymap.insert(make_pair(XK_BackSpace, NE_KEY_BKSPACE));
	_keymap.insert(make_pair(XK_Tab, NE_KEY_TAB));
	_keymap.insert(make_pair(XK_Print, NE_KEY_PRTSCRN));
	_keymap.insert(make_pair(XK_Pause, NE_KEY_PAUSE));
	_keymap.insert(make_pair(XK_semicolon, NE_KEY_SEMICOLON));
	_keymap.insert(make_pair(XK_slash, NE_KEY_SLASH));
	_keymap.insert(make_pair(XK_asciitilde, NE_KEY_TILDE));
	_keymap.insert(make_pair(XK_bracketleft, NE_KEY_LBRACKET));
	_keymap.insert(make_pair(XK_backslash, NE_KEY_BKSLASH));
	_keymap.insert(make_pair(XK_bracketright, NE_KEY_RBRACKET));
	_keymap.insert(make_pair(XK_quotedbl, NE_KEY_QUOTE));

	_keymap.insert(make_pair(XK_F1, NE_KEY_F1));
	_keymap.insert(make_pair(XK_F2, NE_KEY_F2));
	_keymap.insert(make_pair(XK_F3, NE_KEY_F3));
	_keymap.insert(make_pair(XK_F4, NE_KEY_F4));
	_keymap.insert(make_pair(XK_F5, NE_KEY_F5));
	_keymap.insert(make_pair(XK_F6, NE_KEY_F6));
	_keymap.insert(make_pair(XK_F7, NE_KEY_F7));
	_keymap.insert(make_pair(XK_F8, NE_KEY_F8));
	_keymap.insert(make_pair(XK_F9, NE_KEY_F9));
	_keymap.insert(make_pair(XK_F10, NE_KEY_F10));
	_keymap.insert(make_pair(XK_F11, NE_KEY_F11));
	_keymap.insert(make_pair(XK_F12, NE_KEY_F12));
	_keymap.insert(make_pair(XK_F13, NE_KEY_F13));
	_keymap.insert(make_pair(XK_F14, NE_KEY_F14));
	_keymap.insert(make_pair(XK_F15, NE_KEY_F15));
	_keymap.insert(make_pair(XK_F16, NE_KEY_F16));
	_keymap.insert(make_pair(XK_F17, NE_KEY_F17));
	_keymap.insert(make_pair(XK_F18, NE_KEY_F18));
	_keymap.insert(make_pair(XK_F19, NE_KEY_F19));
	_keymap.insert(make_pair(XK_F20, NE_KEY_F20));
	_keymap.insert(make_pair(XK_F21, NE_KEY_F21));
	_keymap.insert(make_pair(XK_F22, NE_KEY_F22));
	_keymap.insert(make_pair(XK_F23, NE_KEY_F23));
	_keymap.insert(make_pair(XK_F24, NE_KEY_F24));

	_keymap.insert(make_pair(XK_Num_Lock, NE_KEY_NUMLOCK));
	_keymap.insert(make_pair(XK_KP_0, NE_KEY_NUM_0));
	_keymap.insert(make_pair(XK_KP_1, NE_KEY_NUM_1));
	_keymap.insert(make_pair(XK_KP_2, NE_KEY_NUM_2));
	_keymap.insert(make_pair(XK_KP_3, NE_KEY_NUM_3));
	_keymap.insert(make_pair(XK_KP_4, NE_KEY_NUM_4));
	_keymap.insert(make_pair(XK_KP_5, NE_KEY_NUM_5));
	_keymap.insert(make_pair(XK_KP_6, NE_KEY_NUM_6));
	_keymap.insert(make_pair(XK_KP_7, NE_KEY_NUM_7));
	_keymap.insert(make_pair(XK_KP_8, NE_KEY_NUM_8));
	_keymap.insert(make_pair(XK_KP_9, NE_KEY_NUM_9));
	_keymap.insert(make_pair(XK_KP_Add, NE_KEY_NUM_PLUS));
	_keymap.insert(make_pair(XK_KP_Subtract, NE_KEY_NUM_MINUS));
	_keymap.insert(make_pair(XK_KP_Divide, NE_KEY_NUM_DIVIDE));
	_keymap.insert(make_pair(XK_KP_Multiply, NE_KEY_NUM_MULT));
	_keymap.insert(make_pair(XK_KP_Decimal, NE_KEY_NUM_DECIMAL));
}

int Input::_GetControllerCount()
{
	return 0;
}

bool Input::_GetControllerState(int n, ControllerState *state)
{
	return false;
}