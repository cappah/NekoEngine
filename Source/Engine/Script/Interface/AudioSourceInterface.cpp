/* NekoEngine
 *
 * AudioSourceInterface.cpp
 * Author: Alexandru Naiman
 *
 * AudioSource script interface
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

#include <Audio/AudioSource.h>
#include <Script/Interface/AudioSourceInterface.h>

void AudioSourceInterface::Register(lua_State *state)
{
	lua_register(state, "AS_HasClip", HasClip);

	lua_register(state, "AS_SetPitch", SetPitch);
	lua_register(state, "AS_SetGain", SetGain);

	lua_register(state, "AS_SetConeInnerAngle", SetConeInnerAngle);
	lua_register(state, "AS_SetConeOuterAngle", SetConeOuterAngle);
	lua_register(state, "AS_SetConeOuterGain", SetConeOuterGain);

	lua_register(state, "AS_SetDirection", SetDirection);
	lua_register(state, "AS_SetPosition", SetPosition);
	lua_register(state, "AS_SetVelocity", SetVelocity);

	lua_register(state, "AS_SetLooping", SetLooping);

	lua_register(state, "AS_SetMaxDistance", SetMaxDistance);
	lua_register(state, "AS_SetReferenceDistance", SetReferenceDistance);

	lua_register(state, "AS_SetClip", SetClip);

	lua_register(state, "AS_Play", Play);
	lua_register(state, "AS_Pause", Pause);
	lua_register(state, "AS_Stop", Stop);
	lua_register(state, "AS_Rewind", Rewind);
	lua_register(state, "AS_IsPlaying", IsPlaying);
}

int AudioSourceInterface::HasClip(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushboolean(state, ((AudioSource *)lua_touserdata(state, 1))->HasClip());

	return 1;
}

int AudioSourceInterface::SetPitch(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 2)
		return luaL_error(state, "Invalid arguments");

	((AudioSource *)lua_touserdata(state, 1))->SetPitch((float)lua_tonumber(state, 2));

	return 0;
}

int AudioSourceInterface::SetGain(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 2)
		return luaL_error(state, "Invalid arguments");

	((AudioSource *)lua_touserdata(state, 1))->SetGain((float)lua_tonumber(state, 2));

	return 0;
}

int AudioSourceInterface::SetConeInnerAngle(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 2)
		return luaL_error(state, "Invalid arguments");

	((AudioSource *)lua_touserdata(state, 1))->SetConeInnerAngle((float)lua_tonumber(state, 2));

	return 0;
}

int AudioSourceInterface::SetConeOuterAngle(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 2)
		return luaL_error(state, "Invalid arguments");

	((AudioSource *)lua_touserdata(state, 1))->SetConeOuterAngle((float)lua_tonumber(state, 2));

	return 0;
}

int AudioSourceInterface::SetConeOuterGain(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 2)
		return luaL_error(state, "Invalid arguments");

	((AudioSource *)lua_touserdata(state, 1))->SetConeOuterGain((float)lua_tonumber(state, 2));

	return 0;
}

int AudioSourceInterface::SetDirection(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 4)
		return luaL_error(state, "Invalid arguments");

	((AudioSource *)lua_touserdata(state, 1))->SetDirection((float)lua_tonumber(state, 2), (float)lua_tonumber(state, 3), (float)lua_tonumber(state, 4));

	return 0;
}

int AudioSourceInterface::SetPosition(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 4)
		return luaL_error(state, "Invalid arguments");

	((AudioSource *)lua_touserdata(state, 1))->SetPosition((float)lua_tonumber(state, 2), (float)lua_tonumber(state, 3), (float)lua_tonumber(state, 4));

	return 0;
}

int AudioSourceInterface::SetVelocity(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 4)
		return luaL_error(state, "Invalid arguments");

	((AudioSource *)lua_touserdata(state, 1))->SetVelocity((float)lua_tonumber(state, 2), (float)lua_tonumber(state, 3), (float)lua_tonumber(state, 4));

	return 0;
}

int AudioSourceInterface::SetLooping(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 2)
		return luaL_error(state, "Invalid arguments");

	((AudioSource *)lua_touserdata(state, 1))->SetLooping(lua_toboolean(state, 2));

	return 0;
}

int AudioSourceInterface::SetMaxDistance(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 2)
		return luaL_error(state, "Invalid arguments");

	((AudioSource *)lua_touserdata(state, 1))->SetMaxDistance((float)lua_tonumber(state, 2));

	return 0;
}

int AudioSourceInterface::SetReferenceDistance(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 2)
		return luaL_error(state, "Invalid arguments");

	((AudioSource *)lua_touserdata(state, 1))->SetReferenceDistance((float)lua_tonumber(state, 2));

	return 0;
}

int AudioSourceInterface::SetClip(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 2)
		return luaL_error(state, "Invalid arguments");

	lua_pushboolean(state, ((AudioSource *)lua_touserdata(state, 1))->SetClip((AudioClip *)lua_touserdata(state, 2)) == ENGINE_OK);

	return 1;
}

int AudioSourceInterface::Play(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushboolean(state, ((AudioSource *)lua_touserdata(state, 1))->Play());

	return 1;
}

int AudioSourceInterface::Pause(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	((AudioSource *)lua_touserdata(state, 1))->Pause();

	return 0;
}

int AudioSourceInterface::Stop(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	((AudioSource *)lua_touserdata(state, 1))->Stop();

	return 0;
}

int AudioSourceInterface::Rewind(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	((AudioSource *)lua_touserdata(state, 1))->Rewind();

	return 0;
}


int AudioSourceInterface::IsPlaying(lua_State *state)
{
	int args{ lua_gettop(state) };

	if (args != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushboolean(state, ((AudioSource *)lua_touserdata(state, 1))->IsPlaying());

	return 1;
}