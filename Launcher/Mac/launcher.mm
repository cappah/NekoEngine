/* Neko Engine
 *
 * launcher.mm
 * Author: Alexandru Naiman
 *
 * Launcher entry point for Macintosh systems
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

#include <Engine/Engine.h>
#include <Platform/Platform.h>
#include <string>

void CleanUp()
{
	Engine::CleanUp();
}

int main(int argc, char *argv[])
{
	atexit(CleanUp);
	
	@autoreleasepool
	{
		std::string args("");

		for(int i = 0; i < argc; i++)
		{
			args.append(argv[i]);
			args.append(" ");
		}
		
		args.append("--renderer=libMacGLRenderer.dylib --game=libTestGame.dylib ");
		
		args.append("--data=");
		args.append([[[NSBundle mainBundle] resourcePath] UTF8String]);
		args.append("/Data --ini=");
		args.append([[[NSBundle mainBundle] resourcePath] UTF8String]);
		args.append("/Engine.ini --log=");
		
		NSArray *urls = [[NSFileManager defaultManager] URLsForDirectory:NSLibraryDirectory inDomains:NSUserDomainMask];
		NSURL *logUrl = [[urls lastObject] URLByAppendingPathComponent:@"Logs"];
		logUrl = [logUrl URLByAppendingPathComponent:@"NekoEngine.log"];
		args.append([[logUrl path] UTF8String]);

		if (Engine::Initialize(args, false) != ENGINE_OK)
		{
			printf("Failed to initialize engine.\n");			
			Platform::MessageBox("Fatal error", "Failed to initialize engine", MessageBoxButtons::OK, MessageBoxIcon::Error);
			return -1;
		}

		Engine::Run();
	}

	return 0;
}
