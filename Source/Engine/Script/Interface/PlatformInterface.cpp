/* NekoEngine
 *
 * PlatformInterface.cpp
 * Author: Alexandru Naiman
 *
 * Platform script interface
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

#include <Platform/Platform.h>
#include <Script/Interface/PlatformInterface.h>

void PlatformInterface::Register(lua_State *state)
{
	lua_pushinteger(state, (int)MessageBoxButtons::OK);
	lua_setglobal(state, "MB_BTN_OK");

	lua_pushinteger(state, (int)MessageBoxButtons::YesNo);
	lua_setglobal(state, "MB_BTN_YESNO");

	lua_pushinteger(state, (int)MessageBoxIcon::Question);
	lua_setglobal(state, "MB_ICON_QUESTION");

	lua_pushinteger(state, (int)MessageBoxIcon::Information);
	lua_setglobal(state, "MB_ICON_INFORMATION");

	lua_pushinteger(state, (int)MessageBoxIcon::Warning);
	lua_setglobal(state, "MB_ICON_WARNING");

	lua_pushinteger(state, (int)MessageBoxIcon::Error);
	lua_setglobal(state, "MB_ICON_ERROR");

	lua_pushinteger(state, (int)MessageBoxResult::OK);
	lua_setglobal(state, "MB_RES_OK");

	lua_pushinteger(state, (int)MessageBoxResult::Yes);
	lua_setglobal(state, "MB_RES_YES");

	lua_pushinteger(state, (int)MessageBoxResult::No);
	lua_setglobal(state, "MB_RES_NO");

	lua_register(state, "P_GetName", GetName);
	lua_register(state, "P_GetMachineName", GetMachineName);
	lua_register(state, "P_GetMachineArchitecture", GetMachineArchitecture);
	lua_register(state, "P_GetVersion", GetVersion);
	lua_register(state, "P_SetWindowTitle", SetWindowTitle);
	lua_register(state, "P_EnterFullscreen", EnterFullscreen);
	lua_register(state, "P_MessageBox", MessageBox);
	lua_register(state, "P_Rand", Rand);
}

int PlatformInterface::GetName(lua_State *state)
{
	lua_pushstring(state, Platform::GetName());
	return 1;
}

int PlatformInterface::GetMachineName(lua_State *state)
{
	lua_pushstring(state, Platform::GetMachineName());
	return 1;
}

int PlatformInterface::GetMachineArchitecture(lua_State *state)
{
	lua_pushstring(state, Platform::GetMachineArchitecture());
	return 1;
}

int PlatformInterface::GetVersion(lua_State *state)
{
	lua_pushstring(state, Platform::GetVersion());
	return 1;
}

int PlatformInterface::SetWindowTitle(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	Platform::SetWindowTitle(Platform::GetActiveWindow(), lua_tostring(state, 1));
	return 0;
}

int PlatformInterface::EnterFullscreen(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	lua_pushboolean(state, Platform::EnterFullscreen((int)lua_tointeger(state, 1), (int)lua_tointeger(state, 2)));

	return 1;
}

int PlatformInterface::MessageBox(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 4)
		return luaL_error(state, "Invalid arguments");

	lua_pushinteger(state, (int)Platform::MessageBox(lua_tostring(state, 1), lua_tostring(state, 2), (MessageBoxButtons)lua_tointeger(state, 3), (MessageBoxIcon)lua_tointeger(state, 4)));

	return 1;
}

int PlatformInterface::Rand(lua_State *state)
{
	lua_pushinteger(state, Platform::Rand());
	return 1;
}
