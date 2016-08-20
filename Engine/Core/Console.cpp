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

#include <Engine/Engine.h>
#include <Engine/Console.h>
#include <Engine/Keycodes.h>

using namespace std;
using namespace glm;

bool Console::_open;
NArray<ConsoleString> Console::_text;
map<NString, Console::CVarFuncs> Console::_vars;
map<NString, function<void()>> Console::_voidFuncs;
map<NString, function<void(NArray<NString> &)>> Console::_argFuncs;

static float _nextX, _nextY;
static NString _buff;

void eng_setres(NArray<NString> &args)
{
	Engine::ScreenResized((int)args[1], (int)args[2]);
}

void eng_showstats(NArray<NString> &args)
{

}

int Console::Initialize()
{
	_voidFuncs.insert(make_pair("eng_exit", Engine::Exit));
	_voidFuncs.insert(make_pair("eng_screenshot", Engine::SaveScreenshot));
	
	_argFuncs.insert(make_pair("eng_setres", eng_setres));

	return _buff.Resize(100) ? ENGINE_OK : ENGINE_FAIL;
}

void Console::Print(NString &string)
{
	_nextX += 20;
}

void Console::Print(const char *fmt, ...)
{
	//
}

void Console::Clear()
{
	_text.Clear();
}

void Console::Draw()
{
	if (!_open)
		return;

	for (ConsoleString &cstr : _text)
		Engine::DrawString(cstr.pos, cstr.color, cstr.text);

	Engine::DrawString(vec2(100, 100), vec3(1, 1, 1), _buff);
}

void Console::HandleKeyDown(uint8_t key)
{
	if (key == NE_KEY_RETURN)
	{
		ExecuteCommand(_buff);
		_buff.Clear();
	}
	else if (key == NE_KEY_LEFT)
	{
		//
	}
	else if (key == NE_KEY_RIGHT)
	{
		//
	}
	else if (key == NE_KEY_UP)
	{
		//
	}
	else if (key == NE_KEY_DOWN)
	{
		//
	}
	else if (key == NE_KEY_BKSPACE)
		_buff.RemoveLast();
	else if (key == NE_KEY_DELETE)
	{
		//
	}
	else
	{

		_buff.Append(keycodeToChar[key]);
	}
}

void Console::ExecuteCommand(NString cmd)
{
	NArray<NString> cmdSplit = cmd.Split(' ');

	if (cmdSplit.Count() < 2)
		return;

	if (cmdSplit[0] == "call")
	{
		if (cmdSplit.Count() == 2)
		{
			if (_voidFuncs[cmdSplit[1]])
				_voidFuncs[cmdSplit[1]]();
			else
				Print("%s: undefined function", *cmdSplit[1]);
		}
		else
		{
			cmdSplit.Remove(0);

			if (_argFuncs[cmdSplit[0]])
				_argFuncs[cmdSplit[0]](cmdSplit);
			else
				Print("%s: undefined function", *cmdSplit[1]);
		}
	}
	else if (cmdSplit[0] == "set")
	{
		if(_vars[cmdSplit[1]].Set)
			_vars[cmdSplit[1]].Set(cmdSplit[2]);
	}
	else if (cmdSplit[0] == "get")
	{
		if (_vars[cmdSplit[1]].Get)
			Print("%s = %s", *cmdSplit[1], *_vars[cmdSplit[2]].Get());
	}
}