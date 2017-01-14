/* NekoEngine
 *
 * AnimationClipInterface.cpp
 * Author: Alexandru Naiman
 *
 * AnimationClip script interface
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

#include <Animation/AnimationClip.h>
#include <Script/Interface/AnimationClipInterface.h>

void AnimationClipInterface::Register(lua_State *state)
{
	lua_register(state, "AnimClip_GetName", GetName);
	lua_register(state, "AnimClip_GetDuration", GetDuration);
	lua_register(state, "AnimClip_GetTicksPerSecond", GetTicksPerSecond);
}

int AnimationClipInterface::GetName(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushstring(state, ((AnimationClip *)lua_touserdata(state, 1))->GetName().c_str());

	return 1;
}

int AnimationClipInterface::GetDuration(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushnumber(state, ((AnimationClip *)lua_touserdata(state, 1))->GetDuration());

	return 1;
}

int AnimationClipInterface::GetTicksPerSecond(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushnumber(state, ((AnimationClip *)lua_touserdata(state, 1))->GetTicksPerSecond());

	return 1;
}