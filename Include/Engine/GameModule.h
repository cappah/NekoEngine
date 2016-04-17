/* Neko Engine
 *
 * GameModule.h
 * Author: Alexandru Naiman
 *
 * GameModule class definition 
 * This class must be implemented by the game
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

#include <Engine/Engine.h>

#ifndef ENGINE_INTERNAL

#define NEKO_GAME_MODULE()																\
class Object;																			\
template<typename T> Object *gameModuleFactoryCreateObject() { return new T(); }		\
class GameModuleClassFactory															\
{																						\
public:																					\
	static Object *NewObject(const std::string &s)										\
	{																					\
		ObjectClassMapType::iterator it = GetMap()->find(s);							\
		if (it == GetMap()->end())														\
			return nullptr;																\
		return it->second();															\
	}																					\
	static ObjectClassMapType *GetMap()	noexcept										\
	{																					\
		if(!_objectClassMap)															\
			_objectClassMap = new ObjectClassMapType();									\
		return _objectClassMap;															\
	}																					\
	static void CleanUp() noexcept														\
	{																					\
		if(_objectClassMap)																\
			delete _objectClassMap;														\
	}																					\
protected:																				\
	static ObjectClassMapType *_objectClassMap;											\
};																						\
template<typename T>																	\
class GameModuleClassFactoryRegisterClass : GameModuleClassFactory						\
{																						\
public:																					\
	GameModuleClassFactoryRegisterClass(const std::string &name) noexcept				\
	{																					\
		GetMap()->insert(std::make_pair(name, &gameModuleFactoryCreateObject<T>));		\
	}																					\
};																						\

#define NEKO_GAME_MODULE_CLASS(ClassName)												\
public:																					\
	ClassName() noexcept { _gameModuleName = #ClassName; }								\
	virtual Object *NewObject(const std::string &s) override;							\
	~ClassName() noexcept;																\

#ifdef WIN32
#define NEKO_GAME_MODULE_BASE_EXPORT __declspec(dllexport)
#else
#define NEKO_GAME_MODULE_BASE_EXPORT
#endif

#define NEKO_GAME_MODULE_IMPL(ClassName)												\
ObjectClassMapType *GameModuleClassFactory::_objectClassMap = nullptr;					\
Object *ClassName::NewObject(const std::string &s)										\
{																						\
	return GameModuleClassFactory::NewObject(s);										\
}																						\
ClassName::~ClassName()	noexcept														\
{																						\
	CleanUp();																			\
	GameModuleClassFactory::CleanUp();													\
}																						\
extern "C" GameModule NEKO_GAME_MODULE_BASE_EXPORT *createGameModule()					\
{																						\
	return new ClassName();																\
}																						\

#define REGISTER_OBJECT_CLASS(x) GameModuleClassFactoryRegisterClass<x> regClass ## x(#x);

#else
#ifdef ES_DEVICE_MOBILE
	extern "C" GameModule *createGameModule();
#endif
#endif

class ENGINE_API GameModule
{
public:
	GameModule() : _gameModuleName("GameModule") {};

	virtual Object *NewObject(const std::string &s) = 0;
	const char *GetModuleName() { return _gameModuleName; }

	virtual int Initialize() = 0;
	virtual void LoadObjectOptionalArguments(Object *obj, const std::vector<char*> &args) = 0;
	virtual void CleanUp() = 0;

	virtual ~GameModule() {};

protected:
	const char *_gameModuleName;
};
