/* NekoEngine
 *
 * launcher.cpp
 * Author: Alexandru Naiman
 *
 * Launcher entry point for Windows
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

#include <Windows.h>
#include <Engine/Engine.h>

#include <stdexcept>

using namespace std;

void CleanUp()
{
	Engine::CleanUp();

#ifdef _DEBUG
	if (!IsDebuggerPresent())
		FreeConsole();
#endif
}

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	try
	{
		atexit(CleanUp);

		string str(lpCmdLine);

#ifdef _DEBUG
		if (!IsDebuggerPresent() && (str.find("--noconsole") == string::npos))
		{
			FreeConsole();
			AllocConsole();
			AttachConsole(GetCurrentProcessId());

			freopen("CON", "w", stdout);
			freopen("CON", "w", stderr);

			system("title NekoEngine Debug Console");
		}
#endif

		_CrtSetDbgFlag(0);

		if(str.find("--waitrdoc") != string::npos)
			MessageBoxA(HWND_DESKTOP, "Press OK after RenderDoc injection", "Waiting for RenderDoc", MB_OK);

		if (Engine::Initialize(lpCmdLine, false) != ENGINE_OK)
		{
			MessageBoxA(HWND_DESKTOP, "Failed to initialize engine. The application will now exit.", "Fatal error", MB_ICONERROR | MB_OK);
			return -1;
		}

		return Engine::Run();
	}
	catch (exception e)
	{
		char buff[4096];
		snprintf(buff, 4096, "Runtime error: %s\nThe application will now exit.", e.what());
		MessageBoxA(HWND_DESKTOP, buff, "Fatal error", MB_ICONERROR | MB_OK);
		return -1;
	}
}
