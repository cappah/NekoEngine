/* NekoEngine
 *
 * ScriptComponent.cpp
 * Author: Alexandru Naiman
 *
 * LUA script component
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

#include <Script/Script.h>
#include <System/Logger.h>
#include <System/VFS/VFS.h>
#include <Scene/Components/ScriptComponent.h>

#define SC_COMP_MODULE	"ScriptComponent"

ENGINE_REGISTER_COMPONENT_CLASS(ScriptComponent);

ScriptComponent::ScriptComponent(ComponentInitializer *initializer)
	: ObjectComponent(initializer)
{
	ArgumentMapType::iterator it{};
	const char *ptr{ nullptr };

	_enabled = false;

	if (((it = initializer->arguments.find("script")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_scriptFile = ptr;

	_state = Script::NewState();

	lua_pushlightuserdata(_state, _parent);
	lua_setglobal(_state, "parent");
}

int ScriptComponent::Load()
{
	int ret = ObjectComponent::Load();
	if (ret != ENGINE_OK)
		return ret;

	if (!Script::LoadScript(_state, _scriptFile))
		return ENGINE_FAIL;

	_enabled = true;

	lua_getglobal(_state, "Load");

	if (!lua_isfunction(_state, lua_gettop(_state)))
		return ENGINE_OK;

	if (lua_pcall(_state, 0, 1, 0) && lua_gettop(_state))
	{
		Logger::Log(SC_COMP_MODULE, LOG_CRITICAL, "Failed to execute Load() function of script %s: %s", *_scriptFile, lua_tostring(_state, -1));
		Logger::Log(SC_COMP_MODULE, LOG_DEBUG, "\n%s", *Script::StackDump(_state));
		lua_pop(_state, 1);
		return ENGINE_FAIL;
	}

	ret = (int)lua_tointeger(_state, -1);
	lua_pop(_state, 1);

	return ret;
}

int ScriptComponent::InitializeComponent()
{
	int ret = ObjectComponent::InitializeComponent();
	if (ret != ENGINE_OK)
		return ret;

	lua_getglobal(_state, "InitializeComponent");

	if (!lua_isfunction(_state, lua_gettop(_state)))
	{
		lua_pop(_state, 1);
		return ENGINE_OK;
	}

	if (lua_pcall(_state, 0, 1, 0) && lua_gettop(_state))
	{
		Logger::Log(SC_COMP_MODULE, LOG_CRITICAL, "Failed to execute InitializeComponent() function of script %s: %s", *_scriptFile, lua_tostring(_state, -1));
		Logger::Log(SC_COMP_MODULE, LOG_DEBUG, "\n%s", *Script::StackDump(_state));
		lua_pop(_state, 1);
		return ENGINE_FAIL;
	}

	ret = (int)lua_tointeger(_state, -1);
	lua_pop(_state, 1);

	return ret;
}

void ScriptComponent::Update(double deltaTime) noexcept
{
	if (!_enabled)
		return;

	ObjectComponent::Update(deltaTime);

	lua_getglobal(_state, "Update");

	if (!lua_isfunction(_state, lua_gettop(_state)))
	{
		Logger::Log(SC_COMP_MODULE, LOG_CRITICAL, "Script %s missing Update() function.", *_scriptFile);
		Logger::Log(SC_COMP_MODULE, LOG_DEBUG, "\n%s", *Script::StackDump(_state));
		lua_pop(_state, 1);
		_enabled = false;
		return;
	}

	lua_pushnumber(_state, deltaTime);
	if (lua_pcall(_state, 1, 0, 0) && lua_gettop(_state))
	{
		Logger::Log(SC_COMP_MODULE, LOG_CRITICAL, "Failed to execute Update() function of script %s: %s", *_scriptFile, lua_tostring(_state, -1));
		Logger::Log(SC_COMP_MODULE, LOG_DEBUG, "\n%s", *Script::StackDump(_state));
		lua_pop(_state, 1);
	}
}

void ScriptComponent::UpdatePosition() noexcept
{
	if (!_enabled)
		return;

	ObjectComponent::UpdatePosition();

	lua_getglobal(_state, "UpdatePosition");
	if (!lua_isfunction(_state, lua_gettop(_state)))
	{
		lua_pop(_state, 1);
		return;
	}

	if (lua_pcall(_state, 0, 0, 0) && lua_gettop(_state))
	{
		Logger::Log(SC_COMP_MODULE, LOG_CRITICAL, "Failed to execute UpdatePosition() function of script %s: %s", *_scriptFile, lua_tostring(_state, -1));
		Logger::Log(SC_COMP_MODULE, LOG_DEBUG, "\n%s", *Script::StackDump(_state));
		lua_pop(_state, 1);
	}
}

bool ScriptComponent::Unload()
{
	if (!ObjectComponent::Unload())
		return false;

	lua_getglobal(_state, "Unload");
	if (!lua_isfunction(_state, lua_gettop(_state)))
	{
		lua_pop(_state, 1);
		return true;
	}

	if (lua_pcall(_state, 0, 1, 0) && lua_gettop(_state))
	{
		Logger::Log(SC_COMP_MODULE, LOG_CRITICAL, "Failed to execute Unload() function of script %s: %s", *_scriptFile, lua_tostring(_state, -1));
		Logger::Log(SC_COMP_MODULE, LOG_DEBUG, "\n%s", *Script::StackDump(_state));
		lua_pop(_state, 1);
		return true;
	}

	bool ret = lua_toboolean(_state, -1);
	lua_pop(_state, 1);

	return ret;
}

bool ScriptComponent::CanUnload()
{
	if (!ObjectComponent::CanUnload())
		return false;

	lua_getglobal(_state, "CanUnload");
	if (!lua_isfunction(_state, lua_gettop(_state)))
	{
		lua_pop(_state, 1);
		return true;
	}

	if (lua_pcall(_state, 0, 1, 0) && lua_gettop(_state))
	{
		Logger::Log(SC_COMP_MODULE, LOG_CRITICAL, "Failed to execute CanUnload() function of script %s: %s", *_scriptFile, lua_tostring(_state, -1));
		Logger::Log(SC_COMP_MODULE, LOG_DEBUG, "\n%s", *Script::StackDump(_state));
		lua_pop(_state, 1);
		return true;
	}

	bool ret = lua_toboolean(_state, -1);
	lua_pop(_state, 1);

	return ret;
}

ScriptComponent::~ScriptComponent() noexcept
{
	lua_close(_state);
}
