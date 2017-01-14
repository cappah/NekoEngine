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

#pragma once

#include <thread>

class EngineDebug
{
public:
	static void SetThreadName(std::thread::id tid, const char *name);
	static const char *GetThreadName(std::thread::id tid);
	static void RegisterAlloc(void *mem, size_t size, const char *name);
	static void UnregisterAlloc(void *mem);
	static void LogLeaks();
};

#if defined(NE_CONFIG_DEBUG) || defined(NE_CONFIG_DEVELOPMENT)
	#define DBG_SET_THREAD_NAME(name) EngineDebug::SetThreadName(std::this_thread::get_id(), name)
	#define DBG_GET_THREAD_NAME() EngineDebug::GetThreadName(std::this_thread::get_id())
	#define DBG_REGISTER_ALLOC(mem, size, name)
	#define DBG_UNREGISTER_ALLOC(mem)
#else
	#define DBG_SET_THREAD_NAME(name)
	#define DBG_GET_THREAD_NAME() "debuggging disabled in release builds"
#endif
