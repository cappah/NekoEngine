/* NekoEngine
 *
 * SystemInterface.cpp
 * Author: Alexandru Naiman
 *
 * Core script functions
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

#include <Script/Interface/SystemInterface.h>

void SystemInterface::Register(lua_State *state)
{
	lua_register(state, "Sys_Alloc", Alloc);
	lua_register(state, "Sys_Free", Free);

	lua_register(state, "Sys_GetUInt8", GetUInt8);
	lua_register(state, "Sys_GetInt", GetInt);
	lua_register(state, "Sys_GetStr", GetStr);
	lua_register(state, "Sys_GetBool", GetBool);

	lua_register(state, "Sys_SetUInt8", SetUInt8);
	lua_register(state, "Sys_SetInt", SetInt);
	lua_register(state, "Sys_SetStr", SetStr);
	lua_register(state, "Sys_SetBool", SetBool);
}

int SystemInterface::Alloc(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");
	
	lua_pushlightuserdata(state, calloc(1, lua_tointeger(state, 2)));

	return 1;
}

int SystemInterface::Free(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	free(lua_touserdata(state, 1));

	return 0;
}

int SystemInterface::GetUInt8(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushinteger(state, *((uint8_t *)lua_touserdata(state, 1)));

	return 1;
}

int SystemInterface::GetInt(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushinteger(state, *((int *)lua_touserdata(state, 2)));

	return 1;
}

int SystemInterface::GetStr(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushstring(state, (char *)lua_touserdata(state, 2));

	return 1;
}

int SystemInterface::GetBool(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushboolean(state, *((bool *)lua_touserdata(state, 2)));

	return 1;
}

int SystemInterface::SetUInt8(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	*((uint8_t *)lua_touserdata(state, 1)) = lua_tointeger(state, 2);

	return 0;
}

int SystemInterface::SetInt(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	*((int *)lua_touserdata(state, 1)) = lua_tointeger(state, 2);

	return 0;
}

int SystemInterface::SetStr(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	char *dst = (char *)lua_touserdata(state, 1);
	const char *src = lua_tostring(state, 2);
	size_t len = strlen(src);

	strncpy(dst, src, len);

	return 0;
}

int SystemInterface::SetBool(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	*((bool *)lua_touserdata(state, 1)) = lua_toboolean(state, 2);

	return 0;
}