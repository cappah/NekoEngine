/* Neko Engine
 *
 * Compat.h
 * Author: Alexandru Naiman
 *
 * Engine compatibility functions
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

#include <Engine/Engine.h>

#include <stddef.h>
#include <stdint.h>

#if !defined(NE_PLATFORM_OPENBSD) && !defined(NE_PLATFORM_LINUX)
void ENGINE_API *reallocarray(void *optr, size_t nmemb, size_t size);
#endif

#if defined(NE_PLATFORM_MAC) || defined(NE_PLATFORM_IOS) || defined(NE_PLATFORM_OPENBSD) || defined(NE_PLATFORM_FREEBSD)
	#include <stdlib.h>
	#define NE_RANDOM() (arc4random() % ((unsigned)RAND_MAX + 1))
	#define NE_SRANDOM(x)
#elif defined(NE_PLATFORM_LINUX)
	#include <bsd/stdlib.h>
	#define NE_RANDOM() (arc4random() % ((unsigned)RAND_MAX + 1))
	#define NE_SRANDOM(x)
//#elif defined(NE_PLATFORM_WIN32) || defined(NE_PLATFORM_WIN64)
#else
	#define NE_RANDOM() rand()
	#define NE_SRANDOM(x) srand(x)
#endif
