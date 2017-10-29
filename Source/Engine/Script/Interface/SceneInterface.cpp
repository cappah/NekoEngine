/* NekoEngine
 *
 * SceneInterface.h
 * Author: Alexandru Naiman
 *
 * Scene script interface
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

#include <Scene/Scene.h>
#include <Scene/SceneManager.h>
#include <Script/Interface/SceneInterface.h>

void SceneInterface::Register(lua_State *state)
{
	lua_register(state, "SC_GetObjectByID", GetObjectByID);
	lua_register(state, "SC_GetObjectByName", GetObjectByName);
	lua_register(state, "SC_GetObjectCount", GetObjectCount);
	lua_register(state, "SC_AddObject", AddObject);
	lua_register(state, "SC_RemoveObject", RemoveObject);
}

int SceneInterface::GetObjectByID(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	Scene *scn = SceneManager::GetActiveScene();
	if (!scn) scn = SceneManager::GetLoadingScene();
	lua_pushlightuserdata(state, scn ? scn->GetObjectByID((int32_t)lua_tonumber(state, 1)) : nullptr);

	return 1;
}

int SceneInterface::GetObjectByName(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	Scene *scn = SceneManager::GetActiveScene();
	if (!scn) scn = SceneManager::GetLoadingScene();
	lua_pushlightuserdata(state, scn ? scn->GetObjectByName(lua_tostring(state, 1)) : nullptr);

	return 1;
}

int SceneInterface::GetObjectCount(lua_State *state)
{
	(void)state;
	Scene *scn = SceneManager::GetActiveScene();
	if (!scn) scn = SceneManager::GetLoadingScene();
	lua_pushinteger(state, scn ? scn->GetObjectCount() : 0);
	return 1;
}

int SceneInterface::AddObject(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	Scene *scn = SceneManager::GetActiveScene();
	if (!scn) scn = SceneManager::GetLoadingScene();
	if (scn) scn->AddObject((Object *)lua_touserdata(state, 1));

	return 0;
}

int SceneInterface::RemoveObject(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	Scene *scn = SceneManager::GetActiveScene();
	if (!scn) scn = SceneManager::GetLoadingScene();
	if (scn) scn->RemoveObject((Object *)lua_touserdata(state, 1));

	return 0;
}
