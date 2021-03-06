/* NekoEngine
 *
 * GameModule.h
 * Author: Alexandru Naiman
 *
 * GameModule class definition 
 * This class must be implemented by the game
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

#pragma once

#include <Engine/Engine.h>

#ifndef ENGINE_INTERNAL

#define NEKO_GAME_MODULE()																													\
class Object;																																\
template<typename T> Object *gameModuleFactoryCreateObject(ObjectInitializer *initializer) { return new T(initializer); }					\
template<typename T> ObjectComponent *gameModuleFactoryCreateComponent(ComponentInitializer *initializer) { return new T(initializer); }	\
class GameModuleClassFactory																												\
{																																			\
public:																																		\
	static Object *NewObject(const std::string &s, ObjectInitializer *initializer)															\
	{																																		\
		ObjectClassMapType::iterator it = GetObjectMap()->find(s);																			\
		if (it == GetObjectMap()->end())																									\
			return nullptr;																													\
		return it->second(initializer);																										\
	}																																		\
	static ObjectComponent *NewComponent(const std::string &s, ComponentInitializer *initializer)											\
	{																																		\
		ComponentClassMapType::iterator it = GetComponentMap()->find(s);																	\
		if (it == GetComponentMap()->end())																									\
			return nullptr;																													\
		return it->second(initializer);																										\
	}																																		\
	static ObjectClassMapType *GetObjectMap()	noexcept																					\
	{																																		\
		if(!_objectClassMap)																												\
			_objectClassMap = new ObjectClassMapType();																						\
		return _objectClassMap;																												\
	}																																		\
	static ComponentClassMapType *GetComponentMap()	noexcept																				\
	{																																		\
		if(!_componentClassMap)																												\
			_componentClassMap = new ComponentClassMapType();																				\
		return _componentClassMap;																											\
	}																																		\
	static void CleanUp() noexcept																											\
	{																																		\
		if(_objectClassMap)																													\
			delete _objectClassMap;																											\
		if(_componentClassMap)																												\
			delete _componentClassMap;																										\
	}																																		\
protected:																																	\
	static ObjectClassMapType *_objectClassMap;																								\
	static ComponentClassMapType *_componentClassMap;																						\
};																																			\
template<typename T>																														\
class GameModuleClassFactoryRegisterObjectClass : GameModuleClassFactory																	\
{																																			\
public:																																		\
	GameModuleClassFactoryRegisterObjectClass(const std::string &name) noexcept																\
	{																																		\
		GetObjectMap()->insert(std::make_pair(name, &gameModuleFactoryCreateObject<T>));													\
	}																																		\
};																																			\
template<typename T>																														\
class GameModuleClassFactoryRegisterComponentClass : GameModuleClassFactory																	\
{																																			\
public:																																		\
	GameModuleClassFactoryRegisterComponentClass(const std::string &name) noexcept															\
	{																																		\
		GetComponentMap()->insert(std::make_pair(name, &gameModuleFactoryCreateComponent<T>));												\
	}																																		\
};																																			\

#define NEKO_GAME_MODULE_CLASS(ClassName)																									\
public:																																		\
	ClassName() noexcept { _gameModuleName = #ClassName; }																					\
	virtual Object *NewObject(const std::string &s, ObjectInitializer *initializer = nullptr) override;										\
	virtual ObjectComponent *NewComponent(const std::string &s, ComponentInitializer *initializer) override;								\
	~ClassName() noexcept;																													\

#ifdef WIN32
#define NEKO_GAME_MODULE_BASE_EXPORT __declspec(dllexport)
#else
#define NEKO_GAME_MODULE_BASE_EXPORT
#endif

#define NEKO_GAME_MODULE_IMPL(ClassName)																									\
ObjectClassMapType *GameModuleClassFactory::_objectClassMap = nullptr;																		\
ComponentClassMapType *GameModuleClassFactory::_componentClassMap = nullptr;																\
Object *ClassName::NewObject(const std::string &s, ObjectInitializer *initializer)															\
{																																			\
	return GameModuleClassFactory::NewObject(s, initializer);																				\
}																																			\
ObjectComponent *ClassName::NewComponent(const std::string &s, ComponentInitializer *initializer)											\
{																																			\
	return GameModuleClassFactory::NewComponent(s, initializer);																			\
}																																			\
ClassName::~ClassName()	noexcept																											\
{																																			\
	CleanUp();																																\
	GameModuleClassFactory::CleanUp();																										\
}																																			\
extern "C" GameModule NEKO_GAME_MODULE_BASE_EXPORT *createGameModule()																		\
{																																			\
	return new ClassName();																													\
}																																			\

#define REGISTER_OBJECT_CLASS(x) GameModuleClassFactoryRegisterObjectClass<x> regClass ## x(#x);
#define REGISTER_COMPONENT_CLASS(x) GameModuleClassFactoryRegisterComponentClass<x> regClass ## x(#x);

#else
#ifdef NE_DEVICE_MOBILE
	extern "C" GameModule *createGameModule();
#endif
#endif

class ENGINE_API GameModule
{
public:
	GameModule() : _gameModuleName("GameModule") {};

	virtual Object *NewObject(const std::string &s, ObjectInitializer *initializer) = 0;
	virtual ObjectComponent *NewComponent(const std::string &s, ComponentInitializer *initializer) = 0;
	const char *GetModuleName() { return _gameModuleName; }

	virtual int Initialize() = 0;
	virtual void CleanUp() = 0;

	virtual ~GameModule() {};

protected:
	const char *_gameModuleName;
};
