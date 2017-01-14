/* NekoEngine
 *
 * CrashHandler.cpp
 * Author: Alexandru Naiman
 *
 * Windows Crash Handler Implementation
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

#include <System/Logger.h>
#include <Platform/CrashHandler.h>
#include <Platform/PlatformDetect.h>

#include <Psapi.h>
#include <DbgHelp.h>

#define WIN32_CH_MODULE		"Win32_CrashHandler"

static UINT8 *_win32_stackTraceBuffer{ nullptr };
static UINT32 _win32_stackTraceBufferSize{ 0 };
static UINT32 _win32_numEntries{ 0 };
static DWORD64 _win32_rawStackTrace[CH_MAX_STACKTRACE_ENTRIES];

struct CoreDumpParams
{
	char *fileName;
	EXCEPTION_POINTERS *ep;
};

typedef bool(WINAPI *EnumProcessModulesProc)(HANDLE hProcess, HMODULE* lphModule, DWORD cb, LPDWORD lpcbNeeded);
typedef DWORD(WINAPI *GetModuleBaseNameProc)(HANDLE hProcess, HMODULE hModule, LPSTR lpBaseName, DWORD nSize);
typedef DWORD(WINAPI *GetModuleFileNameExProc)(HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize);
typedef bool(WINAPI *GetModuleInformationProc)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb);

static HMODULE _win32_PSAPIModule{ nullptr };
static EnumProcessModulesProc _win32_EnumProcessModules{ nullptr };
static GetModuleBaseNameProc _win32_GetModuleBaseName{ nullptr };
static GetModuleFileNameExProc _win32_GetModuleFileNameEx{ nullptr };
static GetModuleInformationProc _win32_GetModuleInformation{ nullptr };

DWORD CALLBACK _win32_writeCoreDump(void *params)
{
	HANDLE hFile{ CreateFileA(((CoreDumpParams *)params)->fileName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr) };

	MINIDUMP_EXCEPTION_INFORMATION info{};
	info.ThreadId = GetCurrentThreadId();
	info.ExceptionPointers = ((CoreDumpParams *)params)->ep;
	info.ClientPointers = false;

	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &info, nullptr, nullptr);

	CloseHandle(hFile);

	return 0;
}

int CrashHandler::Initialize()
{
	_win32_stackTraceBufferSize = sizeof(PIMAGEHLP_SYMBOL64) + CH_MAX_STACKTRACE_NAME_BYTES;

	_win32_stackTraceBuffer = (UINT8 *)calloc(_win32_stackTraceBufferSize, sizeof(UINT8));
	if (!_win32_stackTraceBuffer)
		return ENGINE_OUT_OF_RESOURCES;

	_win32_PSAPIModule = LoadLibraryA("PSAPI.dll");
	_win32_EnumProcessModules = (EnumProcessModulesProc)GetProcAddress(_win32_PSAPIModule, "EnumProcessModules");
	_win32_GetModuleBaseName = (GetModuleBaseNameProc)GetProcAddress(_win32_PSAPIModule, "GetModuleBaseNameA");
	_win32_GetModuleFileNameEx = (GetModuleFileNameExProc)GetProcAddress(_win32_PSAPIModule, "GetModuleFileNameExA");
	_win32_GetModuleInformation = (GetModuleInformationProc)GetProcAddress(_win32_PSAPIModule, "GetModuleInformation");

	HANDLE hProcess{ GetCurrentProcess() };
	UINT32 options{ SymGetOptions() };

	options |= SYMOPT_LOAD_LINES;
	options |= SYMOPT_EXACT_SYMBOLS;
	options |= SYMOPT_UNDNAME;
	options |= SYMOPT_FAIL_CRITICAL_ERRORS;
	options |= SYMOPT_NO_PROMPTS;

	SymSetOptions(options);
	if (!SymInitialize(hProcess, nullptr, false))
		return ENGINE_FAIL;

	DWORD bufferSize{};
	_win32_EnumProcessModules(hProcess, nullptr, 0, &bufferSize);

	HMODULE *modules = (HMODULE *)calloc(1, bufferSize);
	_win32_EnumProcessModules(hProcess, modules, bufferSize, &bufferSize);

	UINT32 num{ bufferSize / sizeof(HMODULE) };
	for (UINT32 i = 0; i < num; ++i)
	{
		MODULEINFO moduleInfo{};

		char moduleName[CH_MAX_STACKTRACE_NAME_BYTES];
		char imageName[CH_MAX_STACKTRACE_NAME_BYTES];

		_win32_GetModuleInformation(hProcess, modules[i], &moduleInfo, sizeof(moduleInfo));
		_win32_GetModuleBaseName(hProcess, modules[i], moduleName, CH_MAX_STACKTRACE_NAME_BYTES);
		_win32_GetModuleFileNameEx(hProcess, modules[i], imageName, CH_MAX_STACKTRACE_NAME_BYTES);

		char pdbPath[CH_MAX_STACKTRACE_NAME_BYTES];
		char *fileName{ nullptr };
		GetFullPathNameA(moduleName, CH_MAX_STACKTRACE_NAME_BYTES, pdbPath, &fileName);
		*fileName = 0x0;

		SymSetSearchPath(GetCurrentProcess(), pdbPath);

		DWORD64 address = SymLoadModule64(hProcess, modules[i], imageName, moduleName, (DWORD64)moduleInfo.lpBaseOfDll, (DWORD)moduleInfo.SizeOfImage);

		if (!address)
		{
			Logger::Log(WIN32_CH_MODULE, LOG_WARNING, "Failed to load symbols for module: %s", moduleName);
			continue;
		}

		IMAGEHLP_MODULE64 imageInfo{};
		ZeroMemory(&imageInfo, sizeof(imageInfo));
		imageInfo.SizeOfStruct = sizeof(imageInfo);

		if (!SymGetModuleInfo64(GetCurrentProcess(), address, &imageInfo))
		{
			Logger::Log(WIN32_CH_MODULE, LOG_WARNING, "Failed to load info for module: %s", moduleName);
			continue;
		}
	}

	free(modules);

	return ENGINE_OK;
}

NString CrashHandler::GetStackTrace()
{
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();
	CONTEXT ctx{};
	UINT32 machineType{ 0 };
	STACKFRAME64 stackFrame{};
	ZeroMemory(&stackFrame, sizeof(STACKFRAME64));

	_win32_numEntries = 0;

	RtlCaptureContext(&ctx);

	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrStack.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Mode = AddrModeFlat;

#if defined(NE_ARCH_X8664)
	stackFrame.AddrPC.Offset = ctx.Rip;
	stackFrame.AddrStack.Offset = ctx.Rsp;
	stackFrame.AddrFrame.Offset = ctx.Rbp;

	machineType = IMAGE_FILE_MACHINE_AMD64;
#elif defined(NE_ARCH_X86)
	stackFrame.AddrPC.Offset = ctx.Eip;
	stackFrame.AddrStack.Offset = ctx.Esp;
	stackFrame.AddrFrame.Offset = ctx.Ebp;

	machineType = IMAGE_FILE_MACHINE_I386;
#endif

	while (1)
	{
		if (!StackWalk64(machineType, hProcess, hThread, &stackFrame, &ctx, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
			break;

		_win32_rawStackTrace[_win32_numEntries++] = stackFrame.AddrPC.Offset;

		if (_win32_numEntries == CH_MAX_STACKTRACE_ENTRIES || stackFrame.AddrPC.Offset == 0 || stackFrame.AddrFrame.Offset == 0)
			break;
	}

	PIMAGEHLP_SYMBOL64 symbol = (PIMAGEHLP_SYMBOL64)_win32_stackTraceBuffer;
	symbol->SizeOfStruct = _win32_stackTraceBufferSize;
	symbol->MaxNameLength = CH_MAX_STACKTRACE_NAME_BYTES;

	NString output{};

	// Skip GetStackTrace() & SaveCrashDump()
	for (UINT32 i = 4; i < _win32_numEntries; ++i)
	{
		DWORD64 address = _win32_rawStackTrace[i];

		DWORD64 displacement{ 0 };
		if (SymGetSymFromAddr64(hProcess, address, &displacement, symbol))
			output.AppendFormat(CH_MAX_STACKTRACE_NAME_BYTES, "%s() - ", symbol->Name);

		IMAGEHLP_LINE64 line{};
		line.SizeOfStruct = sizeof(line);

		NString addressString = NString::StringWithFormat(50, "0x%08x", address);

		DWORD col{};
		if (SymGetLineFromAddr64(hProcess, address, &col, &line))
			output.AppendFormat(CH_MAX_STACKTRACE_NAME_BYTES, "0x%08x:\n\t[File: %s]\n\t[Line: %d]\n\t[Column: %d]", address, line.FileName, line.LineNumber, col);
		else
			output.AppendFormat(CH_MAX_STACKTRACE_NAME_BYTES, "0x%08x", address);

		IMAGEHLP_MODULE64 module{};
		module.SizeOfStruct = sizeof(module);

		if (SymGetModuleInfo64(hProcess, address, &module))
			output.AppendFormat(CH_MAX_STACKTRACE_NAME_BYTES, "\n\t[Module: %s]", module.ImageName);

		output.Append('\n');
	}

	return output;
}

NString CrashHandler::GetErrorString(void *params)
{
	EXCEPTION_POINTERS *ep{ (EXCEPTION_POINTERS *)params };

	switch (ep->ExceptionRecord->ExceptionCode)
	{
		case EXCEPTION_ACCESS_VIOLATION:
		{
			if (ep->ExceptionRecord->NumberParameters == 2)
			{
				if (ep->ExceptionRecord->ExceptionInformation[0] == 0)
					return NString::StringWithFormat(512, "Exception at 0x%x. Access violation reading location 0x%x.",
													 ep->ExceptionRecord->ExceptionAddress, ep->ExceptionRecord->ExceptionInformation[1]);
				else if (ep->ExceptionRecord->ExceptionInformation[0] == 8)
					return NString::StringWithFormat(512, "Exception at 0x%x. DEP access violation at location 0x%x.",
													 ep->ExceptionRecord->ExceptionAddress, ep->ExceptionRecord->ExceptionInformation[1]);
				else
					return NString::StringWithFormat(512, "Exception at 0x%x. Access violation writing location 0x%x.",
													 ep->ExceptionRecord->ExceptionAddress, ep->ExceptionRecord->ExceptionInformation[1]);
			}
			else
				return NString::StringWithFormat(512, "Exception at 0x%x. Access violation.", ep->ExceptionRecord->ExceptionAddress);
		}
		case EXCEPTION_IN_PAGE_ERROR:
		{
			if (ep->ExceptionRecord->NumberParameters == 3)
			{
				if (ep->ExceptionRecord->ExceptionInformation[0] == 0)
					return NString::StringWithFormat(512, "Exception at 0x%x. Page fault reading location 0x%x, code 0x%x.",
													 ep->ExceptionRecord->ExceptionAddress, ep->ExceptionRecord->ExceptionInformation[1], ep->ExceptionRecord->ExceptionInformation[3]);
				else if (ep->ExceptionRecord->ExceptionInformation[0] == 8)
					return NString::StringWithFormat(512, "Exception at 0x%x. DEP page fault at location 0x%x, code 0x%x.",
													 ep->ExceptionRecord->ExceptionAddress, ep->ExceptionRecord->ExceptionInformation[1], ep->ExceptionRecord->ExceptionInformation[3]);
				else
					return NString::StringWithFormat(512, "Exception at 0x%x. Page fault writing location 0x%x, code 0x%x.",
													 ep->ExceptionRecord->ExceptionAddress, ep->ExceptionRecord->ExceptionInformation[1], ep->ExceptionRecord->ExceptionInformation[3]);
			}
			else
				return NString::StringWithFormat(512, "Exception at 0x%x. Page fault.", ep->ExceptionRecord->ExceptionAddress);
		}
		case STATUS_ARRAY_BOUNDS_EXCEEDED:
			return NString::StringWithFormat(512, "Exception at 0x%x. Attempt to access an out of bounds element.", ep->ExceptionRecord->ExceptionAddress);
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			return NString::StringWithFormat(512, "Exception at 0x%x. Misaligned data access.", ep->ExceptionRecord->ExceptionAddress);
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			return NString::StringWithFormat(512, "Exception at 0x%x. Floating point operand too small", ep->ExceptionRecord->ExceptionAddress);
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			return NString::StringWithFormat(512, "Exception at 0x%x. Floating point divide by zero.", ep->ExceptionRecord->ExceptionAddress);
		case EXCEPTION_FLT_INVALID_OPERATION:
			return NString::StringWithFormat(512, "Exception at 0x%x. Floating point invalid operation", ep->ExceptionRecord->ExceptionAddress);
		case EXCEPTION_FLT_OVERFLOW:
			return NString::StringWithFormat(512, "Exception at 0x%x. Floating point overflow.", ep->ExceptionRecord->ExceptionAddress);
		case EXCEPTION_FLT_UNDERFLOW:
			return NString::StringWithFormat(512, "Exception at 0x%x. Floating point underflow.", ep->ExceptionRecord->ExceptionAddress);
		case EXCEPTION_FLT_STACK_CHECK:
			return NString::StringWithFormat(512, "Exception at 0x%x. Floating point stack check.", ep->ExceptionRecord->ExceptionAddress);
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			return NString::StringWithFormat(512, "Exception at 0x%x. Attempt to execute illegal instruction.", ep->ExceptionRecord->ExceptionAddress);
		case EXCEPTION_PRIV_INSTRUCTION:
			return NString::StringWithFormat(512, "Exception at 0x%x. Attempt to execute private instruction.", ep->ExceptionRecord->ExceptionAddress);
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			return NString::StringWithFormat(512, "Exception at 0x%x. Integer divide by zero.", ep->ExceptionRecord->ExceptionAddress);
		case EXCEPTION_INT_OVERFLOW:
			return NString::StringWithFormat(512, "Exception at 0x%x. Integer overflow.", ep->ExceptionRecord->ExceptionAddress);
		case EXCEPTION_STACK_OVERFLOW:
			return NString::StringWithFormat(512, "Exception at 0x%x. Stack overflow.", ep->ExceptionRecord->ExceptionAddress);
		default:
			return NString::StringWithFormat(512, "Exception at 0x%x. Code 0x%x.", ep->ExceptionRecord->ExceptionAddress, ep->ExceptionRecord->ExceptionCode);
	}
}

void CrashHandler::SaveCoreDump(char *coreDumpFile, void *params)
{
	DWORD threadId{ 0 };
	CoreDumpParams cdParams{ coreDumpFile, (EXCEPTION_POINTERS *)params };
	HANDLE hThread{ CreateThread(nullptr, 0, _win32_writeCoreDump, &cdParams, 0, &threadId) };

	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
}

void CrashHandler::Cleanup()
{
	free(_win32_stackTraceBuffer);
	FreeLibrary(_win32_PSAPIModule);
}