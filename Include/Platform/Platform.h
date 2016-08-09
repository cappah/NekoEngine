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

#define ENGINE_API

typedef Window PlatformWindowType;
typedef Display* PlatformDisplayType;
typedef void* PlatformModuleType;
#elif defined(NE_PLATFORM_MAC)
#import <Cocoa/Cocoa.h>

#define ENGINE_API

typedef NSWindow* PlatformWindowType;
typedef NSView* PlatformDisplayType;
typedef void* PlatformModuleType;
#elif defined(NE_PLATFORM_IOS)
#import <UIKit/UIKit.h>

#define ENGINE_API

typedef UIWindow* PlatformWindowType;
typedef UIView* PlatformDisplayType;
typedef void* PlatformModuleType;

@protocol EngineInputDelegateProtocol <NSObject>

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event;

@end

@protocol EngineViewProtocol <NSObject>

- (void)setInputDelegate:(id<EngineInputDelegateProtocol>)delegate;

@end

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

	// Platform specific
	static const char* GetName();
	static const char* GetMachineName();
	static const char* GetMachineArchitecture();
	static const char* GetVersion();
	
	static void SetActiveWindow(PlatformWindowType hWnd) { _activeWindow = hWnd; }
	static PlatformWindowType GetActiveWindow() { return _activeWindow; }

	static PlatformWindowType CreateWindow(int width, int height, bool fullscreen);
	static void SetWindowTitle(PlatformWindowType hWnd, const char* title);	
	static bool EnterFullscreen(int width, int height);

	static bool CapturePointer();
	static void ReleasePointer();
	static bool GetPointerPosition(long &x, long &y);
	static bool SetPointerPosition(long x, long y);
	static bool GetTouchMovementDelta(float &x, float &y);

	static MessageBoxResult MessageBox(const char* title, const char* message, MessageBoxButtons buttons, MessageBoxIcon icon);

	static void LogDebugMessage(const char* message);

	static PlatformModuleType LoadModule(const char* module);
	static void* GetProcAddress(PlatformModuleType module, const char* proc);
	static void ReleaseModule(PlatformModuleType module);

	static int MainLoop();

	static void CleanUp();

	static void Exit();

	// Shared
	static size_t GetConfigString(const char *section, const char *entry, const char *def, char *buffer, int buffer_len, const char *file);
	static int GetConfigInt(const char *section, const char *entry, int def, const char *file);
	static float GetConfigFloat(const char *section, const char *entry, float def, const char *file);
	static double GetConfigDouble(const char *section, const char *entry, double def, const char *file);
	static size_t GetConfigSection(const char *section, char *out, size_t size, const char *file);

private:
	static PlatformWindowType _activeWindow;
};
