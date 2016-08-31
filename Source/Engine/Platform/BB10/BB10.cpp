/* NekoEngine
 *
 * BB10.cpp
 * Author: Alexandru Naiman
 *
 * BlackBerry 10 platform support
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

#include <bps/navigator.h>
#include <bps/screen.h>
#include <bps/event.h>
#include <bps/bps.h>

static screen_context_t _screenContext;
static screen_display_t _screenDisplay;
static screen_window_t _screenWindow;
static screen_event_t _screenEvent;
static bool _shouldExit = false;
static int _screenSize[2];

PlatformWindowType Platform::_activeWindow = nullptr;

static inline void _bb10_handleScreenEvent(bps_event_t *event)
{
    screen_event_t screen_event = screen_event_get_event(event);

    int screen_val;
    screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_TYPE, &screen_val);

    switch (screen_val)
    {
        case SCREEN_EVENT_MTOUCH_TOUCH:
        case SCREEN_EVENT_MTOUCH_MOVE:
        case SCREEN_EVENT_MTOUCH_RELEASE:
        break;
    }
}

static inline void _bb10_handleNavigatorEvent(bps_event_t *event)
{
    switch(bps_event_get_code(event))
    {
        case NAVIGATOR_SWIPE_DOWN:
        break;
        case NAVIGATOR_EXIT:
            _shouldExit = true;
        break;
    }
}

static inline void _bb10_handleEvents()
{
    int screenDomain = screen_get_domain();
    int navigatorDomain = screen_get_domain();
    int rc;

    for(;;)
    {
        bps_event_t *event = nullptr;
        rc = bps_get_event(&event, 0);

        if(rc != BPS_SUCCESS)
        {
           // log err
           break;
        }

        if(event)
        {
            int domain = bps_event_get_domain(event);

            if(domain == screenDomain)
                _bb10_handleScreenEvent(event);
            else
                _bb10_handleNavigatorEvent(event);
        }
        else
            break;
    }
}

PlatformWindowType Platform::CreateWindow(int width, int height, bool fullscreen)
{
    int rc = 0;
    int screenFormat = SCREEN_FORMAT_RGBX8888;
    int screenUsage = SCREEN_USAGE_OPENGL_ES3;
    int screenSwapInterval = 1;
    int screenTransparency = SCREEN_TRANSPARENCY_NONE;

    int pos[] = {0, 0};

    if(screen_create_context(&_screenContext, 0))
    {
        //
        return nullptr;
    }

    bps_initialize();

    if(screen_create_window(&_screenWindow, _screenContext))
    {
        return nullptr;
    }

    if(screen_set_window_property_iv(_screenWindow, SCREEN_PROPERTY_FORMAT, &screenFormat))
    {
        return nullptr;
    }

    if(screen_set_window_property_iv(_screenWindow, SCREEN_PROPERTY_USAGE, &screenUsage))
    {
        return nullptr;
    }

    if(screen_get_window_property_iv(_screenWindow, SCREEN_PROPERTY_SIZE, _screenSize))
    {
        return nullptr;
    }

    if(screen_set_window_property_iv(_screenWindow, SCREEN_PROPERTY_TRANSPARENCY, &screenTransparency))
    {
        return nullptr;
    }

    if(screen_set_window_property_iv(_screenWindow, SCREEN_PROPERTY_SWAP_INTERVAL, &screenSwapInterval))
    {
        return nullptr;
    }

    if(screen_create_window_buffers(_screenWindow, 2))
    {
        return nullptr;
    }

    if(screen_create_event(&_screenEvent))
    {
        return nullptr;
    }

    screen_get_window_property_pv(_screenWindow, SCREEN_PROPERTY_DISPLAY, (void **)&_screenDisplay);

    screen_request_events(_screenContext);
    navigator_request_events(0);

    return _screenWindow;
}

void Platform::SetWindowTitle(PlatformWindowType hWnd, const char* title)
{
}

bool Platform::EnterFullscreen(int width, int height)
{
	return true;
}

bool Platform::CapturePointer()
{
	return false;
}

void Platform::ReleasePointer()
{
}

bool Platform::GetPointerPosition(long &x, long &y)
{
	return false;
}

bool Platform::SetPointerPosition(long x, long y)
{
	return false;
}

bool Platform::GetTouchMovementDelta(float &x, float &y)
{


	return true;
}

MessageBoxResult Platform::MessageBox(const char* title, const char* message, MessageBoxButtons buttons, MessageBoxIcon icon)
{
	MessageBoxResult ret = MessageBoxResult::No;



	return ret;
}

void Platform::LogDebugMessage(const char* message)
{
	//NSLog(@"%s", message);
}

int Platform::MainLoop()
{
    for(;;)
    {
        _bb10_handleEvents();

        if(_shouldExit)
            break;

        Engine::Frame();
    }

    screen_stop_events(_screenContext);
    screen_destroy_window(_screenWindow);
    bps_shutdown();
    screen_destroy_context(_screenContext);

	return 0;
}

void Platform::CleanUp()
{
}

// Unused
bool Input::SetControllerVibration(int n, float left, float right) { return false; }
void Input::_InitializeKeymap() { }
int Input::_GetControllerCount() { return 0; }
bool Input::_GetControllerState(int n, ControllerState *state) { return false; }
