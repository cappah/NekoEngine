/* NekoEngine
 *
 * UNIX.cpp
 * Author: Alexandru Naiman
 *
 * UNIX platform support
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

#include <unistd.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <Platform/Platform.h>

#include <string>

using namespace std;

static struct utsname uname_data;

const char* Platform::GetName()
{
	uname(&uname_data);
	return uname_data.sysname;
}

const char* Platform::GetMachineName()
{
	uname(&uname_data);
	return uname_data.nodename;
}

const char* Platform::GetMachineArchitecture()
{
	uname(&uname_data);
	return uname_data.machine;
}

const char* Platform::GetVersion()
{
	uname(&uname_data);
	return uname_data.release;
}

int Platform::GetNumberOfProcessors()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

PlatformModuleType Platform::LoadModule(const char* module)
{
	char path[1024];
	string prefix, suffix;

	if (access(module, R_OK) < 0)
	{
		// Module file does not exits; attempt to add "lib" and the shared library extension.

		if (strncmp(module, "lib", 3))
			prefix = "lib";

		size_t len = strlen(module);

#ifdef __APPLE__
		if (len > 7)
		{
			if(strncmp(module + len - 6, ".dylib", 6))
				suffix = ".dylib";
		}
		else
			suffix = ".dylib";
#else
		if (len > 4)
		{
			if(strncmp(module + len - 3, ".so", 3))
				suffix = ".so";
		}
		else
			suffix = ".so";
#endif
	}

	snprintf(path, 1024, "%s%s%s", prefix.c_str(), module, suffix.c_str());

	void *handle = dlopen(path, RTLD_NOW);

	if(!handle)
		fprintf(stderr, "dlopen() error %s\n", dlerror());

	return handle;
}

void* Platform::GetProcAddress(PlatformModuleType module, const char* proc)
{
	return dlsym(module, proc);
}

void Platform::ReleaseModule(PlatformModuleType module)
{
	dlclose(module);
}
