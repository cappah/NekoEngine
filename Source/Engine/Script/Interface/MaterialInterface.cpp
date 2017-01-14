/* NekoEngine
 *
 * MaterialInterface.cpp
 * Author: Alexandru Naiman
 *
 * Material script interface
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

#include <Renderer/Material.h>
#include <Script/Interface/MaterialInterface.h>

void MaterialInterface::Register(lua_State *state)
{
	lua_pushinteger(state, MaterialType::MT_Phong);
	lua_setglobal(state, "MT_Phong");

	lua_pushinteger(state, MaterialType::MT_PhongSpecular);
	lua_setglobal(state, "MT_PhongSpecular");

	lua_pushinteger(state, MaterialType::MT_PhongSpecularEmission);
	lua_setglobal(state, "MT_PhongSpecularEmission");

	lua_pushinteger(state, MaterialType::MT_NormalPhong);
	lua_setglobal(state, "MT_NormalPhong");

	lua_pushinteger(state, MaterialType::MT_NormalPhongSpecular);
	lua_setglobal(state, "MT_NormalPhongSpecular");

	lua_pushinteger(state, MaterialType::MT_NormalPhongSpecularEmission);
	lua_setglobal(state, "MT_NormalPhongSpecularEmission");

	lua_pushinteger(state, MaterialType::MT_Unlit);
	lua_setglobal(state, "MT_Unlit");

	lua_pushinteger(state, MaterialType::MT_Skysphere);
	lua_setglobal(state, "MT_Skysphere");

	lua_pushinteger(state, MaterialType::MT_SkysphereReflection);
	lua_setglobal(state, "MT_SkysphereReflection");

	lua_pushinteger(state, MaterialType::MT_Terrain);
	lua_setglobal(state, "MT_Terrain");

	lua_register(state, "Mat_GetType", GetType);
	lua_register(state, "Mat_IsTransparent", IsTransparent);
	lua_register(state, "Mat_SetType", SetType);
	lua_register(state, "Mat_SetDiffuseTexture", SetDiffuseTexture);
	lua_register(state, "Mat_SetNormalTexture", SetNormalTexture);
	lua_register(state, "Mat_SetSpecularTexture", SetSpecularTexture);
	lua_register(state, "Mat_SetEmissionTexture", SetEmissionTexture);
}

int MaterialInterface::GetType(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushinteger(state, ((Material *)lua_touserdata(state, 1))->GetType());

	return 1;
}

int MaterialInterface::IsTransparent(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushboolean(state, ((Material *)lua_touserdata(state, 1))->IsTransparent());

	return 1;
}

int MaterialInterface::SetType(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 2)
		return luaL_error(state, "Invalid arguments");

	((Material *)lua_touserdata(state, 1))->SetType((MaterialType)lua_tointeger(state, 2));

	return 0;
}

int MaterialInterface::SetDiffuseTexture(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 2)
		return luaL_error(state, "Invalid arguments");

	lua_pushlightuserdata(state, ((Material *)lua_touserdata(state, 1))->SetDiffuseTexture((Texture *)lua_touserdata(state, 2)));

	return 1;
}

int MaterialInterface::SetNormalTexture (lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 2)
		return luaL_error(state, "Invalid arguments");

	lua_pushlightuserdata(state, ((Material *)lua_touserdata(state, 1))->SetNormalTexture((Texture *)lua_touserdata(state, 2)));

	return 1;
}

int MaterialInterface::SetSpecularTexture(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 2)
		return luaL_error(state, "Invalid arguments");

	lua_pushlightuserdata(state, ((Material *)lua_touserdata(state, 1))->SetSpecularTexture((Texture *)lua_touserdata(state, 2)));

	return 1;
}

int MaterialInterface::SetEmissionTexture(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 2)
		return luaL_error(state, "Invalid arguments");

	lua_pushlightuserdata(state, ((Material *)lua_touserdata(state, 1))->SetEmissionTexture((Texture *)lua_touserdata(state, 2)));

	return 1;
}