/* NekoEngine
 *
 * SoundManagerInterface.h
 * Author: Alexandru Naiman
 *
 * SoundManager script interface
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

#include <Audio/AudioSystem.h>
#include <Engine/SoundManager.h>
#include <Script/Interface/SoundManagerInterface.h>

void SoundManagerInterface::Register(lua_State *state)
{
	lua_register(state, "SM_SetBackgroundMusic", SetBackgroundMusic);
	lua_register(state, "SM_SetBackgroundMusicVolume", SetBackgroundMusicVolume);

	lua_register(state, "SM_PlayBackgroundMusic", PlayBackgroundMusic);
	lua_register(state, "SM_StopBackgroundMusic", StopBackgroundMusic);

	lua_register(state, "SM_SetListenerPosition", SetListenerPosition);
	lua_register(state, "SM_SetListenerOrientation", SetListenerOrientation);
}

int SoundManagerInterface::SetBackgroundMusic(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	lua_pushboolean(state, SoundManager::SetBackgroundMusic(lua_tostring(state, 1)) == ENGINE_OK);

	return 1;
}

int SoundManagerInterface::SetBackgroundMusicVolume(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
		return luaL_error(state, "Invalid arguments");

	SoundManager::SetBackgroundMusicVolume((float)lua_tonumber(state, 1));

	return 0;
}

int SoundManagerInterface::PlayBackgroundMusic(lua_State *state)
{
	lua_pushboolean(state, SoundManager::PlayBackgroundMusic());
	return 1;
}

int SoundManagerInterface::StopBackgroundMusic(lua_State *state)
{
	SoundManager::StopBackgroundMusic();
	return 0;
}

int SoundManagerInterface::SetListenerPosition(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 3)
		return luaL_error(state, "Invalid arguments");

	glm::vec3 position{ (float)lua_tonumber(state, 1), (float)lua_tonumber(state, 2), (float)lua_tonumber(state, 3) };
	AudioSystem::GetInstance()->SetListenerPosition(position);

	return 0;
}

int SoundManagerInterface::SetListenerOrientation(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 6)
		return luaL_error(state, "Invalid arguments");

	glm::vec3 front{ (float)lua_tonumber(state, 1), (float)lua_tonumber(state, 2), (float)lua_tonumber(state, 3) };
	glm::vec3 up{ (float)lua_tonumber(state, 4), (float)lua_tonumber(state, 5), (float)lua_tonumber(state, 6) };
	AudioSystem::GetInstance()->SetListenerOrientation(front, up);

	return 0;
}