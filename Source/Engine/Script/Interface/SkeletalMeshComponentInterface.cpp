/* NekoEngine
 *
 * SkeletalMeshComponentInterface.cpp
 * Author: Alexandru Naiman
 *
 * SkeletalMeshComponent script interface
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

#include <Scene/Components/SkeletalMeshComponent.h>
#include <Script/Interface/SkeletalMeshComponentInterface.h>

using namespace std;

void SkeletalMeshComponentInterface::Register(lua_State *state)
{
	lua_register(state, "SKMC_GetMesh", GetMesh);
	lua_register(state, "SKMC_GetMeshId", GetMeshId);
	
	lua_register(state, "SKMC_GetVertexCount", GetVertexCount);
	lua_register(state, "SKMC_GetTriangleCount", GetTriangleCount);

	lua_register(state, "SKMC_AddGroup", AddGroup);
	lua_register(state, "SKMC_ResetGroups", ResetGroups);

	/*lua_register(state, "SKMC_LoadStatic", LoadStatic);
	lua_register(state, "SKMC_LoadDynamic", LoadDynamic);*/
}

int SkeletalMeshComponentInterface::GetMesh(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushlightuserdata(state, ((SkeletalMeshComponent *)lua_touserdata(state, 1))->GetMesh());

	return 1;
}

int SkeletalMeshComponentInterface::GetMeshId(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushstring(state, *((SkeletalMeshComponent *)lua_touserdata(state, 1))->GetMeshID());

	return 1;
}
	
int SkeletalMeshComponentInterface::GetVertexCount(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushinteger(state, ((SkeletalMeshComponent *)lua_touserdata(state, 1))->GetVertexCount());

	return 1;
}

int SkeletalMeshComponentInterface::GetTriangleCount(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushinteger(state, ((SkeletalMeshComponent *)lua_touserdata(state, 1))->GetTriangleCount());

	return 1;
}

int SkeletalMeshComponentInterface::LoadStatic(lua_State *state)
{
	/*int args{ lua_gettop(state) };
	bool createGroup{ false };

	if (args < 3)
		return luaL_error(state, "Invalid arguments");

	if (args == 4) createGroup = lua_toboolean(state, 4);

	NArray<SkeletalVertex> *vertices = (NArray<SkeletalVertex> *)lua_touserdata(state, 2);
	NArray<uint32_t> *indices = (NArray<uint32_t>*)lua_touserdata(state, 3);

	lua_pushboolean(state, ((SkeletalMeshComponent *)lua_touserdata(state, 1))->LoadStatic(vertices, indices, createGroup) == ENGINE_OK);

	return 1;*/
	return 0;
}

int SkeletalMeshComponentInterface::LoadDynamic(lua_State *state)
{
	return 0;
}

int SkeletalMeshComponentInterface::AddGroup(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 4)
		return luaL_error(state, "Invalid arguments");

	((SkeletalMeshComponent *)lua_touserdata(state, 1))->AddGroup((uint32_t)lua_tointeger(state, 2), (uint32_t)lua_tointeger(state, 3), (Material *)lua_touserdata(state, 4));

	return 0;
}

int SkeletalMeshComponentInterface::ResetGroups(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	((SkeletalMeshComponent *)lua_touserdata(state, 1))->ResetGroups();

	return 0;
}