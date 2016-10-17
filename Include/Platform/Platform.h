/* NekoEngine
 *
 * Platform.h
 * Author: Alexandru Naiman
 *
 * Platform specific functions
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

#include <Platform/PlatformDetect.h>

#include <stddef.h>

#ifdef NE_PLATFORM_WINDOWS
	#ifdef PLATFORM_INTERNAL
		#define PLATFORM_API	__declspec(dllexport)
	#else
		#define PLATFORM_API	__declspec(dllimport)
	#endif
#else
	#define PLATFORM_API
#endif

#ifdef _WIN32
#include <Windows.h>

#undef CreateWindow
#undef MessageBox

typedef HWND PlatformWindowType;
typedef HDC PlatformDisplayType;
typedef HMODULE PlatformModuleType;

#elif defined(PLATFORM_X11)
#include <X11/Xlib.h>

typedef Window PlatformWindowType;
typedef Display *PlatformDisplayType;
typedef void *PlatformModuleType;
#elif defined(NE_PLATFORM_MAC)
#import <Cocoa/Cocoa.h>

typedef NSWindow *PlatformWindowType;
typedef NSView *PlatformDisplayType;
typedef void *PlatformModuleType;
#elif defined(NE_PLATFORM_IOS)
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

typedef UIWindow *PlatformWindowType;
typedef UIView *PlatformDisplayType;
typedef void *PlatformModuleType;

@protocol EngineInputDelegateProtocol <NSObject>

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event;

@end

@protocol EngineViewProtocol <NSObject>

- (void)setInputDelegate:(id<EngineInputDelegateProtocol>)delegate;

@end

#elif defined(NE_PLATFORM_BB10)

#include <screen/screen.h>

typedef screen_window_t PlatformWindowType;
typedef screen_context_t PlatformDisplayType;
typedef void *PlatformModuleType;

#elif defined(NE_PLATFORM_ANDROID)

#include <android_native_app_glue.h>

typedef ANativeWindow *PlatformWindowType;
typedef void *PlatformDisplayType;
typedef void *PlatformModuleType;

#endif

enum class MessageBoxButtons : unsigned char
{
	OK,
	YesNo
};

enum class MessageBoxIcon : unsigned char
{
	Information,
	Warning,
	Error,
	Question
};

enum class MessageBoxResult : unsigned char
{
	OK,
	Yes,
	No
};

class Platform
{
public:

	// Platform specific public functions
	PLATFORM_API static const char* GetName();
	PLATFORM_API static const char* GetMachineName();
	PLATFORM_API static const char* GetMachineArchitecture();
	PLATFORM_API static const char* GetVersion();

	PLATFORM_API static PlatformWindowType GetActiveWindow() { return _activeWindow; }

	PLATFORM_API static void SetWindowTitle(PlatformWindowType hWnd, const char* title);
	PLATFORM_API static bool EnterFullscreen(int width, int height);

	PLATFORM_API static bool CapturePointer();
	PLATFORM_API static void ReleasePointer();
	PLATFORM_API static bool GetPointerPosition(long &x, long &y);
	PLATFORM_API static bool SetPointerPosition(long x, long y);
	PLATFORM_API static bool GetTouchMovementDelta(float &x, float &y);

	PLATFORM_API static MessageBoxResult MessageBox(const char* title, const char* message, MessageBoxButtons buttons, MessageBoxIcon icon);

	PLATFORM_API static void LogDebugMessage(const char* message);

	PLATFORM_API static void Exit();

	// Shared
	PLATFORM_API static size_t GetConfigString(const char *section, const char *entry, const char *def, char *buffer, int buffer_len, const char *file);
	PLATFORM_API static int GetConfigInt(const char *section, const char *entry, int def, const char *file);
	PLATFORM_API static float GetConfigFloat(const char *section, const char *entry, float def, const char *file);
	PLATFORM_API static double GetConfigDouble(const char *section, const char *entry, double def, const char *file);
	PLATFORM_API static size_t GetConfigSection(const char *section, char *out, size_t size, const char *file);
	PLATFORM_API static int Rand();

#ifdef PLATFORM_INTERNAL // Platform specific private functions
	static void SetActiveWindow(PlatformWindowType hWnd) { _activeWindow = hWnd; }
	static PlatformWindowType CreateWindow(int width, int height, bool fullscreen);

	static PlatformModuleType LoadModule(const char* module);
	static void* GetProcAddress(PlatformModuleType module, const char* proc);
	static void ReleaseModule(PlatformModuleType module);

	static int MainLoop();
	static void CleanUp();	
#endif

private:
	static PlatformWindowType _activeWindow;
};
