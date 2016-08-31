/* NekoEngine
 *
 * Windows.cpp
 * Author: Alexandru Naiman
 *
 * Windows platform support
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

#include <Engine/Engine.h>
#include <Engine/Input.h>
#include <Platform/Platform.h>

#include "../Launcher/resource.h"

PlatformWindowType Platform::_activeWindow = nullptr;

static LPCSTR WindowClassName = "NekoEngineWindowClass";
static HINSTANCE hEngineInstance;

static char _name[512] = { 0 };
static char _machineName[512] = { 0 };
static char _version[512] = { 0 };

PIXELFORMATDESCRIPTOR pfd =
{
	sizeof(PIXELFORMATDESCRIPTOR),
	1,
	PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
	PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
	32,                        //Colordepth of the framebuffer.
	0, 0, 0, 0, 0, 0,
	0,
	0,
	0,
	0, 0, 0, 0,
	24,                        //Number of bits for the depthbuffer
	8,                        //Number of bits for the stencilbuffer
	0,                        //Number of Aux buffers in the framebuffer.
	PFD_MAIN_PLANE,
	0,
	0, 0, 0
};

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
	HCRYPTPROV hCtx;
	int ret;

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

LRESULT CALLBACK EngineWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet = 0;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (uMsg)
	{
		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		break;
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
			// Will be used for controller support
		}
		default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

const char* Platform::GetName()
{
	if (_name[0] != 0)
		return _name;

	HKEY key;
	DWORD size = sizeof(_name);

	RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &key);
	RegQueryValueExA(key, "ProductName", 0, NULL, (LPBYTE)_name, &size);
	RegCloseKey(key);

	return _name;
}

const char* Platform::GetMachineName()
{
	if (_machineName[0] != 0)
		return _machineName;

	DWORD size = 512;
	GetComputerNameA(_machineName, &size);

	return _machineName;
}

const char* Platform::GetMachineArchitecture()
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	
	switch(sysInfo.wProcessorArchitecture)
	{
		case PROCESSOR_ARCHITECTURE_AMD64: return "x86_64";
		case PROCESSOR_ARCHITECTURE_ARM: return "arm";
		case PROCESSOR_ARCHITECTURE_IA64: return "ia64";
		case PROCESSOR_ARCHITECTURE_INTEL: return "x86";
		default: return "Unknown";
	}
}

const char* Platform::GetVersion()
{
	if (_version[0] != 0)
		return _version;

	char ver[256], build[256];

	HKEY key;
	DWORD size = sizeof(ver);

	RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &key);
	RegQueryValueExA(key, "CurrentVersion", 0, NULL, (LPBYTE)ver, &size);

	size = sizeof(build);
	RegQueryValueExA(key, "CurrentBuild", 0, NULL, (LPBYTE)build, &size);
	RegCloseKey(key);

	if (snprintf(_version, 512, "%s.%s", ver, build) >= 512)
	{
#ifdef _DEBUG
		OutputDebugStringA("ERROR: Platform version too long");
#endif
	}

	return _version;
}

PlatformWindowType Platform::CreateWindow(int width, int height, bool fullscreen)
{
	HWND hWnd;
	WNDCLASS wndclass = { 0 };
	DWORD    wStyle = 0;
	DWORD    wExStyle = 0;
	RECT     windowRect;
	int posX = 0, posY = 0;

	hEngineInstance = GetModuleHandle(NULL);

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = (WNDPROC)EngineWindowProc;
	wndclass.hInstance = hEngineInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszClassName = WindowClassName;
	wndclass.hIcon = LoadIcon(hEngineInstance, MAKEINTRESOURCE(IDI_APPICON));

	if (!RegisterClass(&wndclass))
		return nullptr;

	if (fullscreen)
	{
		wExStyle = WS_EX_APPWINDOW;
		wStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		wExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		wStyle = WS_VISIBLE | WS_OVERLAPPEDWINDOW;

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

	ShowWindow(hWnd, Engine::IsEditor() ? SW_SHOWMINIMIZED : SW_SHOWDEFAULT);

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
	UINT x, type = 0;

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

	while (msg.message != WM_QUIT)
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
	DestroyWindow(_activeWindow);
	UnregisterClass(WindowClassName, hEngineInstance);
}

void Platform::Exit()
{
	PostQuitMessage(0);
}
