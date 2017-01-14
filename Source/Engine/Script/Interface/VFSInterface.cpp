/* NekoEngine
 *
 * VFSInterface.h
 * Author: Alexandru Naiman
 *
 * VFS script interface
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

#include <System/VFS/VFS.h>
#include <Script/Interface/VFSInterface.h>

void VFSInterface::Register(lua_State *state)
{
	lua_pushinteger(state, SEEK_SET);
	lua_setglobal(state, "SEEK_SET");
	
	lua_pushinteger(state, SEEK_CUR);
	lua_setglobal(state, "SEEK_CUR");

	lua_pushinteger(state, SEEK_END);
	lua_setglobal(state, "SEEK_END");

	lua_register(state, "VFS_Open", Open);
	lua_register(state, "VFS_LoadArchive", LoadArchive);

	lua_register(state, "VFile_Open", F_Open);
	lua_register(state, "VFile_IsOpen", F_IsOpen);
	lua_register(state, "VFile_Read", F_Read);
	lua_register(state, "VFile_Gets", F_Gets);
	lua_register(state, "VFile_Seek", F_Seek);
	lua_register(state, "VFile_Tell", F_Tell);
	lua_register(state, "VFile_EoF", F_EoF);
	lua_register(state, "VFile_Close", F_Close);

	lua_register(state, "VArchive_MakeResident", A_MakeResident);
	lua_register(state, "VArchive_MakeNonResident", A_MakeNonResident);
	lua_register(state, "VArchive_IsResident", A_IsResident);
	lua_register(state, "VArchive_Read", A_Read);
	lua_register(state, "VArchive_Open", A_Open);
}

int VFSInterface::Open(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	NString str = lua_tostring(state, 1);
	lua_pushlightuserdata(state, VFS::Open(str));

	return 1;
}

int VFSInterface::LoadArchive(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	NString str = lua_tostring(state, 1);
	lua_pushboolean(state, VFS::LoadArchive(str) == ENGINE_OK);

	return 1;
}

int VFSInterface::F_Open(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushboolean(state, ((VFSFile *)lua_touserdata(state, 1))->Open() == ENGINE_OK);

	return 1;
}

int VFSInterface::F_IsOpen(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushboolean(state, ((VFSFile *)lua_touserdata(state, 1))->IsOpen());

	return 1;
}

int VFSInterface::F_Read(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 4)
		return luaL_error(state, "Invalid arguments");

	lua_pushinteger(state, ((VFSFile *)lua_touserdata(state, 1))->Read(lua_touserdata(state, 2), (size_t)lua_tointeger(state, 3), (size_t)lua_tointeger(state, 4)));

	return 1;
}

int VFSInterface::F_Gets(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 2)
		return luaL_error(state, "Invalid arguments");

	int size = (int)lua_tointeger(state, 2);
	char *str = (char *)calloc(size + 1, sizeof(char));
	((VFSFile *)lua_touserdata(state, 1))->Gets(str, size);
	lua_pushstring(state, str);
	free(str);

	return 1;
}

int VFSInterface::F_Seek(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 3)
		return luaL_error(state, "Invalid arguments");

	lua_pushboolean(state, !((VFSFile *)lua_touserdata(state, 1))->Seek(lua_tointeger(state, 2), (int)lua_tointeger(state, 3)));

	return 1;
}

int VFSInterface::F_Tell(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushinteger(state, ((VFSFile *)lua_touserdata(state, 1))->Tell());

	return 1;
}

int VFSInterface::F_EoF(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushboolean(state, !((VFSFile *)lua_touserdata(state, 1))->EoF());

	return 1;
}

int VFSInterface::F_Close(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	((VFSFile *)lua_touserdata(state, 1))->Close();

	return 0;
}

int VFSInterface::A_MakeResident(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushboolean(state, ((VFSArchive *)lua_touserdata(state, 1))->MakeResident() == ENGINE_OK);

	return 1;
}

int VFSInterface::A_MakeNonResident(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	((VFSArchive *)lua_touserdata(state, 1))->MakeNonResident();

	return 0;
}

int VFSInterface::A_IsResident(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushboolean(state, ((VFSArchive *)lua_touserdata(state, 1))->IsResident());

	return 1;
}

int VFSInterface::A_Read(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 5)
		return luaL_error(state, "Invalid arguments");

	lua_pushinteger(state, ((VFSArchive *)lua_touserdata(state, 1))->Read(lua_touserdata(state, 2), (size_t)lua_tointeger(state, 3), (size_t)lua_tointeger(state, 4), (size_t)lua_tointeger(state, 5)));

	return 1;
}

int VFSInterface::A_Open(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 2)
		return luaL_error(state, "Invalid arguments");

	NString str = lua_tostring(state, 2);
	lua_pushlightuserdata(state, ((VFSArchive *)lua_touserdata(state, 1))->Open(str));

	return 1;
}