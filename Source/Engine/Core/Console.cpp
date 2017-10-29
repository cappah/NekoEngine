/* NekoEngine
 *
 * Console.cpp
 * Author: Alexandru Naiman
 *
 * NekoEngine console
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

#include <stack>

#include <GUI/GUI.h>
#include <Input/Input.h>
#include <Input/Keycodes.h>
#include <Engine/Engine.h>
#include <Engine/Console.h>
#include <System/Logger.h>
#include <Script/Script.h>

#define MAX_SCREEN_LINES	15
#define CON_MODULE		"Console"

using namespace std;
using namespace glm;

bool Console::_open;
NArray<NString> Console::_text;
map<NString, Console::CVarFuncs> Console::_vars;
static NArray<NString> _commandHistory;
static uint64_t _historyId = 0;

static lua_State *_consoleState = nullptr;
static NString _buff;
static bool _shift = false;

void eng_setres(NArray<NString> &args)
{
	Engine::ScreenResized((int)args[1], (int)args[2]);
}

void eng_msgbox(NArray<NString> &args)
{
	Platform::MessageBox("Message from console", *args[0], MessageBoxButtons::OK, MessageBoxIcon::Information);
}

int Console::Initialize()
{
	Logger::Log(CON_MODULE, LOG_INFORMATION, "Initializing...");

	if (!_buff.Resize(100))
		return ENGINE_FAIL;
	
	_buff.Clear();

	_consoleState = Script::NewState();

	Logger::Log(CON_MODULE, LOG_INFORMATION, "Initialized");

	return ENGINE_OK;
}

void Console::Print(NString &string)
{
	_text.Add(string);
}

void Console::Print(const char *fmt, ...)
{
	va_list args;
	char buff[100];

	va_start(args, fmt);
	vsnprintf(buff, 100, fmt, args);
	va_end(args);
	
	_text.Add(buff);
}

void Console::Clear()
{
	_text.Clear();
}

void Console::Update()
{
	if (!_open)
		return;

	vec2 pos = vec2(10, Engine::GetScreenHeight() * .9);

	GUIManager::DrawString(vec2(pos.x, pos.y), vec3(1, 1, 1), "> ");
	GUIManager::DrawString(vec2(pos.x + 20, pos.y), vec3(1, 1, 1), _buff);
	pos.y -= 20 * (_text.Count() + 1);

	for (const NString &nstr : _text)
	{
		GUIManager::DrawString(pos, vec3(1, 1, 1), nstr);
		pos.y += 20;
	}
}

void Console::HandleKeyDown(uint8_t key)
{
	if (key == NE_KEY_RETURN)
	{
		Print(_buff);
		ExecuteCommand(_buff);
		_buff.Clear();
	}
	else if (key == NE_KEY_UP)
	{
		if (!_commandHistory.Count())
			return;

		_buff = _commandHistory[_historyId];
		if (_historyId > 0) --_historyId;
	}
	else if (key == NE_KEY_DOWN)
	{
		if (!_commandHistory.Count())
			return;

		if (_historyId == _commandHistory.Count() - 1)
		{
			_buff.Clear();
			return;
		}

		_buff = _commandHistory[_historyId];
		if (_historyId < _commandHistory.Count() - 1) ++_historyId;
	}
	else if ((key == NE_KEY_BKSPACE) || (key == NE_KEY_DELETE))
		_buff.RemoveLast();
	else if (key == NE_KEY_TILDE)
		_open = false;
	else if (key == NE_KEY_RSHIFT || key == NE_KEY_LSHIFT)
		_shift = true;
	else
		_buff.Append(Input::KeycodeToChar(key, _shift));
}

void Console::HandleKeyUp(uint8_t key)
{
	if (key == NE_KEY_RSHIFT || key == NE_KEY_LSHIFT)
		_shift = false;
}

void Console::ExecuteCommand(NString cmd, bool record)
{
	NArray<NString> cmdSplit = cmd.Split(' ');

	if (cmdSplit[0] == "set")
	{
		if(_vars[cmdSplit[1]].Set)
			_vars[cmdSplit[1]].Set(cmdSplit[2]);
	}
	else if (cmdSplit[0] == "get")
	{
		if (_vars[cmdSplit[1]].Get)
			Print("%s = %s", *cmdSplit[1], *_vars[cmdSplit[2]].Get());
	}
	else if (cmdSplit[0] == "clear")
		Console::Clear();
	else
	{
		const int err = luaL_dostring(_consoleState, *cmd);
		if (err && lua_gettop(_consoleState))
		{
			Print("ERROR: %s", lua_tostring(_consoleState, -1));
			lua_pop(_consoleState, 1);
		}
	}

	if (!record) return;

	_commandHistory.Add(cmd);
	_historyId = _commandHistory.Count() - 1;
}

void Console::Release()
{
	if (_consoleState)
		lua_close(_consoleState);
}
