/* NekoEngine
 *
 * GUIInterface.h
 * Author: Alexandru Naiman
 *
 * Engine script interface
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
#include <Engine/Version.h>
#include <Scene/Object.h>
#include <Scene/ObjectComponent.h>
#include <Script/Interface/EngineInterface.h>

using namespace std;

void EngineInterface::Register(lua_State *state)
{
	lua_register(state, "E_GetVersion", GetVersion);
	lua_register(state, "E_GetScreenWidth", GetScreenWidth);
	lua_register(state, "E_GetScreenHeight", GetScreenHeight);
	lua_register(state, "E_IsPaused", IsPaused);
	lua_register(state, "E_TogglePause", TogglePause);
	lua_register(state, "E_ToggleStats", ToggleStats);
	lua_register(state, "E_NewObject", NewObject);
	lua_register(state, "E_NewComponent", NewComponent);
	lua_register(state, "E_Exit", Exit);
}

int EngineInterface::GetVersion(lua_State *state)
{
	lua_pushstring(state, ENGINE_VERSION_STRING);
	return 1;
}

int EngineInterface::GetScreenWidth(lua_State *state)
{
	lua_pushinteger(state, Engine::GetScreenWidth());
	return 1;
}

int EngineInterface::GetScreenHeight(lua_State *state)
{
	lua_pushinteger(state, Engine::GetScreenHeight());
	return 1;
}

int EngineInterface::IsPaused(lua_State *state)
{
	lua_pushboolean(state, Engine::IsPaused());
	return 1;
}

int EngineInterface::TogglePause(lua_State *state)
{
	(void)state;
	Engine::TogglePause();
	return 0;
}

int EngineInterface::ToggleStats(lua_State *state)
{
	(void)state;
	Engine::ToggleStats();
	return 0;
}

int EngineInterface::NewObject(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args < 3)
	{
		lua_pushlightuserdata(state, nullptr);
		return luaL_error(state, "Invalid arguments");
	}

	NString className = lua_tostring(state, 1);

	ObjectInitializer oi{};

	oi.name = lua_tostring(state, 2);
	oi.parent = (Object *)lua_touserdata(state, 3);

	if (args > 3)
	{
		for (int i = 4; i <= args; ++i)
		{
			NString str = lua_tostring(state, i);
			NArray<NString> split = str.Split('=');

			if (split.Count() != 2)
			{
				lua_pushlightuserdata(state, nullptr);
				return luaL_error(state, "Invalid arguments");
			}

			oi.arguments.insert(make_pair(*split[0], *split[1]));
		}
	}

	Object *obj = Engine::NewObject(*className, &oi);
	lua_pushlightuserdata(state, obj);

	return 1;
}

int EngineInterface::NewComponent(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args < 2)
	{
		lua_pushlightuserdata(state, nullptr);
		return luaL_error(state, "Invalid arguments");
	}

	NString className = lua_tostring(state, 1);

	ComponentInitializer ci{};
	ci.parent = (Object *)lua_touserdata(state, 2);

	if (args > 2)
	{
		for (int i = 3; i <= args; ++i)
		{
			NString str = lua_tostring(state, i);
			NArray<NString> split = str.Split('=');

			if (split.Count() != 2)
			{
				lua_pushlightuserdata(state, nullptr);
				return luaL_error(state, "Invalid arguments");
			}

			ci.arguments.insert(make_pair(*split[0], *split[1]));
		}
	}

	ObjectComponent *comp = Engine::NewComponent(*className, &ci);
	lua_pushlightuserdata(state, comp);

	return 1;
}

int EngineInterface::Exit(lua_State *state)
{
	(void)state;
	Engine::Exit();
	return 0;
}
