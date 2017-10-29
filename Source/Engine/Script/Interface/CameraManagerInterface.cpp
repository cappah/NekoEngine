/* NekoEngine
 *
 * CameraManagerInterface.cpp
 * Author: Alexandru Naiman
 *
 * CameraManager script interface
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

#include <Scene/CameraManager.h>
#include <Script/Interface/CameraManagerInterface.h>

void CameraManagerInterface::Register(lua_State *state)
{
	lua_register(state, "CM_GetActiveCamera", GetActiveCamera);
	lua_register(state, "CM_SetActiveCamera", SetActiveCamera);
	lua_register(state, "CM_SetActiveCameraId", SetActiveCameraId);
	lua_register(state, "CM_AddCamera", AddCamera);
}

int CameraManagerInterface::GetActiveCamera(lua_State *state)
{
	lua_pushlightuserdata(state, CameraManager::GetActiveCamera());
	return 1;
}

int CameraManagerInterface::SetActiveCamera(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	CameraManager::SetActiveCamera((Camera *)lua_touserdata(state, 1));

	return 0;
}

int CameraManagerInterface::SetActiveCameraId(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	CameraManager::SetActiveCameraId((int32_t)lua_tointeger(state, 1));

	return 0;
}

int CameraManagerInterface::AddCamera(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushinteger(state, CameraManager::AddCamera((Camera *)lua_touserdata(state, 1)));

	return 1;
}
