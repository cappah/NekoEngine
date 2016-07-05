/* Neko Engine
 *
 * iOS.mm
 * Author: Alexandru Naiman
 *
 * iOS platform support
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

#define ENGINE_INTERNAL
#define PLATFORM_INTERNAL

#include <Engine/Engine.h>
#include <Platform/Platform.h>

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#import "EngineAppDelegate.h"
#import "EngineInputDelegate.h"

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
static EngineAppDelegate *_engineAppDelegate;
static EngineInputDelegate *_engineInputDelegate;

PlatformWindowType Platform::_activeWindow = nullptr;

PlatformWindowType Platform::CreateWindow(int width, int height, bool fullscreen)
{
	if((_engineApp = [[EngineApp alloc] init]) == nil)
	{ DIE("Failed to initialize EngineApp"); }
	
	if((_engineInputDelegate = [[EngineInputDelegate alloc] init]) == nil)
	{ DIE("Failed to initialize EngineInputDelegate"); }
	
	CGRect rect = [[UIScreen mainScreen] bounds];
    
	Engine::GetConfiguration().Engine.ScreenWidth = rect.size.width;
	Engine::GetConfiguration().Engine.ScreenHeight = rect.size.height;
	
	return [[[UIApplication sharedApplication] delegate] window];
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
	x = -_engineInputDelegate.xDelta * .5;
	y = -_engineInputDelegate.yDelta * .5;
	
	_engineInputDelegate.xDelta = 0;
	_engineInputDelegate.yDelta = 0;
	
	return true;
}

MessageBoxResult Platform::MessageBox(const char* title, const char* message, MessageBoxButtons buttons, MessageBoxIcon icon)
{
	MessageBoxResult ret = MessageBoxResult::No;
	
	UIAlertController *alert = [UIAlertController alertControllerWithTitle:[NSString stringWithUTF8String:title] message:[NSString stringWithUTF8String:message] preferredStyle:UIAlertControllerStyleAlert];
	if(!alert)
	{ DIE("Failed to create UIAlertController"); }
	
	switch (buttons)
	{
		case MessageBoxButtons::YesNo:
		{
			UIAlertAction *yesAction = [UIAlertAction actionWithTitle:@"Yes" style:UIAlertActionStyleDefault handler:^(UIAlertAction * action) {}];
			UIAlertAction *noAction = [UIAlertAction actionWithTitle:@"Yes" style:UIAlertActionStyleCancel handler:^(UIAlertAction * action) {}];
			
			[alert addAction:yesAction];
			[alert addAction:noAction];
		}
		break;
		case MessageBoxButtons::OK:
		default:
		{
			UIAlertAction *action = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:^(UIAlertAction * action) {}];
			[alert addAction:action];
		}
		break;
	}
	
	[[[[UIApplication sharedApplication] keyWindow] rootViewController] presentViewController:alert animated:true completion:nil];
	
	return ret;
}

void Platform::LogDebugMessage(const char* message)
{
	NSLog(@"%s", message);
}

int Platform::MainLoop()
{
	id<EngineViewProtocol> engineView = (id<EngineViewProtocol>)[[[[[UIApplication sharedApplication] delegate] window] rootViewController] view];	
	[engineView setInputDelegate:_engineInputDelegate];
	
	[NSTimer scheduledTimerWithTimeInterval:1.0/60.0 target:_engineApp selector:@selector(frame) userInfo:nil repeats:true];
	
	return 0;
}

void Platform::CleanUp()
{
}
