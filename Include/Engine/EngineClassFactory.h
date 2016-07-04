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

#define ENGINE_REGISTER_OBJECT_CLASS(x) static EngineClassFactoryRegisterObjectClass<x> regClass ## x(#x);
#define ENGINE_REGISTER_COMPONENT_CLASS(x) static EngineClassFactoryRegisterComponentClass<x> regClass ## x(#x);

class GameModule;
class Object;
class ObjectComponent;
class ObjectInitializer;
class ComponentInitializer;

typedef GameModule *(*CreateGameModuleProc)();
typedef std::map<std::string, Object*(*)(ObjectInitializer*)> ObjectClassMapType;
typedef std::map<std::string, ObjectComponent*(*)(ComponentInitializer*)> ComponentClassMapType;

template<typename T> Object *engineFactoryCreateObject(ObjectInitializer *initializer) { return new T(initializer); }
template<typename T> ObjectComponent *engineFactoryCreateComponent(ComponentInitializer *initializer) { return new T(initializer); }

class EngineClassFactory
{
public:
	static Object *NewObject(const std::string &s, ObjectInitializer *initializer)
	{
		ObjectClassMapType::iterator it = GetObjectMap()->find(s);
		if (it == GetObjectMap()->end())
			return nullptr;
		return it->second(initializer);
	}

	static ObjectComponent *NewComponent(const std::string &s, ComponentInitializer *initializer)
	{
		ComponentClassMapType::iterator it = GetComponentMap()->find(s);
		if (it == GetComponentMap()->end())
			return nullptr;
		return it->second(initializer);
	}

	static ObjectClassMapType *GetObjectMap() noexcept
	{
		if(!_objectClassMap)
			_objectClassMap = new ObjectClassMapType();
		return _objectClassMap;
	}

	static ComponentClassMapType *GetComponentMap() noexcept
	{
		if (!_componentClassMap)
			_componentClassMap = new ComponentClassMapType();
		return _componentClassMap;
	}

	static void CleanUp() noexcept
	{
		if(_objectClassMap)
			delete _objectClassMap;

		if (_componentClassMap)
			delete _componentClassMap;
	}

protected:
	static ObjectClassMapType *_objectClassMap;
	static ComponentClassMapType *_componentClassMap;
};

template<typename T>
class ENGINE_API EngineClassFactoryRegisterObjectClass : EngineClassFactory
{
public:
	EngineClassFactoryRegisterObjectClass(const std::string &name) noexcept
	{
		GetObjectMap()->insert(std::make_pair(name, &engineFactoryCreateObject<T>));
	}
};

template<typename T>
class ENGINE_API EngineClassFactoryRegisterComponentClass : EngineClassFactory
{
public:
	EngineClassFactoryRegisterComponentClass(const std::string &name) noexcept
	{
		GetComponentMap()->insert(std::make_pair(name, &engineFactoryCreateComponent<T>));
	}
};
