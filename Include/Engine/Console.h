/* NekoEngine
 *
 * Console.h
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

#pragma once

#include <map>
#include <functional>
#include <glm/glm.hpp>

#include <Engine/Engine.h>
#include <Runtime/Runtime.h>

class Console
{
public:
	static int Initialize();

	ENGINE_API static void RegisterCVar(NString name, std::function<NString()> get, std::function<void(NString &)> set) { _vars.insert(std::make_pair(name, CVarFuncs(get, set))); }
	ENGINE_API static void UnregisterCVar() { }

	ENGINE_API static void RegisterFunc(NString name, std::function<void()> func) { _voidFuncs.insert(std::make_pair(name, func)); }
	ENGINE_API static void RegisterFunc(NString name, std::function<void(NArray<NString> &)> func) { _argFuncs.insert(std::make_pair(name, func)); }
	ENGINE_API static void UnregisterFunc(NString name) { }
	
	ENGINE_API static void Print(NString &string);
	ENGINE_API static void Print(const char *fmt, ...);
	ENGINE_API static void Clear();

	ENGINE_API static void OpenConsole() { _open = true; }
	ENGINE_API static void CloseConsole() { _open = false; }
	ENGINE_API static bool IsOpen() { return _open; }

	static void Draw();

	static void HandleKeyDown(uint8_t key);

	ENGINE_API static void ExecuteCommand(NString cmd);

private:
	struct CVarFuncs
	{
		CVarFuncs() { Get = nullptr; Set = nullptr; }
		CVarFuncs(std::function<NString()> get, std::function<void(NString &)> set) 
		{
			Get = get;
			Set = set;
		}
		CVarFuncs(const CVarFuncs &other)
		{
			Get = other.Get;
			Set = other.Set;
		}
		std::function<NString()> Get;
		std::function<void(NString &)> Set;
	};

	static bool _open;
	static NArray<NString> _text;
	static std::map<NString, CVarFuncs> _vars;
	static std::map<NString, std::function<void()>> _voidFuncs;
	static std::map<NString, std::function<void(NArray<NString> &)>> _argFuncs;

	Console();
};

template<typename T>
class ConsoleVariable
{
public:
	ConsoleVariable(NString name) { Console::RegisterCVar(name, [=]() -> NString &{ return this->GetString(); }, [=](NString &str) { this->SetString(str); }); }
	
	NString &GetString() { return _str; }
	void SetString(NString &str) { }
	
	T &Get() { return _t; }
	void Set(T t) { _t = t; }
	
private:
	T _t;
	NString _str;
};

#define REGISTER_CVAR(a, b) static ConsoleVariable<a> b(#b)
#define REGISTER_CVAR_FUNCS(a, b, c) Console::RegisterCVar(#a, [=]() -> NString { return this->b(); }, [=](NString &str) { this->c(str); });
#define REGISTER_CVAR_STATIC(a, b, c) Console::RegisterCVar(#a, [=]() -> NString { return b(); }, [=](NString &str) { c(str); });

#if defined(_MSC_VER)
template class ENGINE_API ConsoleVariable<float>;
#endif
