/* NekoEngine
 *
 * ObjectComponentInterface.cpp
 * Author: Alexandru Naiman
 *
 * ObjectComponent script interface
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

#include <Scene/ObjectComponent.h>
#include <Script/Interface/ObjectComponentInterface.h>

void ObjectComponentInterface::Register(lua_State *state)
{
	lua_register(state, "OC_GetParent", GetParent);
	lua_register(state, "OC_Load", Load);
	lua_register(state, "OC_InitializeComponent", InitializeComponent);
	lua_register(state, "OC_Enable", Enable);
	lua_register(state, "OC_IsEnabled", IsEnabled);
}

int ObjectComponentInterface::GetParent(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushlightuserdata(state, ((ObjectComponent *)lua_touserdata(state, 1))->GetParent());

	return 1;
}

int ObjectComponentInterface::Load(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushinteger(state, ((ObjectComponent *)lua_touserdata(state, 1))->Load());

	return 1;
}

int ObjectComponentInterface::InitializeComponent(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushinteger(state, ((ObjectComponent *)lua_touserdata(state, 1))->InitializeComponent());

	return 1;
}

int ObjectComponentInterface::Enable(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	((ObjectComponent *)lua_touserdata(state, 1))->Enable(lua_toboolean(state, 2));

	return 0;
}

int ObjectComponentInterface::IsEnabled(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushboolean(state, ((ObjectComponent *)lua_touserdata(state, 1))->IsEnabled());

	return 1;
}