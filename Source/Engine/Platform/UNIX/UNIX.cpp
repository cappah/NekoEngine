/* NekoEngine
 *
 * UNIX.cpp
 * Author: Alexandru Naiman
 *
 * UNIX platform support
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

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <sys/vfs.h>
#include <sys/signal.h>
#include <sys/utsname.h>
#include <System/Logger.h>
#include <Platform/Platform.h>

#include <string>

using namespace std;

static struct utsname uname_data;
static string _nix_cpuName{};
extern char *ne_executable_name;

#define NIX_PLATFORM_MODULE	"Unix_Platform"

int Platform::Initialize()
{
	return ENGINE_OK;
}

const char *Platform::GetName()
{
	uname(&uname_data);
	return uname_data.sysname;
}

const char *Platform::GetMachineName()
{
	uname(&uname_data);
	return uname_data.nodename;
}

const char *Platform::GetMachineArchitecture()
{
	uname(&uname_data);
	return uname_data.machine;
}

const char *Platform::GetVersion()
{
	uname(&uname_data);
	return uname_data.release;
}

const char *Platform::GetProcessorName()
{
	char buff[512]{};

	if (_nix_cpuName.length())
		return _nix_cpuName.c_str();

	FILE *fp{ fopen("/proc/cpuinfo", "r") };
	if (!fp)
	{
		_nix_cpuName = "Unknown";
		return "Unkown";
	}

	while (fgets(buff, 512, fp))
	{
		if (strstr(buff, "model name"))
		{
			char *ptr{ strchr(buff, ':') };
			if (!ptr)
				_nix_cpuName = "Unknown";
			else
				_nix_cpuName = (ptr + 2);
			break;
		}
		memset(buff, 0x0, 512);
	}

	fclose(fp);

	return _nix_cpuName.c_str();
}

uint32_t Platform::GetProcessorFrequency()
{
	return 0;
}

int32_t Platform::GetNumberOfProcessors()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

uint64_t Platform::GetProcessMemory()
{
	return 0;
}

uint64_t Platform::GetUsedSystemMemory()
{
	long pages{ sysconf(_SC_PHYS_PAGES) };
	long avPages{ sysconf(_SC_AVPHYS_PAGES) };
	long pageSize{ sysconf(_SC_PAGE_SIZE) };
	return (pages * pageSize) - (avPages * pageSize);
}

uint64_t Platform::GetFreeSystemMemory()
{
	long pages{ sysconf(_SC_AVPHYS_PAGES) };
	long pageSize{ sysconf(_SC_PAGE_SIZE) };
	return pages * pageSize;
}

uint64_t Platform::GetTotalSystemMemory()
{
	long pages{ sysconf(_SC_PHYS_PAGES) };
	long pageSize{ sysconf(_SC_PAGE_SIZE) };
	return pages * pageSize;
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

void Platform::Sleep(uint32_t seconds)
{
	(void)sleep(seconds);
}

void Platform::USleep(uint32_t microseconds)
{
	(void)usleep(microseconds);
}

void Platform::Restart()
{
	pid_t child = fork();

	if (child < 0)
	{
		Logger::Log(NIX_PLATFORM_MODULE, LOG_CRITICAL, "fork() failed, errno: %d", errno);
		Terminate();
	}

	if (!child) Terminate();

	execl(ne_executable_name, "", NULL);

	// execl never returns
	// if we are here, an error has occured
	Logger::Log(NIX_PLATFORM_MODULE, LOG_CRITICAL, "execl() failed, errno: %d", errno);
	Terminate();
}

void Platform::Terminate()
{
	raise(SIGKILL);
}
