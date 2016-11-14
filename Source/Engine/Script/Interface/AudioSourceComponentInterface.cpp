/* NekoEngine
 *
 * AudioSourceComponentInterface.cpp
 * Author: Alexandru Naiman
 *
 * AudioSourceComponent script interface
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

#include <Scene/Components/AudioSourceComponent.h>
#include <Script/Interface/AudioSourceComponentInterface.h>

void AudioSourceComponentInterface::Register(lua_State *state)
{
	lua_register(state, "ASC_PlayDefaultClip", PlayDefaultClip);
	lua_register(state, "ASC_PlayClip", PlayClip);
}

int AudioSourceComponentInterface::PlayDefaultClip(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	((AudioSourceComponent *)lua_touserdata(state, 1))->PlayDefaultClip();

	return 0;
}

int AudioSourceComponentInterface::PlayClip(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 2)
		return luaL_error(state, "Invalid arguments");

	((AudioSourceComponent *)lua_touserdata(state, 1))->PlayClip((AudioClip *)lua_touserdata(state, 2));

	return 0;
}