/* NekoEngine
 *
 * Windows.cpp
 * Author: Alexandru Naiman
 *
 * Windows platform support
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

#define VK_USE_PLATFORM_WIN32_KHR

#include <Engine/Engine.h>
#include <Engine/Input.h>
#include <System/Logger.h>
#include <Platform/Platform.h>

#include <dbt.h>
#include <Psapi.h>
#include <ShlObj.h>
#include <Wbemidl.h>

#include "../Source/Launcher/resource.h"

using namespace std;

#define _WIN32_TOTAL_MEMORY			0
#define _WIN32_USED_MEMORY			1
#define _WIN32_FREE_MEMORY			2
#define _WIN32_PROCESS_MEMORY		3

#define WIN32_PLATFORM_MODULE		"Win32_Platform"

REGISTER_USER_MESSAGE(NWM_SHOWCURSOR);
REGISTER_USER_MESSAGE(NWM_HIDECURSOR);

PlatformWindowType Platform::_activeWindow = nullptr;

static LPCSTR WindowClassName = "NekoEngineWindowClass";
static HINSTANCE hEngineInstance;

static char _win32_name[512]{ 0x0 };
static char _win32_machineName[512]{ 0x0 };
static char _win32_version[512]{ 0x0 };
static char _win32_processorName[512]{ 0x0 };
static char _win32_architecture[512]{ 0x0 };
static uint32_t _win32_numberOfProcessors{ 0 };
static uint32_t _win32_processorFrequency{ 0 };
static uint64_t _win32_totalSystemMemory{ 0 };
static IWbemLocator *_win32_wbemLocator{ nullptr };
static IWbemServices *_win32_wbemServices{ nullptr };

static inline WPARAM _win32MapKeys(WPARAM vk, LPARAM lParam)
{
	UINT scan = (lParam & 0x00FF0000) >> 16;
	int ext = (lParam & 0x01000000) != 0;

	switch (vk)
	{
		case VK_SHIFT: return MapVirtualKey(scan, MAPVK_VSC_TO_VK_EX);
		case VK_CONTROL: return ext ? VK_RCONTROL : VK_LCONTROL;
		case VK_MENU: return ext ? VK_RMENU : VK_LMENU;
		default: return vk;
	}
}

int _win32Rand()
{
	HCRYPTPROV hCtx = 0;
	int ret = 0;

	if (!CryptAcquireContext(&hCtx, NULL, NULL, PROV_RSA_FULL, 0))
	{
		if (GetLastError() == NTE_BAD_KEYSET)
		{
			if (!CryptAcquireContext(&hCtx, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET))
			{ DIE("CryptAcquireContext failed"); }
		}
		else
		{ DIE("CryptAcquireContext failed"); }
	}

	if(!CryptGenRandom(hCtx, sizeof(int), (BYTE *)&ret))
	{ DIE("CryptGenRandom failed"); }

	return ret;
}

static inline uint64_t _win32_GetMemoryStatus(int type)
{
	if (type == _WIN32_PROCESS_MEMORY)
	{
		PROCESS_MEMORY_COUNTERS pmc{};
		GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));

		return pmc.WorkingSetSize / 1024;
	}

	MEMORYSTATUSEX memStatus{};
	memStatus.dwLength = sizeof(memStatus);

	GlobalMemoryStatusEx(&memStatus);

	if (type == _WIN32_TOTAL_MEMORY)
		return memStatus.ullTotalPhys / 1024;
	else if (type == _WIN32_FREE_MEMORY)
		return memStatus.ullAvailPhys / 1024;
	else if (type == _WIN32_USED_MEMORY)
		return (memStatus.ullTotalPhys - memStatus.ullAvailPhys) / 1024;

	return 0;
}

LRESULT CALLBACK EngineWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet{ 0 };
	PAINTSTRUCT ps{};
	HDC hdc{};

	switch (uMsg)
	{
		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		break;
		case WM_ERASEBKGND:
			return 0; // ignore
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_KEYDOWN:
		{
			Input::Key((int)_win32MapKeys(wParam, lParam), true);
			return 0;
		}
		case WM_KEYUP:
		{
			Input::Key((int)_win32MapKeys(wParam, lParam), false);
			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			Input::Key(NE_MOUSE_LMB, true);
			return 0;
		}
		case WM_LBUTTONUP:
		{
			Input::Key(NE_MOUSE_LMB, false);
			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			Input::Key(NE_MOUSE_RMB, true);
			return 0;
		}
		case WM_RBUTTONUP:
		{
			Input::Key(NE_MOUSE_RMB, false);
			return 0;
		}
		case WM_MBUTTONDOWN:
		{
			Input::Key(NE_MOUSE_MMB, true);
			return 0;
		}
		case WM_MBUTTONUP:
		{
			Input::Key(NE_MOUSE_MMB, false);
			return 0;
		}
		case WM_XBUTTONDOWN:
		{
			Input::Key(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? NE_MOUSE_BTN4 : NE_MOUSE_BTN5, true);
			return 0;
		}
		case WM_XBUTTONUP:
		{
			Input::Key(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? NE_MOUSE_BTN4 : NE_MOUSE_BTN5, false);
			return 0;
		}
		case WM_SIZE:
			Engine::ScreenResized(LOWORD(lParam), HIWORD(lParam));
		break;
		case WM_SYSCOMMAND:
		{
			// Prevent monitor from turning off & screen saver from starting
			if (wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER)
				return 0;
			else if (wParam == SC_RESTORE)
				ShowWindow(hWnd, SW_RESTORE); // Required to show a minimized fullscreen window
		}
		break;
		case WM_DEVICECHANGE:
		{
			if (wParam != DBT_DEVNODES_CHANGED)
				break;

			// TODO: update controller list
		}
		default:
		{
			if (uMsg == NWM_SHOWCURSOR)
			{
				ShowCursor(true);
				return 0;
			}
			else if (uMsg == NWM_HIDECURSOR)
			{
				ShowCursor(false);
				return 0;
			}
		}
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int Platform::Initialize()
{
	HRESULT hr{ 0 };

	hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		Logger::Log(WIN32_PLATFORM_MODULE, LOG_CRITICAL, "Failed to initialize the COM library: 0x%x", hr);
		return ENGINE_FAIL;
	}

	hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
							  RPC_C_AUTHN_LEVEL_DEFAULT,
							  RPC_C_IMP_LEVEL_IMPERSONATE,
							  NULL, EOAC_NONE, NULL);
	if (FAILED(hr))
	{
		Logger::Log(WIN32_PLATFORM_MODULE, LOG_CRITICAL, "Failed to initialize COM security: 0x%x", hr);
		return ENGINE_FAIL;
	}

	// Initialize WMI
	hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&_win32_wbemLocator);
	if (FAILED(hr))
	{
		Logger::Log(WIN32_PLATFORM_MODULE, LOG_CRITICAL, "Failed to create IWbemLocator: 0x%x", hr);
		return ENGINE_FAIL;
	}

	hr = _win32_wbemLocator->ConnectServer(L"ROOT\\CIMV2", NULL, NULL, 0, NULL, 0, 0, &_win32_wbemServices);
	if (FAILED(hr))
	{
		Logger::Log(WIN32_PLATFORM_MODULE, LOG_CRITICAL, "Failed to connect IWbemServices: 0x%x", hr);
		return ENGINE_FAIL;
	}

	hr = CoSetProxyBlanket(_win32_wbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
						   RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	if (FAILED(hr))
	{
		Logger::Log(WIN32_PLATFORM_MODULE, LOG_CRITICAL, "Failed to set proxy blanket: 0x%x", hr);
		return ENGINE_FAIL;
	}

	IEnumWbemClassObject *enumerator{ nullptr };
	hr = _win32_wbemServices->ExecQuery(L"WQL", L"SELECT * FROM Win32_Processor",
										WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
										NULL, &enumerator);
	if (FAILED(hr))
	{
		Logger::Log(WIN32_PLATFORM_MODULE, LOG_CRITICAL, "Failed to query processor name: 0x%x", hr);
		return ENGINE_FAIL;
	}

	IWbemClassObject *classObject{ nullptr };
	ULONG ret{ 0 };

	while (enumerator)
	{
		hr = enumerator->Next(WBEM_INFINITE, 1, &classObject, &ret);
		if (ret == 0)
			break;

		VARIANT vt{};

		classObject->Get(L"Name", 0, &vt, 0, 0);
		wcstombs(_win32_processorName, vt.bstrVal, 512);
		VariantClear(&vt);

		classObject->Get(L"MaxClockSpeed", 0, &vt, 0, 0);
		_win32_processorFrequency = vt.uintVal;
		VariantClear(&vt);

		classObject->Release();
	}

	enumerator->Release();

	// Load platform info

	// Machine name
	DWORD size = 512;
	GetComputerNameA(_win32_machineName, &size);

	// Platform version
	{
		char ver[256], build[256];
		memset(ver, 0x0, 256);
		memset(build, 0x0, 256);

		HKEY key{ 0 };
		DWORD size{ sizeof(ver) };

		RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &key);
		RegQueryValueExA(key, "CurrentVersion", 0, NULL, (LPBYTE)ver, &size);

		size = sizeof(build);
		RegQueryValueExA(key, "CurrentBuild", 0, NULL, (LPBYTE)build, &size);
		RegCloseKey(key);

		if (snprintf(_win32_version, 512, "%s.%s", ver, build) >= 512)
		{
			#ifdef _DEBUG
			OutputDebugStringA("ERROR: Platform version too long");
			#endif
		}
	}

	// Machine architecture
	{
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);

		switch (sysInfo.wProcessorArchitecture)
		{
			case PROCESSOR_ARCHITECTURE_AMD64: snprintf(_win32_architecture, 512, "x86_64"); break;
			case PROCESSOR_ARCHITECTURE_ARM: snprintf(_win32_architecture, 512, "arm"); break;
			case PROCESSOR_ARCHITECTURE_IA64: snprintf(_win32_architecture, 512, "ia64"); break;
			case PROCESSOR_ARCHITECTURE_INTEL: snprintf(_win32_architecture, 512, "x86"); break;
			default: snprintf(_win32_architecture, 512, "Unknown"); break;
		}

		// Number of processors
		_win32_numberOfProcessors = sysInfo.dwNumberOfProcessors;
	}

	// Platform name
	{
		char *platform{ nullptr }, *type{ nullptr }, *suite{ nullptr };
		OSVERSIONINFOEXA ver{};
		ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
		GetVersionExA((OSVERSIONINFOA *)&ver);

		if (ver.dwPlatformId == VER_PLATFORM_WIN32s)
			platform = "Win32s";
		else if (ver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
			platform = "Windows";
		else if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT)
			platform = "Windows NT";

		if (ver.wSuiteMask & VER_SUITE_EMBEDDEDNT)
			suite = " Embedded";

		if (ver.wProductType != VER_NT_WORKSTATION)
			type = " Server";

		snprintf(_win32_name, 512, "%s%s%s", platform, type ? type : "", suite ? suite : "");
	}

	_win32_totalSystemMemory = _win32_GetMemoryStatus(_WIN32_TOTAL_MEMORY);

	return ENGINE_OK;
}

const char *Platform::GetName()
{
	return _win32_name;
}

const char *Platform::GetMachineName()
{
	return _win32_machineName;
}

const char *Platform::GetMachineArchitecture()
{
	return _win32_architecture;
}

const char *Platform::GetVersion()
{
	return _win32_version;
}

const char *Platform::GetProcessorName()
{
	return _win32_processorName;
}

uint32_t Platform::GetProcessorFrequency()
{
	return _win32_processorFrequency;
}

int32_t Platform::GetNumberOfProcessors()
{
	return _win32_numberOfProcessors;
}

uint64_t Platform::GetProcessMemory()
{
	return _win32_GetMemoryStatus(_WIN32_PROCESS_MEMORY);
}

uint64_t Platform::GetUsedSystemMemory()
{
	return _win32_GetMemoryStatus(_WIN32_USED_MEMORY);
}

uint64_t Platform::GetFreeSystemMemory()
{
	return _win32_GetMemoryStatus(_WIN32_FREE_MEMORY);
}

uint64_t Platform::GetTotalSystemMemory()
{
	return _win32_totalSystemMemory;
}

int Platform::GetSpecialDirectoryPath(SpecialDirectory directory, char *buff, uint32_t buffLen)
{
	int csidl{};
	
	if (directory == SpecialDirectory::Temp)
	{
		if (!GetTempPathA(buffLen, buff))
			return ENGINE_FAIL;

		return ENGINE_OK;
	}

	switch(directory)
	{
		case SpecialDirectory::ApplicationData: csidl = CSIDL_LOCAL_APPDATA; break;
		case SpecialDirectory::Documents: csidl = CSIDL_MYDOCUMENTS; break;
		case SpecialDirectory::Pictures: csidl = CSIDL_MYPICTURES; break;
		case SpecialDirectory::Desktop: csidl = CSIDL_DESKTOP; break;
		case SpecialDirectory::Music: csidl = CSIDL_MYMUSIC; break;
		case SpecialDirectory::Home: csidl = CSIDL_PROFILE; break;
		default: return ENGINE_FAIL;
	};

	HRESULT hr{ SHGetFolderPathA(NULL, csidl, NULL, SHGFP_TYPE_CURRENT, buff) };
	
	if (FAILED(hr))
		return ENGINE_FAIL;

	return ENGINE_OK;
}

PlatformWindowType Platform::CreateWindow(int width, int height, bool fullscreen)
{
	HWND hWnd{ 0 };
	WNDCLASS wndClass{ 0 };
	DWORD    wStyle{ 0 };
	DWORD    wExStyle{ 0 };
	RECT     windowRect{};
	int posX{ 0 }, posY{ 0 };

	hEngineInstance = GetModuleHandle(NULL);

	wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndClass.lpfnWndProc = (WNDPROC)EngineWindowProc;
	wndClass.hInstance = hEngineInstance;
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszClassName = WindowClassName;
	wndClass.hIcon = LoadIcon(hEngineInstance, MAKEINTRESOURCE(IDI_APPICON));

	if (!RegisterClass(&wndClass))
		return nullptr;

	if (fullscreen)
	{
		wExStyle = WS_EX_APPWINDOW;
		wStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		wExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		wStyle = WS_VISIBLE | WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME;

		int screenX = GetSystemMetrics(SM_CXSCREEN),
			screenY = GetSystemMetrics(SM_CYSCREEN);

		posX = (screenX - width) / 2;
		posY = (screenY - height) / 2;
	}

	// Adjust the window rectangle so that the client area has
	// the correct number of pixels
	windowRect.left = 0;
	windowRect.top = 0;
	windowRect.right = width;
	windowRect.bottom = height;

	AdjustWindowRectEx(&windowRect, wStyle, FALSE, wExStyle);

	hWnd = CreateWindowEx(
		wExStyle,
		WindowClassName,
		"NekoEngine",
		wStyle,
		posX,
		posY,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		hEngineInstance,
		NULL);

	if (hWnd == NULL)
		return nullptr;

	ShowWindow(hWnd, Engine::IsEditor() ? SW_SHOWMINIMIZED : SW_SHOW);
	SetForegroundWindow(hWnd);

	if (fullscreen)
		EnterFullscreen(width, height);

	return hWnd;
}

void Platform::SetWindowTitle(PlatformWindowType hWnd, const char* title)
{
	SetWindowTextA(hWnd, title);
}

bool Platform::EnterFullscreen(int width, int height)
{
	DEVMODE dmScreenSettings;
	memset(&dmScreenSettings, 0x0, sizeof(dmScreenSettings));

	dmScreenSettings.dmSize = sizeof(dmScreenSettings);
	dmScreenSettings.dmPelsWidth = width;
	dmScreenSettings.dmPelsHeight = height;
	dmScreenSettings.dmBitsPerPel = 32;
	dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		return false;

	return true;
}

MessageBoxResult Platform::MessageBox(const char* title, const char* message, MessageBoxButtons buttons, MessageBoxIcon icon)
{
	UINT x{ 0 }, type{ 0 };

	switch (buttons)
	{
		case MessageBoxButtons::YesNo:
			type = MB_YESNO;
		break;
		case MessageBoxButtons::OK:
		default:
			type = MB_OK;
		break;
	}

	switch (icon)
	{
		case MessageBoxIcon::Warning:
			type |= MB_ICONWARNING;
		break;
		case MessageBoxIcon::Error:
			type |= MB_ICONERROR;
		break;
		case MessageBoxIcon::Question:
			type |= MB_ICONQUESTION;
		break;
		case MessageBoxIcon::Information:
		default:
			type |= MB_ICONINFORMATION;
		break;
	}

	x = MessageBoxA(_activeWindow, message, title, type);

	if (x == IDYES)
		return MessageBoxResult::Yes;

	if (x == IDNO)
		return MessageBoxResult::No;

	return MessageBoxResult::OK;
}

void Platform::LogDebugMessage(const char* message)
{
	if (!IsDebuggerPresent())
	{
		fprintf(stdout, "%s%c", message, (message[strlen(message) - 1] == '\n') ? '\0' : '\n');
		return;
	}

	OutputDebugStringA(message);

	if (message[strlen(message) - 1] != '\n')
		OutputDebugStringA("\n");
}

PlatformModuleType Platform::LoadModule(const char* module)
{
	return LoadLibraryA(module);
}

void* Platform::GetProcAddress(PlatformModuleType module, const char* proc)
{
	return ::GetProcAddress(module, proc);
}

void Platform::ReleaseModule(PlatformModuleType module)
{
	FreeLibrary(module);
}

int Platform::MainLoop()
{
	MSG msg = { 0 };
	int ret = 0;

	PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE);

	while (!_exit && msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
			Engine::Frame();
	}

	return ret;
}

void Platform::CleanUp()
{
	CoUninitialize();

	DestroyWindow(_activeWindow);
	UnregisterClass(WindowClassName, hEngineInstance);
}

vector<const char*> Platform::GetRequiredExtensions(bool debug)
{
	vector<const char*> extensions;
	extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	extensions.push_back("VK_KHR_win32_surface");

	if (debug)
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	return extensions;
}

bool Platform::CreateSurface(VkInstance instance, VkSurfaceKHR &surface, PlatformWindowType hWnd, VkAllocationCallbacks *allocator)
{
	VkResult err{};
	VkWin32SurfaceCreateInfoKHR createInfo{};
	PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR{ nullptr };

	vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
	if (!vkCreateWin32SurfaceKHR)
	{
		Logger::Log("Platform", LOG_CRITICAL, "Vulkan instance missing VK_KHR_win32_surface extension");
		return false;
	}

	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hinstance = GetModuleHandle(NULL);
	createInfo.hwnd = hWnd;

	if ((err = vkCreateWin32SurfaceKHR(instance, &createInfo, allocator, &surface)))
	{
		Logger::Log("Platform", LOG_CRITICAL, "Failed to create Vulkan surface: %d", err);
		return false;
	}

	return true;
}

void Platform::Sleep(uint32_t seconds)
{
	::Sleep(seconds * 1000);
}

void Platform::USleep(uint32_t microseconds)
{
	::Sleep(microseconds / 1000);
}

void Platform::Terminate()
{
	TerminateProcess(GetCurrentProcess(), -1);
}