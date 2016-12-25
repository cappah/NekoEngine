/* NekoEngine
 *
 * Version.h
 * Author: Alexandru Naiman
 *
 * Engine version definition
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

#include <Platform/Platform.h>

#define	ENGINE_VERSION_MAJOR	0
#define ENGINE_VERSION_MINOR	3
#define ENGINE_VERSION_REVISION 1
#define ENGINE_VERSION_BUILD	295
#define ENGINE_VERSION_PATCH	2

#define ENGINE_VERSION_STRING	"0.3.1.295.2"

#if defined(NE_PLATFORM_WINDOWS)
	#if defined(NE_ARCH_X8664)
		#define ENGINE_PLATFORM_STRING		"Win64"
	#elif defined(NE_ARCH_X86)
		#define ENGINE_PLATFORM_STRING		"Win32"
	#endif
#elif defined(NE_PLATFORM_LINUX)
	#if defined(NE_ARCH_X8664)
		#define ENGINE_PLATFORM_STRING		"Linux x86_64"
	#elif defined(NE_ARCH_X86)
		#define ENGINE_PLATFORM_STRING		"Linux x86"
	#elif defined(NE_ARCH_ARM64)
		#define ENGINE_PLATFORM_STRING		"Linux arm64"
	#elif defined(NE_ARCH_ARM)
		#define ENGINE_PLATFORM_STRING		"Linux arm"
	#elif defined(NE_ARCH_SPARC)
		#define ENGINE_PLATFORM_STRING		"Linux sparc"
	#elif defined(NE_ARCH_SPARC64)
		#define ENGINE_PLATFORM_STRING		"Linux sparc64"
	#elif defined(NE_ARCH_MIPS)
		#define ENGINE_PLATFORM_STRING		"Linux MIPS"
	#elif defined(NE_ARCH_MIPS64)
		#define ENGINE_PLATFORM_STRING		"Linux MIPS64"
	#elif defined(NE_ARCH_PPC)
		#define ENGINE_PLATFORM_STRING		"Linux PowerPC"
	#elif defined(NE_ARCH_PPC64)
		#define ENGINE_PLATFORM_STRING		"Linux PowerPC 64"
	#endif
#elif defined(NE_PLATFORM_FREEBSD)
	#if defined(NE_ARCH_X8664)
		#define ENGINE_PLATFORM_STRING		"FreeBSD x86_64"
	#elif defined(NE_ARCH_X86)
		#define ENGINE_PLATFORM_STRING		"FreeBSD x86"
	#elif defined(NE_ARCH_ARM64)
		#define ENGINE_PLATFORM_STRING		"FreeBSD arm64"
	#elif defined(NE_ARCH_ARM)
		#define ENGINE_PLATFORM_STRING		"FreeBSD arm"
	#elif defined(NE_ARCH_SPARC)
		#define ENGINE_PLATFORM_STRING		"FreeBSD sparc"
	#elif defined(NE_ARCH_SPARC64)
		#define ENGINE_PLATFORM_STRING		"FreeBSD sparc64"
	#elif defined(NE_ARCH_MIPS)
		#define ENGINE_PLATFORM_STRING		"FreeBSD MIPS"
	#elif defined(NE_ARCH_MIPS64)
		#define ENGINE_PLATFORM_STRING		"FreeBSD MIPS64"
	#elif defined(NE_ARCH_PPC)
		#define ENGINE_PLATFORM_STRING		"FreeBSD PowerPC"
	#elif defined(NE_ARCH_PPC64)
		#define ENGINE_PLATFORM_STRING		"FreeBSD PowerPC 64"
	#endif
#elif defined(NE_PLATFORM_IOS)
	#if defined(NE_PLATFORM_IOS_SIM)
		#define ENGINE_PLATFORM_STRING		"iOS Simulator"
	#elif defined(NE_ARCH_ARM64)
		#define ENGINE_PLATFORM_STRING		"iOS arm64"
	#elif defined(NE_ARCH_ARM)
		#define ENGINE_PLATFORM_STRING		"iOS arm"
	#elif defined(NE_ARCH_X8664)
		#define ENGINE_PLATFORM_STRING		"iOS x86_64"
	#elif defined(NE_ARCH_X86)
		#define ENGINE_PLATFORM_STRING		"iOS x86"
	#else
		#define ENGINE_PLATFORM_STRING		"iOS unknown"
	#endif
#elif defined(NE_PLATFORM_MAC)
	#if defined(NE_ARCH_X8664)
		#define ENGINE_PLATFORM_STRING		"macOS x86_64"
	#else
		#define ENGINE_PLATFORM_STRING		"macOS x86"
	#endif
#elif defined(NE_PLATFORM_BB10)
	#if defined(NE_ARCH_ARM)
		#define ENGINE_PLATFORM_STRING		"BB10 arm"
	#elif defined(NE_ARCH_ARM64)
		#define ENGINE_PLATFORM_STRING		"BB10 arm64"
	#elif defined(NE_ARCH_X86)
		#define ENGINE_PLATFORM_STRING		"BB10 x86"
	#elif defined(NE_ARCH_X8664)
		#define ENGINE_PLATFORM_STRING		"BB10 x86_64"
	#else
		#define ENGINE_PLATFORM_STRING		"BB10 unknown"
	#endif
#elif defined(NE_PLATFORM_SUNOS)
	#if defined(NE_ARCH_SPARC64)
		#define ENGINE_PLATFORM_STRING		"SunOS sparc64"
	#elif defined(NE_ARCH_SPARC)
		#define ENGINE_PLATFORM_STRING		"SunOS sparc"
	#elif defined(NE_ARCH_X86)
		#define ENGINE_PLATFORM_STRING		"SunOS x86"
	#elif defined(NE_ARCH_X8664)
		#define ENGINE_PLATFORM_STRING		"SunOS x86_64"
	#endif
#elif defined(NE_PLATFORM_ANDROID)
	#define ENGINE_PLATFORM_STRING			"Android"
#else
	#define ENGINE_PLATFORM_STRING			"Unknown"
#endif
