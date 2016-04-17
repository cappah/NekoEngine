/* Neko Engine
 *
 * EngineClassFactory.h
 * Author: Alexandru Naiman
 *
 * Class factory for Object types
 *
 * ----------------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman <alexandru dot naiman at icloud dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ALEXANDRU NAIMAN
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#pragma once

#include <Platform/Platform.h>

#include <string>
#include <map>

#ifdef ES_PLATFORM_WINDOWS
	#ifdef ENGINE_INTERNAL
		#define ENGINE_API	__declspec(dllexport)
	#else
		#define ENGINE_API	__declspec(dllimport)
	#endif
#else
	#define ENGINE_API
#endif

#define ENGINE_REGISTER_OBJECT_CLASS(x) static EngineClassFactoryRegisterObjectClass<x> regClass ## x(#x);

class GameModule;
class Object;

typedef GameModule *(*CreateGameModuleProc)();
typedef std::map<std::string, Object*(*)()> ObjectClassMapType;

template<typename T> Object *engineFactoryCreateObject() { return new T(); }

class EngineClassFactory
{
public:
	static Object *NewObject(const std::string &s)
	{
		ObjectClassMapType::iterator it = GetMap()->find(s);
		if (it == GetMap()->end())
			return nullptr;
		return it->second();
	}

	static ObjectClassMapType *GetMap() noexcept
	{
		if(!_objectClassMap)
			_objectClassMap = new ObjectClassMapType();
		return _objectClassMap;
	}

	static void CleanUp() noexcept
	{
		if(_objectClassMap)
			delete _objectClassMap;
	}

protected:
	static ObjectClassMapType *_objectClassMap;
};

template<typename T>
class ENGINE_API EngineClassFactoryRegisterObjectClass : EngineClassFactory
{
public:
	EngineClassFactoryRegisterObjectClass(const std::string &name) noexcept
	{
		GetMap()->insert(std::make_pair(name, &engineFactoryCreateObject<T>));
	}
};
