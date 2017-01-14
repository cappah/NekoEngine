/* NekoEngine
 *
 * MathInterface.h
 * Author: Alexandru Naiman
 *
 * Math script interface
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

class MathInterface
{
public:
	static void Register(lua_State *state);

	static int vec2_Distance(lua_State *state);
	static int vec2_Dot(lua_State *state);
	static int vec2_Length(lua_State *state);
	static int vec2_Reflect(lua_State *state);
	static int vec2_Refract(lua_State *state);

	static int vec3_Distance(lua_State *state);
	static int vec3_Cross(lua_State *state);
	static int vec3_Dot(lua_State *state);
	static int vec3_Length(lua_State *state);
	static int vec3_Reflect(lua_State *state);
	static int vec3_Refract(lua_State *state);

	static int vec4_Distance(lua_State *state);
	static int vec4_Dot(lua_State *state);
	static int vec4_Length(lua_State *state);
	static int vec4_Reflect(lua_State *state);
	static int vec4_Refract(lua_State *state);

	static int mat2_Identity(lua_State *state);
	static int mat2_Determinant(lua_State *state);
	static int mat2_Inverse(lua_State *state);
	static int mat2_Transpose(lua_State *state);
	static int mat2_Mul(lua_State *state);

	static int mat3_Identity(lua_State *state);
	static int mat3_Determinant(lua_State *state);
	static int mat3_Inverse(lua_State *state);
	static int mat3_Transpose(lua_State *state);
	static int mat3_Mul(lua_State *state);

	static int mat4_Identity(lua_State *state);
	static int mat4_Translate(lua_State *state);
	static int mat4_Rotate(lua_State *state);
	static int mat4_Scale(lua_State *state);
	static int mat4_Determinant(lua_State *state);
	static int mat4_Inverse(lua_State *state);
	static int mat4_Transpose(lua_State *state);
	static int mat4_Mul(lua_State *state);
};