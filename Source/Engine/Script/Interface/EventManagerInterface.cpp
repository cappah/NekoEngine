/* NekoEngine
 *
 * EventManagerInterface.h
 * Author: Alexandru Naiman
 *
 * EventManager script interface
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

#include <Engine/EventManager.h>
#include <Script/Interface/EventManagerInterface.h>

void EventManagerInterface::Register(lua_State *state)
{
	lua_register(state, "EM_RegisterHandler",RegisterHandler);
	lua_register(state, "EM_UnregisterHandler", UnregisterHandler);
	lua_register(state, "EM_Broadcast", Broadcast);
}

int EventManagerInterface::RegisterHandler(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	luaL_checktype(state, 2, LUA_TFUNCTION);

	int top = lua_gettop(state);
	lua_settop(state, 2);
	int funcRef = luaL_ref(state, LUA_REGISTRYINDEX);
	lua_settop(state, top);

	lua_pushinteger(state, EventManager::RegisterHandler((int32_t)lua_tointeger(state, 1), [state, funcRef](int32_t event, void *eventArgs) {
		lua_rawgeti(state, LUA_REGISTRYINDEX, funcRef);
		lua_pushinteger(state, event);
		lua_pushlightuserdata(state, eventArgs);
		lua_pcall(state, 2, 0, 0);
	}));

	return 1;
}

int EventManagerInterface::UnregisterHandler(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	EventManager::UnregisterHandler((int32_t)lua_tointeger(state, 1), (uint32_t)lua_tointeger(state, 2));

	return 0;
}

int EventManagerInterface::Broadcast(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	EventManager::Broadcast((int32_t)lua_tointeger(state, 1), lua_touserdata(state, 2));

	return 0;
}
