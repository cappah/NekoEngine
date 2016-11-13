/* NekoEngine
 *
 * MathInterface.h
 * Author: Alexandru Naiman
 *
 * Math script interface
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

#define _USE_MATH_DEFINES
#include <math.h>

#include <Engine/Engine.h>
#include <Script/Interface/MathInterface.h>

using namespace glm;

void MathInterface::Register(lua_State *state)
{
	lua_pushnumber(state, M_E);
	lua_setglobal(state, "M_E");

	lua_pushnumber(state, M_LOG2E);
	lua_setglobal(state, "M_LOG2E");

	lua_pushnumber(state, M_LOG10E);
	lua_setglobal(state, "M_LOG10E");

	lua_pushnumber(state, M_LN2);
	lua_setglobal(state, "M_LN2");

	lua_pushnumber(state, M_LN10);
	lua_setglobal(state, "M_LN10");

	lua_pushnumber(state, M_PI);
	lua_setglobal(state, "M_PI");

	lua_pushnumber(state, M_PI_2);
	lua_setglobal(state, "M_PI_2");

	lua_pushnumber(state, M_PI_4);
	lua_setglobal(state, "M_PI_4");

	lua_pushnumber(state, M_1_PI);
	lua_setglobal(state, "M_1_PI");

	lua_pushnumber(state, M_2_PI);
	lua_setglobal(state, "M_2_PI");

	lua_pushnumber(state, M_2_SQRTPI);
	lua_setglobal(state, "M_2_SQRTPI");

	lua_pushnumber(state, M_SQRT2);
	lua_setglobal(state, "M_SQRT2");

	lua_pushnumber(state, M_SQRT1_2);
	lua_setglobal(state, "M_SQRT1_2");

	lua_pushnumber(state, HUGE);
	lua_setglobal(state, "M_HUGE");

	lua_pushnumber(state, HUGE_VAL);
	lua_setglobal(state, "M_HUGE_VAL");

	lua_register(state, "M_vec2_Distance", vec2_Distance);
	lua_register(state, "M_vec2_Dot", vec2_Dot);
	lua_register(state, "M_vec2_Length", vec2_Length);
	lua_register(state, "M_vec2_Reflect", vec2_Reflect);
	lua_register(state, "M_vec2_Refract", vec2_Refract);

	lua_register(state, "M_vec3_Distance", vec3_Distance);
	lua_register(state, "M_vec3_Cross", vec3_Cross);
	lua_register(state, "M_vec3_Dot", vec3_Dot);
	lua_register(state, "M_vec3_Length", vec3_Length);
	lua_register(state, "M_vec3_Reflect", vec3_Reflect);
	lua_register(state, "M_vec3_Refract", vec3_Refract);

	lua_register(state, "M_vec4_Distance", vec4_Distance);
	lua_register(state, "M_vec4_Dot", vec4_Dot);
	lua_register(state, "M_vec4_Length", vec4_Length);
	lua_register(state, "M_vec4_Reflect", vec4_Reflect);
	lua_register(state, "M_vec4_Refract", vec4_Refract);

	lua_register(state, "M_mat2_Identity", mat2_Identity);
	lua_register(state, "M_mat2_Determinant", mat2_Determinant);
	lua_register(state, "M_mat2_Inverse", mat2_Inverse);
	lua_register(state, "M_mat2_Transpose", mat2_Transpose);
	lua_register(state, "M_mat2_Mul", mat2_Mul);

	lua_register(state, "M_mat3_Identity", mat3_Identity);
	lua_register(state, "M_mat3_Determinant", mat3_Determinant);
	lua_register(state, "M_mat3_Inverse", mat3_Inverse);
	lua_register(state, "M_mat3_Transpose", mat3_Transpose);
	lua_register(state, "M_mat3_Mul", mat3_Mul);

	lua_register(state, "M_mat4_Identity", mat4_Identity);
	lua_register(state, "M_mat4_Translate", mat4_Translate);
	lua_register(state, "M_mat4_Rotate", mat4_Rotate);
	lua_register(state, "M_mat4_Scale", mat4_Scale);
	lua_register(state, "M_mat4_Determinant", mat4_Determinant);
	lua_register(state, "M_mat4_Inverse", mat4_Inverse);
	lua_register(state, "M_mat4_Transpose", mat4_Transpose);
	lua_register(state, "M_mat4_Mul", mat4_Mul);
}

int MathInterface::vec2_Distance(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f1{ (float *)lua_touserdata(state, 1) };
	float *f2{ (float *)lua_touserdata(state, 2) };
	vec2 v1{ f1[0], f1[1] };
	vec2 v2{ f2[0], f2[1] };
	float d = distance(v1, v2);

	lua_pushnumber(state, d);

	return 1;
}

int MathInterface::vec2_Dot(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f1{ (float *)lua_touserdata(state, 1) };
	float *f2{ (float *)lua_touserdata(state, 2) };
	vec2 v1{ f1[0], f1[1] };
	vec2 v2{ f2[0], f2[1] };
	float d = dot(v1, v2);

	lua_pushnumber(state, d);

	return 1;
}

int MathInterface::vec2_Length(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f{ (float *)lua_touserdata(state, 1) };
	vec2 v{ f[0], f[1] };
	float d = length(v);

	lua_pushnumber(state, d);

	return 1;
}

int MathInterface::vec2_Reflect(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 3)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f1{ (float *)lua_touserdata(state, 1) };
	float *f2{ (float *)lua_touserdata(state, 2) };
	vec2 v1{ f1[0], f1[1] };
	vec2 v2{ f2[0], f2[1] };
	vec2 r = reflect(v1, v2);

	float *out{ (float *)lua_touserdata(state, 3) };
	memcpy(out, value_ptr(r), sizeof(vec2));

	return 0;
}

int MathInterface::vec2_Refract(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 4)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f1{ (float *)lua_touserdata(state, 1) };
	float *f2{ (float *)lua_touserdata(state, 2) };
	vec2 v1{ f1[0], f1[1] };
	vec2 v2{ f2[0], f2[1] };
	vec2 r = refract(v1, v2, (float)lua_tonumber(state, 3));

	float *out{ (float *)lua_touserdata(state, 4) };
	memcpy(out, value_ptr(r), sizeof(vec2));

	return 0;
}

int MathInterface::vec3_Distance(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f1{ (float *)lua_touserdata(state, 1) };
	float *f2{ (float *)lua_touserdata(state, 2) };
	vec3 v1{ f1[0], f1[1], f1[2] };
	vec3 v2{ f2[0], f2[1], f2[2] };
	float d = distance(v1, v2);

	lua_pushnumber(state, d);

	return 1;
}

int MathInterface::vec3_Cross(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 3)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f1{ (float *)lua_touserdata(state, 1) };
	float *f2{ (float *)lua_touserdata(state, 2) };
	vec3 v1{ f1[0], f1[1], f1[2] };
	vec3 v2{ f2[0], f2[1], f2[2] };
	vec3 r = cross(v1, v2);

	float *out{ (float *)lua_touserdata(state, 3) };
	memcpy(out, value_ptr(r), sizeof(vec3));

	return 0;
}

int MathInterface::vec3_Dot(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f1{ (float *)lua_touserdata(state, 1) };
	float *f2{ (float *)lua_touserdata(state, 2) };
	vec3 v1{ f1[0], f1[1], f1[2] };
	vec3 v2{ f2[0], f2[1], f2[2] };
	float d = dot(v1, v2);

	lua_pushnumber(state, d);

	return 1;
}

int MathInterface::vec3_Length(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f{ (float *)lua_touserdata(state, 1) };
	vec3 v{ f[0], f[1], f[2] };
	float d = length(v);

	lua_pushnumber(state, d);

	return 1;
}

int MathInterface::vec3_Reflect(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 3)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f1{ (float *)lua_touserdata(state, 1) };
	float *f2{ (float *)lua_touserdata(state, 2) };
	vec3 v1{ f1[0], f1[1], f1[2] };
	vec3 v2{ f2[0], f2[1], f2[2] };
	vec3 r = reflect(v1, v2);

	float *out{ (float *)lua_touserdata(state, 3) };
	memcpy(out, value_ptr(r), sizeof(vec3));

	return 0;
}

int MathInterface::vec3_Refract(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 4)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f1{ (float *)lua_touserdata(state, 1) };
	float *f2{ (float *)lua_touserdata(state, 2) };
	vec3 v1{ f1[0], f1[1], f1[2] };
	vec3 v2{ f2[0], f2[1], f2[2] };
	vec3 r = refract(v1, v2, (float)lua_tonumber(state, 3));

	float *out{ (float *)lua_touserdata(state, 4) };
	memcpy(out, value_ptr(r), sizeof(vec3));

	return 0;
}

int MathInterface::vec4_Distance(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f1{ (float *)lua_touserdata(state, 1) };
	float *f2{ (float *)lua_touserdata(state, 2) };
	vec4 v1{ f1[0], f1[1], f1[2], f1[3] };
	vec4 v2{ f2[0], f2[1], f2[2], f2[3] };
	float d = distance(v1, v2);

	lua_pushnumber(state, d);

	return 1;
}

int MathInterface::vec4_Dot(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f1{ (float *)lua_touserdata(state, 1) };
	float *f2{ (float *)lua_touserdata(state, 2) };
	vec4 v1{ f1[0], f1[1], f1[2], f1[3] };
	vec4 v2{ f2[0], f2[1], f2[2], f2[3] };
	float d = dot(v1, v2);

	lua_pushnumber(state, d);

	return 1;
}

int MathInterface::vec4_Length(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f{ (float *)lua_touserdata(state, 1) };
	vec4 v{ f[0], f[1], f[2], f[3] };
	float d = length(v);

	lua_pushnumber(state, d);

	return 1;
}

int MathInterface::vec4_Reflect(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 3)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f1{ (float *)lua_touserdata(state, 1) };
	float *f2{ (float *)lua_touserdata(state, 2) };
	vec4 v1{ f1[0], f1[1], f1[2], f1[3] };
	vec4 v2{ f2[0], f2[1], f2[2], f2[3] };
	vec4 r = reflect(v1, v2);
	
	float *out{ (float *)lua_touserdata(state, 3) };
	memcpy(out, value_ptr(r), sizeof(vec4));

	return 0;
}

int MathInterface::vec4_Refract(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 4)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f1{ (float *)lua_touserdata(state, 1) };
	float *f2{ (float *)lua_touserdata(state, 2) };
	vec4 v1{ f1[0], f1[1], f1[2], f1[3] };
	vec4 v2{ f2[0], f2[1], f2[2], f2[3] };
	vec4 r = refract(v1, v2, (float)lua_tonumber(state, 3));

	float *out{ (float *)lua_touserdata(state, 4) };
	memcpy(out, value_ptr(r), sizeof(vec4));

	return 0;
}

int MathInterface::mat2_Identity(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	mat2 r = mat2();

	float *out{ (float *)lua_touserdata(state, 1) };
	memcpy(out, value_ptr(r), sizeof(mat2));

	return 0;
}

int MathInterface::mat2_Determinant(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f{ (float *)lua_touserdata(state, 1) };
	mat2 m = make_mat2(f);

	lua_pushnumber(state, determinant(m));

	return 1;
}

int MathInterface::mat2_Inverse(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *in{ (float *)lua_touserdata(state, 1) };
	mat2 m = make_mat2(in);
	mat2 r = inverse(m);
	
	float *out{ (float *)lua_touserdata(state, 2) };
	memcpy(out, value_ptr(r), sizeof(mat2));

	return 0;
}

int MathInterface::mat2_Transpose(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *in{ (float *)lua_touserdata(state, 1) };
	mat2 m = make_mat2(in);
	mat2 r = transpose(m);

	float *out{ (float *)lua_touserdata(state, 2) };
	memcpy(out, value_ptr(r), sizeof(mat2));

	return 0;
}

int MathInterface::mat2_Mul(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 3)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *inMat{ (float *)lua_touserdata(state, 1) };
	float *inVec{ (float *)lua_touserdata(state, 2) };
	mat2 m = make_mat2(inMat);
	vec2 v{ inVec[0], inVec[1] };
	vec2 r = m * v;

	float *out{ (float *)lua_touserdata(state, 3) };
	memcpy(out, value_ptr(r), sizeof(vec2));

	return 0;
}

int MathInterface::mat3_Identity(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	mat3 r = mat3();

	float *out{ (float *)lua_touserdata(state, 1) };
	memcpy(out, value_ptr(r), sizeof(mat3));

	return 0;
}

int MathInterface::mat3_Determinant(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f{ (float *)lua_touserdata(state, 1) };
	mat3 m = make_mat3(f);

	lua_pushnumber(state, determinant(m));

	return 1;
}

int MathInterface::mat3_Inverse(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *in{ (float *)lua_touserdata(state, 1) };
	mat3 m = make_mat3(in);
	mat3 r = inverse(m);

	float *out{ (float *)lua_touserdata(state, 2) };
	memcpy(out, value_ptr(r), sizeof(mat3));

	return 0;
}

int MathInterface::mat3_Transpose(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *in{ (float *)lua_touserdata(state, 1) };
	mat3 m = make_mat3(in);
	mat3 r = transpose(m);

	float *out{ (float *)lua_touserdata(state, 2) };
	memcpy(out, value_ptr(r), sizeof(mat3));

	return 0;
}

int MathInterface::mat3_Mul(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 3)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *inMat{ (float *)lua_touserdata(state, 1) };
	float *inVec{ (float *)lua_touserdata(state, 2) };
	mat3 m = make_mat3(inMat);
	vec3 v{ inVec[0], inVec[1], inVec[2] };
	vec3 r = m * v;

	float *out{ (float *)lua_touserdata(state, 3) };
	memcpy(out, value_ptr(r), sizeof(vec3));

	return 0;
}

int MathInterface::mat4_Identity(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	mat4 r = mat4();

	float *out{ (float *)lua_touserdata(state, 1) };
	memcpy(out, value_ptr(r), sizeof(mat4));

	return 0;
}

int MathInterface::mat4_Translate(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 3)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *in{ (float *)lua_touserdata(state, 1) };
	float *inVec{ (float *)lua_touserdata(state, 2) };
	mat4 m = make_mat4(in);
	vec3 v = vec3(inVec[0], inVec[1], inVec[2]);
	mat4 r = translate(m, v);

	float *out{ (float *)lua_touserdata(state, 3) };
	memcpy(out, value_ptr(r), sizeof(mat4));

	return 0;
}

int MathInterface::mat4_Rotate(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 4)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *in{ (float *)lua_touserdata(state, 1) };
	float *inVec{ (float *)lua_touserdata(state, 3) };
	mat4 m = make_mat4(in);
	vec3 v = vec3(inVec[0], inVec[1], inVec[2]);
	mat4 r = rotate(m, (float)lua_tonumber(state, 2), v);

	float *out{ (float *)lua_touserdata(state, 4) };
	memcpy(out, value_ptr(r), sizeof(mat4));

	return 0;
}

int MathInterface::mat4_Scale(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 3)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *in{ (float *)lua_touserdata(state, 1) };
	float *inVec{ (float *)lua_touserdata(state, 2) };
	mat4 m = make_mat4(in);
	vec3 v = vec3(inVec[0], inVec[1], inVec[2]);
	mat4 r = scale(m, v);

	float *out{ (float *)lua_touserdata(state, 3) };
	memcpy(out, value_ptr(r), sizeof(mat4));

	return 0;
}

int MathInterface::mat4_Determinant(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 1)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *f{ (float *)lua_touserdata(state, 1) };
	mat4 m = make_mat4(f);

	lua_pushnumber(state, determinant(m));

	return 1;
}

int MathInterface::mat4_Inverse(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *in{ (float *)lua_touserdata(state, 1) };
	mat4 m = make_mat4(in);
	mat4 r = inverse(m);

	float *out{ (float *)lua_touserdata(state, 2) };
	memcpy(out, value_ptr(r), sizeof(mat4));

	return 0;
}

int MathInterface::mat4_Transpose(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 2)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *in{ (float *)lua_touserdata(state, 1) };
	mat4 m = make_mat4(in);
	mat4 r = transpose(m);

	float *out{ (float *)lua_touserdata(state, 2) };
	memcpy(out, value_ptr(r), sizeof(mat4));

	return 0;
}

int MathInterface::mat4_Mul(lua_State *state)
{
	int argc{ lua_gettop(state) };

	if (argc != 3)
	{
		lua_pushboolean(state, false);
		return luaL_error(state, "Invalid arguments");
	}

	float *inMat{ (float *)lua_touserdata(state, 1) };
	float *inVec{ (float *)lua_touserdata(state, 2) };
	mat4 m = make_mat4(inMat);
	vec4 v{ inVec[0], inVec[1], inVec[2], inVec[3] };
	vec4 r = m * v;

	float *out{ (float *)lua_touserdata(state, 3) };
	memcpy(out, value_ptr(r), sizeof(vec4));

	return 0;
}