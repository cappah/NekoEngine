/* NekoEngine
 *
 * InputInterface.cpp
 * Author: Alexandru Naiman
 *
 * Input script interface
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

#include <Input/Input.h>
#include <Script/Interface/InputInterface.h>

void InputInterface::Register(lua_State *state)
{
	lua_register(state, "I_GetButton", GetButton);
	lua_register(state, "I_GetButtonUp", GetButtonUp);
	lua_register(state, "I_GetButtonDown", GetButtonDown);
	lua_register(state, "I_GetAxis", GetAxis);
	lua_register(state, "I_SetControllerVibration", SetControllerVibration);
	lua_register(state, "I_GetConnectedControllerCount", GetConnectedControllerCount);
	lua_register(state, "I_EnableMouseAxis", EnableMouseAxis);
	lua_register(state, "I_CapturePointer", CapturePointer);
	lua_register(state, "I_ReleasePointer", ReleasePointer);
	lua_register(state, "I_SetPointerPosition", SetPointerPosition);
	lua_register(state, "I_GetPointerPosition", GetPointerPosition);
}

int InputInterface::GetButton(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	lua_pushboolean(state, Input::GetButton(lua_tostring(state, 1)));

	return 1;
}

int InputInterface::GetButtonUp(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	lua_pushboolean(state, Input::GetButtonUp(lua_tostring(state, 1)));

	return 1;
}

int InputInterface::GetButtonDown(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	lua_pushboolean(state, Input::GetButtonDown(lua_tostring(state, 1)));

	return 1;
}

int InputInterface::GetAxis(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
	{
		lua_pushnumber(state, 0.f);
		return luaL_error(state, "Invalid arguments");
	}

	lua_pushnumber(state, Input::GetAxis(lua_tostring(state, 1)));

	return 1;
}

int InputInterface::SetControllerVibration(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 3)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	lua_pushboolean(state, Input::SetControllerVibration((int)lua_tointeger(state, 1), (float)lua_tonumber(state, 2), (float)lua_tonumber(state, 3)));

	return 1;
}

int InputInterface::GetConnectedControllerCount(lua_State *state)
{
	(void)state;
	lua_pushinteger(state, Input::GetConnectedControllerCount());
	return 1;
}

int InputInterface::EnableMouseAxis(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	lua_pushboolean(state, Input::EnableMouseAxis(lua_toboolean(state, 1)));

	return 1;
}

int InputInterface::GetPointerPosition(lua_State *state)
{
	long x{ 0 }, y{ 0 };

	lua_pushboolean(state, Input::GetPointerPosition(x, y));
	lua_pushinteger(state, x);
	lua_pushinteger(state, y);

	return 3;
}

int InputInterface::SetPointerPosition(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	lua_pushboolean(state, Input::SetPointerPosition((long)lua_tointeger(state, 1), (long)lua_tointeger(state, 2)));

	return 1;
}

int InputInterface::CapturePointer(lua_State *state)
{
	(void)state;
	lua_pushboolean(state, Input::CapturePointer());
	return 1;
}

int InputInterface::ReleasePointer(lua_State *state)
{
	(void)state;
	Input::ReleasePointer();
	return 0;
}
