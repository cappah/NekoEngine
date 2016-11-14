/* NekoEngine
 *
 * Console.cpp
 * Author: Alexandru Naiman
 *
 * NekoEngine console
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

#include <Engine/Input.h>
#include <Engine/Engine.h>
#include <Engine/Console.h>
#include <Engine/Keycodes.h>
#include <Renderer/GUI.h>
#include <System/Logger.h>
#include <Script/Script.h>

#define MAX_SCREEN_LINES	15
#define CON_MODULE		"Console"

using namespace std;
using namespace glm;

bool Console::_open;
NArray<NString> Console::_text;
map<NString, Console::CVarFuncs> Console::_vars;
map<NString, function<void()>> Console::_voidFuncs;
map<NString, function<void(NArray<NString> &)>> Console::_argFuncs;

static lua_State *_consoleState = nullptr;
static NString _buff;
static bool _shift = false;
static map<char, char> _shiftedChars
{
	{ 0x2C, 0x3C },
	{ 0x2D, 0x5F },
	{ 0x2E, 0x3E },
	{ 0x2F, 0x3F },
	{ 0x30, 0x29 },
	{ 0x31, 0x21 },
	{ 0x32, 0x40 },
	{ 0x33, 0x23 },
	{ 0x34, 0x24 },
	{ 0x35, 0x25 },
	{ 0x36, 0x5E },
	{ 0x37, 0x26 },
	{ 0x38, 0x2A },
	{ 0x39, 0x28 },
	{ 0x2B, 0x3D },
	{ '\'', '"' }
};

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

	_voidFuncs.insert(make_pair("exit", Engine::Exit));
	//_voidFuncs.insert(make_pair("screenshot", Engine::SaveScreenshot));
	_voidFuncs.insert(make_pair("showStats", Engine::ToggleStats));
	_voidFuncs.insert(make_pair("pause", Engine::TogglePause));
	_voidFuncs.insert(make_pair("clear", Console::Clear));

	_voidFuncs.insert(make_pair("capturePointer", Input::CapturePointer));
	_voidFuncs.insert(make_pair("releasePointer", Input::ReleasePointer));
	
	_argFuncs.insert(make_pair("setres", eng_setres));
	_argFuncs.insert(make_pair("msgbox", eng_msgbox));

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

	GUI::DrawString(vec2(pos.x, pos.y), vec3(1, 1, 1), "> ");
	GUI::DrawString(vec2(pos.x + 20, pos.y), vec3(1, 1, 1), _buff);
	pos.y -= 20 * (_text.Count() + 1);

	for (NString &nstr : _text)
	{
		GUI::DrawString(pos, vec3(1, 1, 1), nstr);
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
	else if ((key == NE_KEY_BKSPACE) || (key == NE_KEY_DELETE))
		_buff.RemoveLast();
	else if (key == NE_KEY_TILDE)
		_open = false;
	else if (key == NE_KEY_RSHIFT || key == NE_KEY_LSHIFT)
		_shift = true;
	else
	{
		uint8_t ch = keycodeToChar[key];

		if (_shift)
		{
			if (ch > 0x60 && ch < 0x7B) ch -= 0x20;
			else if (ch > 0x5A && ch < 0x5E) ch += 0x20;
			else if (_shiftedChars.find(ch) != _shiftedChars.end())
				ch = _shiftedChars[ch];
		}

		_buff.Append(ch);
	}
}

void Console::HandleKeyUp(uint8_t key)
{
	if (key == NE_KEY_RSHIFT || key == NE_KEY_LSHIFT)
		_shift = false;
}

void Console::ExecuteCommand(NString cmd)
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
	else if (cmdSplit[0] == "luaExec")
	{
		int err = luaL_dostring(_consoleState, *cmd + 8);
		if (err && lua_gettop(_consoleState))
		{
			Print("ERROR: %s", lua_tostring(_consoleState, -1));
			lua_pop(_consoleState, 1);
		}
	}
	else
	{
		if (cmdSplit.Count() < 2)
		{
			if (_voidFuncs[cmdSplit[0]])
				_voidFuncs[cmdSplit[0]]();
			else
				Print("%s: undefined function", *cmdSplit[0]);
		}
		else
		{
			if (_argFuncs[cmdSplit[0]])
				_argFuncs[cmdSplit[0]](cmdSplit);
			else
				Print("%s: undefined function", *cmdSplit[0]);
		}
	}
}

void Console::Release()
{
	lua_close(_consoleState);
}
