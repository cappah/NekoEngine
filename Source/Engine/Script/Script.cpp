/* NekoEngine
 *
 * Script.h
 * Author: Alexandru Naiman
 *
 * NekoEngine script system
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

#include <System/Logger.h>
#include <System/VFS/VFS.h>

#include <Script/Script.h>
#include <Script/Interface/VFSInterface.h>
#include <Script/Interface/GUIInterface.h>
#include <Script/Interface/MathInterface.h>
#include <Script/Interface/InputInterface.h>
#include <Script/Interface/DebugInterface.h>
#include <Script/Interface/SceneInterface.h>
#include <Script/Interface/ObjectInterface.h>
#include <Script/Interface/EngineInterface.h>
#include <Script/Interface/LoggerInterface.h>
#include <Script/Interface/CameraInterface.h>
#include <Script/Interface/SystemInterface.h>
#include <Script/Interface/ConsoleInterface.h>
#include <Script/Interface/PlatformInterface.h>
#include <Script/Interface/MaterialInterface.h>
#include <Script/Interface/AudioSourceInterface.h>
#include <Script/Interface/EventManagerInterface.h>
#include <Script/Interface/SoundManagerInterface.h>
#include <Script/Interface/AnimationClipInterface.h>
#include <Script/Interface/CameraManagerInterface.h>
#include <Script/Interface/LightComponentInterface.h>
#include <Script/Interface/ObjectComponentInterface.h>
#include <Script/Interface/ResourceManagerInterface.h>
#include <Script/Interface/CameraComponentInterface.h>
#include <Script/Interface/ScriptComponentInterface.h>
#include <Script/Interface/AnimatorComponentInterface.h>
#include <Script/Interface/StaticMeshComponentInterface.h>
#include <Script/Interface/AudioSourceComponentInterface.h>
#include <Script/Interface/SkeletalMeshComponentInterface.h>

#define SCRIPT_MODULE	"Script"

lua_State *Script::NewState()
{
	lua_State *state = luaL_newstate();
	luaL_openlibs(state);

	PlatformInterface::Register(state);
	EngineInterface::Register(state);
	InputInterface::Register(state);
	GUIInterface::Register(state);
	LoggerInterface::Register(state);
	ObjectInterface::Register(state);
	ObjectComponentInterface::Register(state);
	SceneInterface::Register(state);
	EventManagerInterface::Register(state);
	ResourceManagerInterface::Register(state);
	CameraManagerInterface::Register(state);
	CameraComponentInterface::Register(state);
	CameraInterface::Register(state);
	MathInterface::Register(state);
	ConsoleInterface::Register(state);
	AudioSourceInterface::Register(state);
	AnimationClipInterface::Register(state);
	AnimatorComponentInterface::Register(state);
	AudioSourceComponentInterface::Register(state);
	StaticMeshComponentInterface::Register(state);
	SkeletalMeshComponentInterface::Register(state);
	SoundManagerInterface::Register(state);
	MaterialInterface::Register(state);
	VFSInterface::Register(state);
	LightComponentInterface::Register(state);
	ScriptComponentInterface::Register(state);
	SystemInterface::Register(state);

	#if defined(NE_CONFIG_DEBUG) || defined(NE_CONFIG_DEVELOPMENT)
		DebugInterface::Register(state);
	#endif

	return state;
}

bool Script::LoadScript(lua_State *state, NString &scriptFile)
{
	size_t sz{ 0 };
	VFSFile *file{ VFS::Open(scriptFile) };
	if (!file)
	{
		Logger::Log(SCRIPT_MODULE, LOG_CRITICAL, "Cannot script file %s", *scriptFile);
		return false;
	}

	char *scriptData = (char *)file->ReadAll(sz, true);

	bool ret = LoadSource(state, scriptData);
	free(scriptData);

	file->Close();

	if (!ret)
		Logger::Log(SCRIPT_MODULE, LOG_CRITICAL, "Failed to load script %s", *scriptFile);
	
	return ret;
}

bool Script::LoadSource(lua_State *state, const char *src)
{
	// Data types
	NString script =
		"local ffi = require('ffi')	\n\
		ffi.cdef([[					\n\
		typedef struct				\n\
		{							\n\
			float x, y;				\n\
		} vec2;						\n\
		typedef struct				\n\
		{							\n\
			float x, y, z;			\n\
		} vec3;						\n\
		typedef struct				\n\
		{							\n\
			float x, y, z, w;		\n\
		} vec4;						\n\
		typedef struct				\n\
		{							\n\
			float d[4];				\n\
		} mat2;						\n\
		typedef struct				\n\
		{							\n\
			float d[9];				\n\
		} mat3;						\n\
		typedef struct				\n\
		{							\n\
			float d[16];			\n\
		} mat4;						\n\
		typedef struct				\n\
		{							\n\
			mat4 position;			\n\
			mat4 direction;			\n\
			mat4 color;				\n\
			mat4 data;				\n\
		} Light;					\n\
		typedef struct				\n\
		{							\n\
			int vertexOffset;		\n\
			int vertexCount;		\n\
			int indexOffset;		\n\
			int indexCount;			\n\
		} MeshGroup;				\n\
	]])\n\n";

	script.Append(src);

	if (luaL_dostring(state, *script) && lua_gettop(state))
	{
		Logger::Log(SCRIPT_MODULE, LOG_CRITICAL, "Script load error: %s", lua_tostring(state, -1));
		lua_pop(state, 1);
		return false;
	}

	return true;
}

NString Script::StackDump(lua_State *state)
{
	int top{ lua_gettop(state) };
	NString dump{ "Lua stack dump:\n\n" };
	
	for (int i = 1; i <= top; ++i)
	{
		int t{ lua_type(state, i) };

		dump.AppendFormat(10, "\t%d: ", i);

		switch (t)
		{
			case LUA_TSTRING:
			{
				const char *str = lua_tostring(state, i);
				dump.AppendFormat(strlen(str) + 14, "<string> [%s]\n", str);
			}
			break;
			case LUA_TBOOLEAN:
				dump.AppendFormat(25, "<boolean> [%s]\n", lua_toboolean(state, i) ? "true" : "false");
			break;
			case LUA_TNUMBER:
				dump.AppendFormat(25, "<number> [%g]\n", lua_tonumber(state, i));
			break;
			default:
				dump.AppendFormat(25, "<%s>\n", lua_typename(state, i));
			break;
		}
	}

	dump.Append("\nEnd");

	return dump;
}