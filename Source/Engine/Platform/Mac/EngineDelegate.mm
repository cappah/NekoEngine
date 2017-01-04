/* NekoEngine
 *
 * EngineDelegate.mm
 * Author: Alexandru Naiman
 *
 * macOS platform support
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

#import "EngineDelegate.h"

#include <Engine/Engine.h>

@implementation EngineDelegate 

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	// Insert code here to initialize your application
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
	Engine::CleanUp();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
	return true;
}

- (void)applicationDidChangeOcclusionState:(NSNotification *)notification
{
	// pause when hidden ?
	// https://developer.apple.com/library/mac/documentation/Cocoa/Reference/ApplicationKit/Classes/NSApplication_Class/index.html#//apple_ref/c/data/NSApplicationDidChangeOcclusionStateNotification
}

- (void)windowDidResize:(NSNotification *)notification
{
	NSWindow *window = Platform::GetActiveWindow();
	CGSize size = window.contentView.frame.size;
	Engine::ScreenResized(size.width, size.height);
}

@end
