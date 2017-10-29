/* NekoEngine
 *
 * Platform.h
 * Author: Alexandru Naiman
 *
 * Platform specific functions
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

#include <Engine/Defs.h>
#include <Platform/PlatformDetect.h>

#include <stddef.h>

#include <vector>

#ifdef ENGINE_INTERNAL
	#include <vulkan/vulkan.h>
#endif

#ifdef _WIN32
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#undef CreateWindow
#undef MessageBox

typedef HWND PlatformWindowType;
typedef HDC PlatformDisplayType;
typedef HMODULE PlatformModuleType;

#define NWM_SHOWCURSOR_MSG_GUID	"NWM_SHOWCURSOR_{916fcbf2-b4be-4df8-884b-f0dc086e03ad}"
#define NWM_HIDECURSOR_MSG_GUID	"NWM_HIDECURSOR_{916fcbf2-b4be-4df8-884b-f0dc086e03ad}"

#define REGISTER_USER_MESSAGE(name) UINT name = RegisterWindowMessage(name##_MSG_GUID)

#elif defined(PLATFORM_X11)
#include <X11/Xlib.h>

typedef Window PlatformWindowType;
typedef Display* PlatformDisplayType;
typedef void* PlatformModuleType;
#elif defined(NE_PLATFORM_MAC)
#import <Cocoa/Cocoa.h>

typedef NSWindow* PlatformWindowType;
typedef NSView* PlatformDisplayType;
typedef void* PlatformModuleType;
#elif defined(NE_PLATFORM_IOS)
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

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

#elif defined(NE_PLATFORM_BB10)

#include <screen/screen.h>

typedef screen_window_t PlatformWindowType;
typedef screen_context_t PlatformDisplayType;
typedef void* PlatformModuleType;

#endif

enum class MessageBoxButtons : unsigned char
{
	OK = 0,
	YesNo
};

enum class MessageBoxIcon : unsigned char
{
	Information = 0,
	Warning,
	Error,
	Question
};

enum class MessageBoxResult : unsigned char
{
	OK = 0,
	Yes,
	No
};

enum SpecialDirectory : uint8_t
{
	ApplicationData = 0,
	Documents,
	Pictures,
	Desktop,
	Music,
	Home,
	Temp
};

class Platform
{
public:

	// Platform specific public functions
	ENGINE_API static const char *GetName();
	ENGINE_API static const char *GetMachineName();
	ENGINE_API static const char *GetMachineArchitecture();
	ENGINE_API static const char *GetVersion();
	ENGINE_API static const char *GetProcessorName();
	ENGINE_API static uint32_t GetProcessorFrequency();
	ENGINE_API static int32_t GetNumberOfProcessors();
	ENGINE_API static uint64_t GetProcessMemory();
	ENGINE_API static uint64_t GetUsedSystemMemory();
	ENGINE_API static uint64_t GetFreeSystemMemory();
	ENGINE_API static uint64_t GetTotalSystemMemory();

	ENGINE_API static int GetSpecialDirectoryPath(SpecialDirectory directory, char *buff, uint32_t buffLen);

	ENGINE_API static PlatformWindowType GetActiveWindow() { return _activeWindow; }

	ENGINE_API static void SetWindowTitle(PlatformWindowType hWnd, const char* title);
	ENGINE_API static bool EnterFullscreen(int width, int height);

	ENGINE_API static MessageBoxResult MessageBox(const char* title, const char* message, MessageBoxButtons buttons, MessageBoxIcon icon);

	ENGINE_API static void LogDebugMessage(const char* message);

	ENGINE_API static void Sleep(uint32_t seconds);
	ENGINE_API static void USleep(uint32_t microseconds);
	
	ENGINE_API static void Terminate();

	// Shared
	ENGINE_API static size_t GetConfigString(const char *section, const char *entry, const char *def, char *buffer, int buffer_len, const char *file);
	ENGINE_API static int GetConfigInt(const char *section, const char *entry, int def, const char *file);
	ENGINE_API static float GetConfigFloat(const char *section, const char *entry, float def, const char *file);
	ENGINE_API static double GetConfigDouble(const char *section, const char *entry, double def, const char *file);
	ENGINE_API static size_t GetConfigSection(const char *section, char *out, size_t size, const char *file);
	ENGINE_API static uint32_t Rand();
	ENGINE_API static uint32_t RandRange(uint32_t max);
	ENGINE_API static float RandRange(float max);
	ENGINE_API static void Exit();
	ENGINE_API static void Restart();

#ifdef ENGINE_INTERNAL // Platform specific private functions
	static int Initialize();

	static void SetActiveWindow(PlatformWindowType hWnd) { _activeWindow = hWnd; }
	static PlatformWindowType CreateWindow(int width, int height, bool fullscreen);

	static PlatformModuleType LoadModule(const char *module);
	static void *GetProcAddress(PlatformModuleType module, const char *proc);
	static void ReleaseModule(PlatformModuleType module);
	static std::vector<const char*> GetRequiredExtensions(bool debug);
	static bool CreateSurface(VkInstance instance, VkSurfaceKHR &surface, PlatformWindowType hWnd, VkAllocationCallbacks *allocator = nullptr);

	static int MainLoop();
	static void CleanUp();	
#endif

private:
	static PlatformWindowType _activeWindow;
	static bool _exit;
};
