/* NekoEngine
 *
 * ObjectInterface.h
 * Author: Alexandru Naiman
 *
 * Object script interface
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

#include <Scene/Object.h>
#include <Engine/Engine.h>
#include <Script/Interface/ObjectInterface.h>

using namespace glm;

void ObjectInterface::Register(lua_State *state)
{
	lua_register(state, "O_GetPosition", GetPosition);
	lua_register(state, "O_GetRotation", GetRotation);
	lua_register(state, "O_GetScale", GetScale);
	lua_register(state, "O_SetPosition", SetPosition);
	lua_register(state, "O_SetRotation", SetRotation);
	lua_register(state, "O_SetScale", SetScale);
	lua_register(state, "O_LookAt", LookAt);
	lua_register(state, "O_AddComponent", AddComponent);
	lua_register(state, "O_GetComponent", GetComponent);
	lua_register(state, "O_RemoveComponent", RemoveComponent);
	lua_register(state, "O_Load", Load);
	lua_register(state, "O_AddToScene", AddToScene);
	lua_register(state, "O_Destroy", Destroy);
}

int ObjectInterface::GetPosition(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushlightuserdata(state, value_ptr(((Object *)lua_touserdata(state, 1))->GetPosition()));

	return 1;
}

int ObjectInterface::GetRotation(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushlightuserdata(state, value_ptr(((Object *)lua_touserdata(state, 1))->GetRotationAngles()));

	return 1;
}

int ObjectInterface::GetScale(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushlightuserdata(state, value_ptr(((Object *)lua_touserdata(state, 1))->GetScale()));

	return 1;
}

int ObjectInterface::SetPosition(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	float *pos{ (float *)lua_touserdata(state, 2) };
	vec3 v{ pos[0], pos[1], pos[2] };

	((Object *)lua_touserdata(state, 1))->SetPosition(v);

	return 0;
}

int ObjectInterface::SetRotation(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	float *rot{ (float *)lua_touserdata(state, 2) };
	vec3 v{ rot[0], rot[1], rot[2] };

	((Object *)lua_touserdata(state, 1))->SetRotation(v);

	return 0;
}

int ObjectInterface::SetScale(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	float *scale{ (float *)lua_touserdata(state, 2) };
	vec3 v{ scale[0], scale[1], scale[2] };

	((Object *)lua_touserdata(state, 1))->SetScale(v);

	return 0;
}

int ObjectInterface::LookAt(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	float *pt{ (float *)lua_touserdata(state, 2) };
	vec3 v{ pt[0], pt[1], pt[2] };

	((Object *)lua_touserdata(state, 1))->LookAt(v);

	return 0;
}

int ObjectInterface::AddComponent(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 3)
		return luaL_error(state, "Invalid arguments");

	((Object *)lua_touserdata(state, 1))->AddComponent(lua_tostring(state, 2), (ObjectComponent *)lua_touserdata(state, 3));

	return 0;
}

int ObjectInterface::GetComponent(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	lua_pushlightuserdata(state, ((Object *)lua_touserdata(state, 1))->GetComponent(lua_tostring(state, 2)));

	return 1;
}

int ObjectInterface::RemoveComponent(lua_State *state)
{
	int argc{ lua_gettop(state) };
	bool force{ false };

	if (argc < 2)
		return luaL_error(state, "Invalid arguments");
	else if (argc == 3)
		force = lua_toboolean(state, 3);

	lua_pushboolean(state, ((Object *)lua_touserdata(state, 1))->RemoveComponent(lua_tostring(state, 2), force));

	return 1;
}

int ObjectInterface::Load(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushinteger(state, ((Object *)lua_touserdata(state, 1))->Load());

	return 1;
}

int ObjectInterface::AddToScene(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	((Object *)lua_touserdata(state, 1))->AddToScene();

	return 0;
}

int ObjectInterface::Destroy(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	((Object *)lua_touserdata(state, 1))->Destroy();

	return 0;
}
