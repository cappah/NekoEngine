/* NekoEngine
 *
 * Debug.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Debugging
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

#include <Engine/Debug.h>
#include <System/Logger.h>

#include <mutex>
#include <unordered_map>

using namespace std;

struct AllocInfo
{
	size_t size;
	const char *name;
};

static unordered_map<thread::id, const char *> _threadNames;
static unordered_map<void *, AllocInfo> _allocInfo;
static mutex _mutex;

void EngineDebug::SetThreadName(thread::id tid, const char *name)
{
	_mutex.lock();
	_threadNames.insert_or_assign(tid, name);
	_mutex.unlock();
}

const char *EngineDebug::GetThreadName(thread::id tid)
{
	const char *name = "no name";

	_mutex.lock();
	if (_threadNames.count(tid))
		name = _threadNames[tid];
	_mutex.unlock();

	return name;
}

void EngineDebug::RegisterAlloc(void *mem, size_t size, const char *name)
{
	_mutex.lock();
	_allocInfo[mem] = { size, name };
	_mutex.unlock();
}

void EngineDebug::UnregisterAlloc(void *mem)
{
	_mutex.lock();
	if (!_allocInfo.count(mem))
		return;

	_allocInfo.erase(mem);
	_mutex.unlock();
}

void EngineDebug::LogLeaks()
{
	/*size_t totalSize{ 0 };
	Logger::Log("EngineDebug", LOG_DEBUG, "Memory leak summary:");

	for (AllocInfo &ai : _allocInfo)
	{
		Logger::Log("EngineDebug", LOG_DEBUG, "\t%s, size %llu, address: 0x%x", ai.name, ai.size, ai.mem);
		totalSize += ai.size;
	}

	Logger::Log("EngineDebug", LOG_DEBUG, "Total leaked memory: %llu", totalSize);*/
}

#if defined(NE_CONFIG_DEBUG) || defined(NE_CONFIG_DEVELOPMENT)
/*void *operator new(size_t count)
{
	void *mem{ calloc(count, 1) };
	if (!mem)
		return nullptr;

	EngineDebug::RegisterAlloc(mem, count, "implicit new");
	return mem;
}

void *operator new[](size_t count)
{
	void *mem{ calloc(count, 1) };
	if (!mem)
		return nullptr;

	EngineDebug::RegisterAlloc(mem, count, "implicit new");
	return mem;
}

void operator delete(void *ptr)
{
	EngineDebug::UnregisterAlloc(ptr);
	free(ptr);
}

void operator delete(void *ptr, size_t sz)
{
	memset(ptr, 0x0, sz);
	EngineDebug::UnregisterAlloc(ptr);
	free(ptr);
}

void operator delete[](void *ptr)
{
	EngineDebug::UnregisterAlloc(ptr);
	free(ptr);
}

void operator delete[](void *ptr, size_t sz)
{
	memset(ptr, 0x0, sz);
	EngineDebug::UnregisterAlloc(ptr);
	free(ptr);
}*/
#endif