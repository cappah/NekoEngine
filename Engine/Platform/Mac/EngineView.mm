/* Neko Engine
 *
 * EngineView.mm
 * Author: Alexandru Naiman
 *
 * macOS platform support
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

#import <Carbon/Carbon.h>

#import "EngineView.h"

#include <Engine/Input.h>
#include <Engine/Engine.h>

// Taken from:
// http://stackoverflow.com/questions/1918841/how-to-convert-ascii-character-to-cgkeycode/1971027#1971027
CFStringRef createStringForKey(CGKeyCode keyCode)
{
	TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
	CFDataRef layoutData = (CFDataRef)TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
	const UCKeyboardLayout *keyboardLayout = (const UCKeyboardLayout *)CFDataGetBytePtr(layoutData);
	
	UInt32 keysDown = 0;
	UniChar chars[4];
	UniCharCount realLength;
	
	UCKeyTranslate(keyboardLayout,
				   keyCode,
				   kUCKeyActionDisplay,
				   0,
				   LMGetKbdType(),
				   kUCKeyTranslateNoDeadKeysBit,
				   &keysDown,
				   sizeof(chars) / sizeof(chars[0]),
				   &realLength,
				   chars);
	CFRelease(currentKeyboard);
	
	return CFStringCreateWithCharacters(kCFAllocatorDefault, chars, 1);
}

@implementation EngineView

@synthesize mouseLocation;

- (void)keyUp:(NSEvent *)theEvent
{
	Input::Key(theEvent.keyCode, false);
}

- (void)keyDown:(NSEvent *)theEvent
{
	Input::Key(theEvent.keyCode, true);
}

- (void)mouseUp:(NSEvent *)theEvent
{
	[super mouseUp:theEvent];
}

- (void)mouseDown:(NSEvent *)theEvent
{
	[super mouseDown:theEvent];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	[self mouseMoved:theEvent];
}

- (void)mouseMoved:(NSEvent *)theEvent
{	
	self.mouseLocation = NSMakePoint(theEvent.absoluteX, theEvent.absoluteY);
}

- (BOOL)acceptsFirstResponder
{
	return true;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
	return true;
}

- (BOOL)becomeFirstResponder
{
	return true;
}

@end
