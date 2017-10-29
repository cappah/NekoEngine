/* DungeonGame
 *
 * ScriptHost.cpp
 * Author: Alexandru Naiman
 *
 * ScriptHost class
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
#include <Scene/ScriptHost.h>

using namespace glm;
using namespace std;

#define SH_MODULE	"ScriptHost"

ENGINE_REGISTER_OBJECT_CLASS(ScriptHost);

ScriptHost::ScriptHost(ObjectInitializer *initializer) :
	Object(initializer),
	_sc(nullptr)
{
	ArgumentMapType::iterator it{};
	const char *ptr{ nullptr };

	_scriptFile = "";
	if (((it = initializer->arguments.find("script")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_scriptFile = ptr;

	_scriptPtr = "";
	if (((it = initializer->arguments.find("scriptptr")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_scriptPtr = ptr;
}

int ScriptHost::Load()
{
	ComponentInitializer scInitializer{};
	scInitializer.parent = this;
	scInitializer.position = vec3(0.f, 0.f, 0.f);
	scInitializer.rotation = vec3(0.f);
	scInitializer.scale = vec3(1.f);

	if (_scriptFile.Length())
		scInitializer.arguments.insert({ "script", *_scriptFile });
	else
		scInitializer.arguments.insert({ "scriptptr", *_scriptPtr });

	_sc = (ScriptComponent *)Engine::NewComponent("ScriptComponent", &scInitializer);
	if (!_sc) {
		Logger::Log(SH_MODULE, LOG_CRITICAL, "Failed to create ScriptComponent");
		return ENGINE_FAIL;
	}

	for (const pair<string, int> &kvp : _initialGlobals)
		_sc->SetGlobalInteger(kvp.first.c_str(), kvp.second);

	if (_sc->Load() != ENGINE_OK)
		return ENGINE_FAIL;

	AddComponent("Script", _sc);

	return Object::Load();
}

void ScriptHost::SetGlobalInteger(const char *name, int value)
{
	if (!_sc) {
		_initialGlobals.insert({ name, value });
		return;
	}

	_sc->SetGlobalInteger(name, value);
}

ScriptHost::~ScriptHost()
{
}