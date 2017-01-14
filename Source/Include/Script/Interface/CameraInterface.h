/* NekoEngine
 *
 * CameraInterface.h
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

#pragma once

#include <Script/Script.h>

class CameraInterface
{
public:
	static void Register(lua_State *state);

	static int GetForward(lua_State *state);
	static int GetRight(lua_State *state);
	static int GetPosition(lua_State *state);
	static int GetRotation(lua_State *state);
	static int GetViewMatrix(lua_State *state);
	static int GetProjectionMatrix(lua_State *state);
	static int GetNear(lua_State *state);
	static int GetFar(lua_State *state);
	static int GetViewDistance(lua_State *state);
	static int GetFogDistance(lua_State *state);
	static int GetFogColor(lua_State *state);

	static int SetPosition(lua_State *state);
	static int SetRotation(lua_State *state);
	static int SetNear(lua_State *state);
	static int SetFar(lua_State *state);
	static int SetViewDistance(lua_State *state);
	static int SetFogDistance(lua_State *state);
	static int SetFogColor(lua_State *state);

	static int EnableSkybox(lua_State *state);
	static int LookAt(lua_State *state);

	static int MoveForward(lua_State *state);
	static int MoveRight(lua_State *state);
	static int MoveUp(lua_State *state);

	static int RotateX(lua_State *state);
	static int RotateY(lua_State *state);
	static int RotateZ(lua_State *state);
};