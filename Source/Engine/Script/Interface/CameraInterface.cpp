/* NekoEngine
 *
 * CameraInterface.cpp
 * Author: Alexandru Naiman
 *
 * Camera script interface
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

#include <Engine/Engine.h>
#include <Engine/Camera.h>
#include <Script/Interface/CameraInterface.h>

using namespace glm;

void CameraInterface::Register(lua_State *state)
{
	lua_register(state, "Cam_GetForward", GetForward);
	lua_register(state, "Cam_GetRight", GetRight);
	lua_register(state, "Cam_GetPosition", GetPosition);
	lua_register(state, "Cam_GetRotation", GetRotation);
	lua_register(state, "Cam_GetViewMatrix", GetViewMatrix);
	lua_register(state, "Cam_GetProjectionMatrix", GetProjectionMatrix);
	lua_register(state, "Cam_GetNear", GetNear);
	lua_register(state, "Cam_GetFar", GetFar);
	lua_register(state, "Cam_GetViewDistance", GetViewDistance);
	lua_register(state, "Cam_GetFogDistance", GetFogDistance);
	lua_register(state, "Cam_GetFogColor", GetFogColor);

	lua_register(state, "Cam_SetPosition", SetPosition);
	lua_register(state, "Cam_SetRotation", SetRotation);
	lua_register(state, "Cam_SetNear", SetNear);
	lua_register(state, "Cam_SetFar", SetFar);
	lua_register(state, "Cam_SetViewDistance", SetViewDistance);
	lua_register(state, "Cam_SetFogDistance", SetFogDistance);
	lua_register(state, "Cam_SetFogColor", SetFogColor);

	lua_register(state, "Cam_EnableSkybox", EnableSkybox);
	lua_register(state, "Cam_LookAt", LookAt);

	lua_register(state, "Cam_MoveForward", MoveForward);
	lua_register(state, "Cam_MoveRight", MoveRight);
	lua_register(state, "Cam_MoveUp", MoveUp);

	lua_register(state, "Cam_RotateX", RotateX);
	lua_register(state, "Cam_RotateY", RotateY);
	lua_register(state, "Cam_RotateZ", RotateZ);
}

int CameraInterface::GetForward(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushlightuserdata(state, value_ptr(((Camera *)lua_touserdata(state, 1))->GetForward()));

	return 1;
}

int CameraInterface::GetRight(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushlightuserdata(state, value_ptr(((Camera *)lua_touserdata(state, 1))->GetRight()));

	return 1;
}

int CameraInterface::GetPosition(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushlightuserdata(state, value_ptr(((Camera *)lua_touserdata(state, 1))->GetPosition()));

	return 1;
}

int CameraInterface::GetRotation(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushlightuserdata(state, value_ptr(((Camera *)lua_touserdata(state, 1))->GetRotation()));

	return 1;
}

int CameraInterface::GetViewMatrix(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushlightuserdata(state, value_ptr(((Camera *)lua_touserdata(state, 1))->GetView()));

	return 1;
}

int CameraInterface::GetProjectionMatrix(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushlightuserdata(state, value_ptr(((Camera *)lua_touserdata(state, 1))->GetProjectionMatrix()));

	return 1;
}

int CameraInterface::GetNear(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushnumber(state, ((Camera *)lua_touserdata(state, 1))->GetNear());

	return 1;
}

int CameraInterface::GetFar(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushnumber(state, ((Camera *)lua_touserdata(state, 1))->GetFar());

	return 1;
}

int CameraInterface::GetViewDistance(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushnumber(state, ((Camera *)lua_touserdata(state, 1))->GetViewDistance());

	return 1;
}

int CameraInterface::GetFogDistance(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushnumber(state, ((Camera *)lua_touserdata(state, 1))->GetFogDistance());

	return 1;
}

int CameraInterface::GetFogColor(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushlightuserdata(state, value_ptr(((Camera *)lua_touserdata(state, 1))->GetFogColor()));

	return 1;
}


int CameraInterface::SetPosition(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	float *f{ (float *)lua_touserdata(state, 2) };
	vec3 v{ f[0], f[1], f[2] };

	((Camera *)lua_touserdata(state, 1))->SetPosition(v, true);

	return 0;
}

int CameraInterface::SetRotation(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	float *f{ (float *)lua_touserdata(state, 2) };
	vec3 v{ f[0], f[1], f[2] };

	((Camera *)lua_touserdata(state, 1))->SetRotation(v, true);

	return 0;
}

int CameraInterface::SetNear(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	((Camera *)lua_touserdata(state, 1))->SetNear((float)lua_tonumber(state, 2));

	return 0;
}

int CameraInterface::SetFar(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	((Camera *)lua_touserdata(state, 1))->SetFar((float)lua_tonumber(state, 2));

	return 0;
}

int CameraInterface::SetViewDistance(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	((Camera *)lua_touserdata(state, 1))->SetViewDistance((float)lua_tonumber(state, 2));

	return 0;
}

int CameraInterface::SetFogDistance(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	((Camera *)lua_touserdata(state, 1))->SetFogDistance((float)lua_tonumber(state, 2));

	return 0;
}

int CameraInterface::SetFogColor(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	float *f{ (float *)lua_touserdata(state, 2) };
	vec3 v{ f[0], f[1], f[2] };

	((Camera *)lua_touserdata(state, 1))->SetFogColor(v);

	return 0;
}

int CameraInterface::EnableSkybox(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	((Camera *)lua_touserdata(state, 1))->EnableSkybox(lua_toboolean(state, 2));

	return 0;
}

int CameraInterface::LookAt(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 4)
		return luaL_error(state, "Invalid arguments");

	float *f{ (float *)lua_touserdata(state, 2) };
	vec3 eye{ f[0], f[1], f[2] };
	f = (float *)lua_touserdata(state, 3);
	vec3 center{ f[0], f[1], f[2] };
	f = (float *)lua_touserdata(state, 4);
	vec3 up{ f[0], f[1], f[2] };

	((Camera *)lua_touserdata(state, 1))->LookAt(eye, center, up);

	return 0;
}

int CameraInterface::MoveForward(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	((Camera *)lua_touserdata(state, 1))->MoveForward((float)lua_tonumber(state, 2));

	return 0;
}

int CameraInterface::MoveRight(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	((Camera *)lua_touserdata(state, 1))->MoveRight((float)lua_tonumber(state, 2));

	return 0;
}

int CameraInterface::MoveUp(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	((Camera *)lua_touserdata(state, 1))->MoveUp((float)lua_tonumber(state, 2));

	return 0;
}

int CameraInterface::RotateX(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	((Camera *)lua_touserdata(state, 1))->RotateX((float)lua_tonumber(state, 2));

	return 0;
}

int CameraInterface::RotateY(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	((Camera *)lua_touserdata(state, 1))->RotateY((float)lua_tonumber(state, 2));

	return 0;
}

int CameraInterface::RotateZ(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
		return luaL_error(state, "Invalid arguments");

	((Camera *)lua_touserdata(state, 1))->RotateZ((float)lua_tonumber(state, 2));

	return 0;
}
