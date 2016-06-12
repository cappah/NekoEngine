/* Neko Engine
 *
 * X11.cpp
 * Author: Alexandru Naiman
 *
 * X11 platform support
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

#define ENGINE_INTERNAL
#define PLATFORM_INTERNAL

#include <stdio.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/cursorfont.h>
#include <Engine/Engine.h>
#include <Platform/Platform.h>

#include "x11_icon.h"
#include "MessageBoxX11.h"

#define	None		0

#define KEYCODE_UP	111
#define KEYCODE_LEFT	113
#define KEYCODE_RIGHT	114
#define KEYCODE_DOWN	116

static Display* x_display;
static bool _pointerCaptured = false;
PlatformWindowType Platform::_activeWindow;

bool UserInterrupt()
{
    XEvent xev;
    KeySym key;
    bool userInterrupt = false;
    char text;

    // Pump all messages from X server. Keypresses are directed to keyfunc (if defined)
    while (XPending(x_display))
    {
        XNextEvent(x_display, &xev);
        if (xev.type == KeyPress)
        {
            if (XLookupString(&xev.xkey, &text, 1, &key, 0) == 1)
		Engine::Key(text, 1);
	    else
	    {
		text = 0;

		if(xev.xkey.keycode == KEYCODE_LEFT)
			text = 37;	
		else if(xev.xkey.keycode == KEYCODE_RIGHT)
			text = 39;	
		else if(xev.xkey.keycode == KEYCODE_UP)
			text = 38;	
		else if(xev.xkey.keycode == KEYCODE_DOWN)
			text = 40;
	
		Engine::Key(text, 1);
	    }
        }
	else if(xev.type == KeyRelease)
	{
            if (XLookupString(&xev.xkey, &text, 1, &key, 0) == 1)
		Engine::Key(text, 0);
	    else
	    {
		text = 0;

		if(xev.xkey.keycode == KEYCODE_LEFT)
			text = 37;	
		else if(xev.xkey.keycode == KEYCODE_RIGHT)
			text = 39;	
		else if(xev.xkey.keycode == KEYCODE_UP)
			text = 38;	
		else if(xev.xkey.keycode == KEYCODE_DOWN)
			text = 40;
	
		Engine::Key(text, 0);
	    }
	}
	else if(xev.type == ConfigureNotify)
	{
		XConfigureEvent xce = xev.xconfigure;

		if (xce.width != Engine::GetConfiguration().Engine.ScreenWidth || xce.height != Engine::GetConfiguration().Engine.ScreenHeight)
			Engine::ScreenResized(xce.width, xce.height);
	}

        if (xev.type == DestroyNotify)
            userInterrupt = true;
    }

    return userInterrupt;
}

void HideCursor(Display *dpy, Window win)
{
	Cursor cur;
	Pixmap cursorBitmap;
	XColor black;
	static char cursorData[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	black.red = black.green = black.blue = 0;

	cursorBitmap = XCreateBitmapFromData(dpy, win, cursorData, 8, 8);
	cur = XCreatePixmapCursor(dpy, cursorBitmap, cursorBitmap, &black, &black, 0, 0);

	XDefineCursor(dpy, win, cur);
	XFreeCursor(dpy, cur);
	XFreePixmap(dpy, cursorBitmap);
}

void ShowCursor(Display *dpy, Window win)
{
	Cursor cur = XCreateFontCursor(dpy, XC_left_ptr);
	XDefineCursor(dpy, win, cur);
	XFreeCursor(dpy, cur);
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
	Screen *screen;

	x_display = XOpenDisplay(NULL);
	
	if(!x_display)
		return 0;

	screen = XDefaultScreenOfDisplay(x_display);
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

bool Platform::CapturePointer()
{
	if(!x_display)
		return false;

	HideCursor(x_display, _activeWindow);
	
	XSetInputFocus(x_display, _activeWindow, RevertToPointerRoot, CurrentTime);
	XGrabPointer(x_display, _activeWindow, False, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, _activeWindow, None, CurrentTime);	

	XFlush(x_display);
	_pointerCaptured = true;

	return true;
}

void Platform::ReleasePointer()
{
	if(!x_display)
		return;

	XUngrabPointer(x_display, CurrentTime);

	ShowCursor(x_display, _activeWindow);

	XFlush(x_display);
	_pointerCaptured = false;
}

bool Platform::GetPointerPosition(long& x, long& y)
{
	Window retWindow;
	int root_x, root_y, win_x, win_y;
	unsigned int mask;

	bool ret = XQueryPointer(x_display, _activeWindow, &retWindow, &retWindow, &root_x, &root_y, &win_x, &win_y, &mask);

	x = win_x;
	y = win_y;

	return ret;
}

bool Platform::SetPointerPosition(long x, long y)
{
	bool ret = XWarpPointer(x_display, None, _activeWindow, 0, 0, 0, 0, (int)x, (int)y);
	XFlush(x_display);
	return ret;
}

bool Platform::GetTouchMovementDelta(float &x, float &y)
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
		ReleasePointer();

	int x = MessageBoxX11(title, message, type);

	if(capture)
		CapturePointer();

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
	while(!UserInterrupt())
		Engine::Frame();

	XFlush(x_display);
	
	return 0;
}

void Platform::CleanUp()
{
	XDestroyWindow(x_display, _activeWindow);
	XCloseDisplay(x_display);
}
