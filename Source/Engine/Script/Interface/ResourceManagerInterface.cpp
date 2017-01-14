/* NekoEngine
 *
 * ResourceManagerInterface.cpp
 * Author: Alexandru Naiman
 *
 * ResourceManager script interface
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

#include <Engine/ResourceManager.h>
#include <Script/Interface/ResourceManagerInterface.h>

void ResourceManagerInterface::Register(lua_State *state)
{
	lua_pushinteger(state, (int)ResourceType::RES_STATIC_MESH);
	lua_setglobal(state, "RES_STATIC_MESH");

	lua_pushinteger(state, (int)ResourceType::RES_SKELETAL_MESH);
	lua_setglobal(state, "RES_SKELETAL_MESH");

	lua_pushinteger(state, (int)ResourceType::RES_TEXTURE);
	lua_setglobal(state, "RES_TEXTURE");

	lua_pushinteger(state, (int)ResourceType::RES_SHADERMODULE);
	lua_setglobal(state, "RES_SHADERMODULE");

	lua_pushinteger(state, (int)ResourceType::RES_AUDIOCLIP);
	lua_setglobal(state, "RES_AUDIOCLIP");

	lua_pushinteger(state, (int)ResourceType::RES_FONT);
	lua_setglobal(state, "RES_FONT");

	lua_pushinteger(state, (int)ResourceType::RES_MATERIAL);
	lua_setglobal(state, "RES_MATERIAL");

	lua_pushinteger(state, (int)ResourceType::RES_ANIMCLIP);
	lua_setglobal(state, "RES_ANIMCLIP");

	lua_register(state, "RM_GetPathForResource", GetPathForResource);
	lua_register(state, "RM_GetResource", GetResource);
	lua_register(state, "RM_UnloadResource", UnloadResource);
}

int ResourceManagerInterface::GetPathForResource(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	NString str = ResourceManager::GetPathForResource(lua_tostring(state, 1), (ResourceType)lua_tointeger(state, 2));
	lua_pushstring(state, *str);

	return 1;
}

int ResourceManagerInterface::GetResource(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	Resource *r = ResourceManager::GetResourceByName(lua_tostring(state, 1), (ResourceType)lua_tointeger(state, 2));
	lua_pushlightuserdata(state, r);

	return 1;
}

int ResourceManagerInterface::UnloadResource(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	ResourceManager::UnloadResourceByName(lua_tostring(state, 1), (ResourceType)lua_tointeger(state, 2));

	return 0;
}
