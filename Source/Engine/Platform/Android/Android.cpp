/* NekoEngine
 *
 * Mac.mm
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

#include <Engine/Engine.h>
#include <Platform/Platform.h>

PlatformWindowType Platform::_activeWindow = nullptr;

PlatformWindowType Platform::CreateWindow(int width, int height, bool fullscreen)
{
	return nullptr;
}

void Platform::SetWindowTitle(PlatformWindowType hWnd, const char* title) { (void)hWnd; (void)title; }

bool Platform::EnterFullscreen(int width, int height) { (void)width; (void)height; return true; }

MessageBoxResult Platform::MessageBox(const char* title, const char* message, MessageBoxButtons buttons, MessageBoxIcon icon)
{
/*	NSAlert *alert = [[NSAlert alloc] init];
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
		if(buttons == MessageBoxButtons::OK)*/
			return MessageBoxResult::OK;
	/*	else
			return MessageBoxResult::Yes;
	}
	else
		return MessageBoxResult::No;*/
}

void Platform::LogDebugMessage(const char* message)
{
	fprintf(stderr, "%s", message);
}

int Platform::MainLoop()
{
	return 0;
}

void Platform::CleanUp()
{
}
