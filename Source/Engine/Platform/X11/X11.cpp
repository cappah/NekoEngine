/* NekoEngine
 *
 * X11.cpp
 * Author: Alexandru Naiman
 *
 * X11 platform support
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

#define VK_USE_PLATFORM_XLIB_KHR

#include <stdio.h>

#include <X11/X.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <Engine/Input.h>
#include <Engine/Engine.h>
#include <System/Logger.h>
#include <Platform/Platform.h>

#include "x11_icon.h"
#include "MessageBoxX11.h"

using namespace std;

Display *x_display;
PlatformWindowType Platform::_activeWindow;
extern bool _pointerCaptured;

bool UserInterrupt()
{
	XEvent xev;
	bool userInterrupt = false;

	while (XPending(x_display))
	{
		XNextEvent(x_display, &xev);

		if (xev.type == KeyPress)
			Input::Key(XLookupKeysym(&xev.xkey, 0), 1);
		else if(xev.type == KeyRelease)
		{
			if(XEventsQueued(x_display, QueuedAfterReading))
			{
			   XEvent nxev;
			   XPeekEvent(x_display, &nxev);
			   
			   if(nxev.type == KeyPress && nxev.xkey.time == xev.xkey.time && nxev.xkey.keycode == xev.xkey.keycode)
				   return userInterrupt;
			}
			
			Input::Key(XLookupKeysym(&xev.xkey, 0), 0);
		}
		else if(xev.type == ConfigureNotify)
		{
			XConfigureEvent xce = xev.xconfigure;

			if ((uint32_t)xce.width != Engine::GetConfiguration().Engine.ScreenWidth || (uint32_t)xce.height != Engine::GetConfiguration().Engine.ScreenHeight)
				Engine::ScreenResized(xce.width, xce.height);
		}

		if (xev.type == DestroyNotify)
			userInterrupt = true;
	}

	return userInterrupt;
}

PlatformWindowType Platform::CreateWindow(int width, int height, bool fullscreen)
{
	int posX = 0, posY = 0;
	Window root;
	XSetWindowAttributes swa;
	XSetWindowAttributes xattr;
	Atom wm_state;
	XWMHints hints;
	XEvent xev;
	Window win;

	x_display = XOpenDisplay(NULL);
	
	if(!x_display)
		return 0;

	root = DefaultRootWindow(x_display);

	swa.event_mask  =  ExposureMask | PointerMotionMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask;

	win = XCreateWindow(
		x_display, root,
		posX, posY, width, height, 0,
		CopyFromParent, InputOutput,
		CopyFromParent, CWEventMask,
		&swa);

	xattr.override_redirect = False;
	XChangeWindowAttributes(x_display, win, CWOverrideRedirect, &xattr);

	hints.input = True;
	hints.flags = InputHint;
	XSetWMHints(x_display, win, &hints);

	// make the window visible on the screen
	XMapWindow(x_display, win);
	XStoreName(x_display, win, "NekoEngine");
	
	XSync(x_display, false);
	
	// get identifiers for the provided atom name strings
	wm_state = XInternAtom (x_display, "_NET_WM_STATE", False);

	memset(&xev, 0, sizeof(xev));
	xev.type                 = ClientMessage;
	xev.xclient.window       = win;
	xev.xclient.message_type = wm_state;
	xev.xclient.format       = 32;
	xev.xclient.data.l[0]    = 1;
	xev.xclient.data.l[1]    = False;

	XSendEvent(x_display,
		DefaultRootWindow(x_display),
		False,
		SubstructureNotifyMask,
		&xev);

	Atom _NET_WM_ICON = XInternAtom(x_display, "_NET_WM_ICON", False);
	Atom cardinal = XInternAtom(x_display, "CARDINAL", False);

	XChangeProperty(x_display, win, _NET_WM_ICON, cardinal, 32, PropModeReplace, (const unsigned char *)X11_ICON, sizeof(X11_ICON) / 8);

	return win;
}

void Platform::SetWindowTitle(PlatformWindowType hWnd, const char* title)
{
	XStoreName(x_display, hWnd, title);
}

bool Platform::EnterFullscreen(int width, int height)
{
	return false;
}

MessageBoxResult Platform::MessageBox(const char* title, const char* message, MessageBoxButtons buttons, MessageBoxIcon icon)
{
	bool capture = _pointerCaptured;
	unsigned int type;
	
	switch (buttons)
	{
		case MessageBoxButtons::YesNo:
			type = X11_MSG_BOX_BTN_YESNO;
		break;
		case MessageBoxButtons::OK:
		default:
			type = X11_MSG_BOX_BTN_OK;
		break;
	}

	switch (icon)
	{
		case MessageBoxIcon::Warning:
			type |= X11_MSG_BOX_ICON_WARNING;
		break;
		case MessageBoxIcon::Error:
			type |= X11_MSG_BOX_ICON_ERROR;
		break;
		case MessageBoxIcon::Question:
			type |= X11_MSG_BOX_ICON_QUESTION;
		break;
		case MessageBoxIcon::Information:
		default:
		type |= X11_MSG_BOX_ICON_INFORMATION;
		break;
	}

	if(_pointerCaptured)
		Input::ReleasePointer();

	int x = MessageBoxX11(title, message, type);

	if(capture)
		Input::CapturePointer();

	if (x == X11_MSG_BOX_RET_YES)
		return MessageBoxResult::Yes;
	
	if (x == X11_MSG_BOX_RET_NO)
		return MessageBoxResult::No;

	return MessageBoxResult::OK;
}

void Platform::LogDebugMessage(const char* message)
{
	fprintf(stderr, "%s%c", message, message[strlen(message) - 1] == '\n' ? '\0' : '\n');
}

int Platform::MainLoop()
{
	while(!_exit && !UserInterrupt())
		Engine::Frame();

	XFlush(x_display);
	
	return 0;
}

vector<const char*> Platform::GetRequiredExtensions(bool debug)
{
	vector<const char*> extensions;
	extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);

	if (debug)
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	return extensions;
}

bool Platform::CreateSurface(VkInstance instance, VkSurfaceKHR &surface, PlatformWindowType hWnd, VkAllocationCallbacks *allocator)
{
	VkResult err;
	VkXlibSurfaceCreateInfoKHR createInfo{};
	PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;

	vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");
	if (!vkCreateXlibSurfaceKHR)
	{
		Logger::Log("Platform", LOG_CRITICAL, "Vulkan instance missing %s extension", VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
		return false;
	}

	createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	createInfo.dpy = x_display;
	createInfo.window = hWnd;

	if ((err = vkCreateXlibSurfaceKHR(instance, &createInfo, allocator, &surface)))
	{
		Logger::Log("Platform", LOG_CRITICAL, "Failed to create Vulkan surface: %d", err);
		return false;
	}

	return true;
}

void Platform::CleanUp()
{
	XDestroyWindow(x_display, _activeWindow);
//	XCloseDisplay(x_display); <-- segfaults
}
