/* Neko Engine
 *
 * PlatformDetect.h
 * Author: Alexandru Naiman
 *
 * Platform detection macros
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

#if defined(_WIN64)
#define NE_PLATFORM_WIN64
#define NE_PLATFORM_WINDOWS
#define NE_ARCH_X8664
#elif defined(_WIN32)
#define NE_PLATFORM_WIN32
#define NE_PLATFORM_WINDOWS
#define NE_ARCH_X86
#elif defined(__linux__)
#define NE_PLATFORM_LINUX
#define NE_PLATFORM_X11
#if defined(__arm__)
#ifdef __LP64__
#define NE_PLATFORM_LINUX_ARM64
#define NE_ARCH_ARM64
#else
#define NE_PLATFORM_LINUX_ARM
#define NE_ARCH_ARM
#endif
#elif defined(__sparc)
#ifdef __LP64__
#define NE_PLATFORM_LINUX_SPARC64
#define NE_ARCH_SPARC64
#else
#define NE_PLATFORM_LINUX_SPARC
#define NE_ARCH_SPARC
#endif
#elif defined(__mips__)
#ifdef __LP64__
#define NE_PLATFORM_LINUX_MIPS64
#define NE_ARCH_MIPS64
#else
#define NE_PLATFORM_LINUX_MIPS
#define NE_ARCH_MIPS
#endif
#elif defined(__powerpc__)
#ifdef __LP64__
#define NE_PLATFORM_LINUX_PPC64
#define NE_ARCH_PPC64
#else
#define NE_PLATFORM_LINUX_PPC
#define NE_ARCH_PPC
#endif
#else
#ifdef __LP64__
#define NE_PLATFORM_LINUX_X8664
#define NE_ARCH_X8664
#else
#define NE_PLATFORM_LINUX_X86
#define NE_ARCH_X86
#endif
#endif
#elif defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR == 1
#define NE_PLATFORM_IOS
#define NE_PLATFORM_IOS_SIM
#define NE_ARCH_X8664
#define NE_DEVICE_MOBILE
#elif TARGET_OS_IPHONE == 1
#define NE_PLATFORM_IOS
#ifdef __LP64__
#define NE_ARCH_ARM64
#else
#define NE_ARCH_ARM
#endif
#define NE_DEVICE_MOBILE
#else
#define NE_PLATFORM_MAC
#ifdef __LP64__
#define NE_ARCH_X8664
#else
#define NE_ARCH_X86
#endif
#endif
#elif defined(__QNX__)
#define NE_DEVICE_MOBILE
#ifdef __arm__
#define NE_PLATFORM_BB10
#ifdef __LP64__
#define NE_ARCH_ARM64
#else
#define NE_ARCH_ARM
#endif
#else
#define NE_PLATFORM_BB10
#ifdef __LP64__
#define NE_ARCH_X8664
#else
#define NE_ARCH_ARM
#endif
#endif
#elif defined(__FreeBSD__)
#define NE_PLATFORM_FREEBSD
#define NE_PLATFORM_X11
#if defined(__arm__)
#ifdef __LP64__
#define NE_PLATFORM_FREEBSD_ARM64
#define NE_ARCH_ARM64
#else
#define NE_PLATFORM_FREEBSD_ARM
#define NE_ARCH_ARM
#endif
#else
#ifdef __LP64__
#define NE_PLATFORM_FREEBSD_X8664
#define NE_ARCH_X8664
#else
#define NE_PLATFORM_FREEBSD_X86
#define NE_ARCH_X86
#endif
#endif
#elif defined(__DragonFly__)
#define NE_PLATFORM_DRAGONFLY
#define NE_PLATFORM_X11
#if defined(__arm__)
#ifdef __LP64__
#define NE_PLATFORM_DRAGONFLY_ARM64
#define NE_ARCH_ARM64
#else
#define NE_PLATFORM_DRAGONFLY_ARM
#define NE_ARCH_ARM
#endif
#else
#ifdef __LP64__
#define NE_PLATFORM_DRAGONFLY_X8664
#define NE_ARCH_X8664
#else
#define NE_PLATFORM_DRAGONFLY_X86
#define NE_ARCH_X86
#endif
#endif
#elif defined(__NetBSD__)
#define NE_PLATFORM_NETBSD
#define NE_PLATFORM_X11
#if defined(__arm__)
#ifdef __LP64__
#define NE_PLATFORM_NETBSD_ARM64
#define NE_ARCH_ARM64
#else
#define NE_PLATFORM_NETBSD_ARM
#define NE_ARCH_ARM
#endif
#else
#ifdef __LP64__
#define NE_PLATFORM_NETBSD_X8664
#define NE_ARCH_X8664
#else
#define NE_PLATFORM_NETBSD_X86
#define NE_ARCH_X86
#endif
#endif
#elif defined(__OpenBSD__)
#define NE_PLATFORM_OPENBSD
#define NE_PLATFORM_X11
#if defined(__arm__)
#ifdef __LP64__
#define NE_PLATFORM_OPENBSD_ARM64
#define NE_ARCH_ARM64
#else
#define NE_PLATFORM_OPENBSD_ARM
#define NE_ARCH_ARM
#endif
#else
#ifdef __LP64__
#define NE_PLATFORM_OPENBSD_X8664
#define NE_ARCH_X8664
#else
#define NE_PLATFORM_OPENBSD_X86
#define NE_ARCH_X86
#endif
#endif
#elif defined(sun) || defined(__sun)
#define NE_PLATFORM_SUNOS
#define NE_PLATFORM_X11
#if defined(__sparc)
#ifdef __LP64__
#define NE_PLATFORM_SUNOS_SPARC64
#define NE_ARCH_SPARC64
#else
#define NE_PLATFORM_SUNOS_SPARC
#define NE_ARCH_SPARC
#endif
#else
#ifdef __LP64__
#define NE_PLATFORM_SUNOS_X8664
#define NE_ARCH_X8664
#else
#define NE_PLATFORM_SUNOS_X86
#define NE_ARCH_X86
#endif
#endif
#elif defined(__ANDROID__)
#define NE_PLATFORM_ANDROID
#endif
