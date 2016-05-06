/* Neko Engine
 *
 * Mac.mm
 * Author: Alexandru Naiman
 *
 * MacOS X platform support
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

#include <Engine/Engine.h>
#include <Platform/Platform.h>

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#import "EngineView.h"
#import "EngineAppDelegate.h"

@interface EngineApp : NSObject

- (void)frame;

@end

@implementation EngineApp

- (void)frame
{
	Engine::Frame();
}

@end

static EngineApp *_engineApp;
static EngineView *_engineView;
static EngineAppDelegate *_engineAppDelegate;

PlatformWindowType Platform::_activeWindow = nullptr;

PlatformWindowType Platform::CreateWindow(int width, int height, bool fullscreen)
{
	NSWindow *hWnd = nil;
	NSMenu *menuBar = nil, *appMenu = nil;
	NSMenuItem *appMenuItem = nil, *quitMenuItem = nil;
	
	NSApplicationLoad();
	[NSApplication sharedApplication];
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	
	if((_engineAppDelegate = [[EngineAppDelegate alloc] init]) == nil)
	{ DIE("Failed to initialize EngineAppDelegate"); }
	[[NSApplication sharedApplication] setDelegate:_engineAppDelegate];
	
	if((menuBar = [[NSMenu alloc] init]) == nil)
	{ DIE("Failed to create menu"); }
	if((appMenuItem = [[NSMenuItem alloc] init]) == nil)
	{ DIE("Failed to create app menu item"); }
	
	[menuBar addItem:appMenuItem];
	[NSApp setMainMenu:menuBar];
	
	if((appMenu = [[NSMenu alloc] init]) == nil)
	{ DIE("Failed to create app menu"); }
	if((quitMenuItem = [[NSMenuItem alloc] initWithTitle:@"Quit NekoEngine" action:@selector(terminate:) keyEquivalent:@"q"]) == nil)
	{ DIE("Failed to create quit menu item"); }
	[appMenu addItem:quitMenuItem];
	[appMenuItem setSubmenu:appMenu];
	
	NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask /*| NSFullScreenWindowMask*/;
	
	if((hWnd = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, width, height) styleMask:styleMask backing:NSBackingStoreBuffered defer:false]) == nil)
	{ DIE("Failed to create window"); }
	[hWnd cascadeTopLeftFromPoint:NSMakePoint(20, 20)];
	[hWnd setTitle:@"NekoEngine"];
	[hWnd setCollectionBehavior:[hWnd collectionBehavior] | NSWindowCollectionBehaviorFullScreenPrimary];
	[hWnd makeKeyAndOrderFront:nil];
	
	if((_engineView = [[EngineView alloc] initWithFrame:((NSView *)hWnd.contentView).frame]) == nil)
	{ DIE("Failed to initialize EngineView"); }
	[hWnd setContentView:_engineView];
	[hWnd setInitialFirstResponder:_engineView];
	[hWnd makeFirstResponder:_engineView];
	
	[hWnd center];	
	
	if(fullscreen)
		[hWnd toggleFullScreen:nil];
	
	if((_engineApp = [[EngineApp alloc] init]) == nil)
	{ DIE("Failed to initialize EngineApp"); }
	
	return hWnd;
}

void Platform::SetWindowTitle(PlatformWindowType hWnd, const char* title)
{
	[hWnd setTitle:[NSString stringWithUTF8String:title]];
}

bool Platform::EnterFullscreen(int width, int height)
{
	[_activeWindow toggleFullScreen:nil];
	
	return true;
}

bool Platform::CapturePointer()
{
	[NSCursor hide];
	return true;
}

void Platform::ReleasePointer()
{
	[NSCursor unhide];
}

bool Platform::GetPointerPosition(long& x, long& y)
{
	NSPoint position = [_activeWindow mouseLocationOutsideOfEventStream];
	
	x = position.x;
	y = _engineView.frame.size.height - position.y - 1;

	return true;
}

bool Platform::SetPointerPosition(long x, long y)
{
	NSRect globalPosition = [_activeWindow convertRectToScreen:NSMakeRect(x, _engineView.frame.size.height - y - 1, 0, 0)];
	
	CGAssociateMouseAndMouseCursorPosition(false);
	CGWarpMouseCursorPosition(CGPointMake(globalPosition.origin.x, CGDisplayBounds(CGMainDisplayID()).size.height - globalPosition.origin.y));
	CGAssociateMouseAndMouseCursorPosition(true);
	
	return true;
}

MessageBoxResult Platform::MessageBox(const char* title, const char* message, MessageBoxButtons buttons, MessageBoxIcon icon)
{
	NSAlert *alert = [[NSAlert alloc] init];
	if(!alert)
	{ DIE("Failed to create NSAlert"); }
	
	switch (buttons)
	{
		case MessageBoxButtons::YesNo:
		{
			[alert addButtonWithTitle:@"Yes"];
			[alert addButtonWithTitle:@"No"];
		}
		break;
		case MessageBoxButtons::OK:
		default:
			[alert addButtonWithTitle:@"OK"];
		break;
	}
	
	[alert setMessageText:[NSString stringWithUTF8String:message]];
	
	switch (icon)
	{
		case MessageBoxIcon::Warning:
			[alert setAlertStyle:NSWarningAlertStyle];
		break;
		case MessageBoxIcon::Error:
			[alert setAlertStyle:NSCriticalAlertStyle];
		break;
		case MessageBoxIcon::Question:
			[alert setAlertStyle:NSInformationalAlertStyle];
		break;
		case MessageBoxIcon::Information:
		default:
			[alert setAlertStyle:NSInformationalAlertStyle];
		break;
	}
	
	if([alert runModal] == NSAlertFirstButtonReturn)
	{
		if(buttons == MessageBoxButtons::OK)
			return MessageBoxResult::OK;
		else
			return MessageBoxResult::Yes;
	}
	else
		return MessageBoxResult::No;
}

void Platform::LogDebugMessage(const char* message)
{
	NSLog(@"%s", message);
}

int Platform::MainLoop()
{
	[NSTimer scheduledTimerWithTimeInterval:1.0/60.0 target:_engineApp selector:@selector(frame) userInfo:nil repeats:true];
	
	[NSApp activateIgnoringOtherApps:true];
	[NSApp run];
	
	return 0;
}

void Platform::CleanUp()
{
}
