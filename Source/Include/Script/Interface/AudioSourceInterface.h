/* NekoEngine
 *
 * AudioSourceInterface.h
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

#pragma once

#include <Script/Script.h>

class AudioSourceInterface
{
public:
	static void Register(lua_State *state);

	static int HasClip(lua_State *state);

	static int SetPitch(lua_State *state);
	static int SetGain(lua_State *state);

	static int SetConeInnerAngle(lua_State *state);
	static int SetConeOuterAngle(lua_State *state);
	static int SetConeOuterGain(lua_State *state);

	static int SetDirection(lua_State *state);
	static int SetPosition(lua_State *state);
	static int SetVelocity(lua_State *state);

	static int SetLooping(lua_State *state);

	static int SetMaxDistance(lua_State *state);
	static int SetReferenceDistance(lua_State *state);

	static int SetClip(lua_State *state);

	static int Play(lua_State *state);
	static int Pause(lua_State *state);
	static int Stop(lua_State *state);
	static int Rewind(lua_State *state);
	static int IsPlaying(lua_State *state);
};