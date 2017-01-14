/* NekoEngine
 *
 * Shared.cpp
 * Author: Alexandru Naiman
 *
 * Shared platform functions
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

#include <assert.h>

#include <Engine/Debug.h>
#include <Engine/Engine.h>
#include <Engine/Version.h>
#include <Engine/GameModule.h>
#include <Platform/Compat.h>
#include <Platform/Platform.h>
#include <Platform/CrashHandler.h>
#include <Renderer/Renderer.h>

#define INI_LINE_BUFF	512

bool Platform::_exit = false;

size_t Platform::GetConfigString(const char *section, const char *entry, const char *def, char *buffer, int buffer_len, const char *file)
{
	FILE *fp = fopen(file, "r");
	bool found = false;
	char lineBuff[INI_LINE_BUFF], sectionBuff[INI_LINE_BUFF];
	memset(lineBuff, 0x0, INI_LINE_BUFF);
	memset(sectionBuff, 0x0, INI_LINE_BUFF);

	assert(section);
	assert(entry);
	assert(def);
	assert(buffer);
	assert(buffer_len > 0);
	assert(file);

	if (!fp)
		return -1;

	if (snprintf(sectionBuff, INI_LINE_BUFF, "[%s]", section) >= INI_LINE_BUFF)
	{
		fclose(fp);
		strncpy(buffer, def, buffer_len);
		return strlen(def);
	}
	size_t len = strlen(sectionBuff);

	while (fgets(lineBuff, INI_LINE_BUFF, fp))
	{
		if (!strncmp(lineBuff, sectionBuff, len))
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		fclose(fp);
		strncpy(buffer, def, buffer_len);
		return strlen(def);
	}

	len = strlen(entry);

	while (fgets(lineBuff, INI_LINE_BUFF, fp))
	{
		if (!strncmp(lineBuff, entry, len))
		{
			char *ptr = strchr(lineBuff, '=');
			ptr++;

			strncpy(buffer, ptr, buffer_len - 1);
			buffer[buffer_len - 1] = '\0';

			ptr = strchr(buffer, '\r');
			if (ptr)
				*ptr = '\0';

			ptr = strchr(buffer, '\n');
			if (ptr)
				*ptr = '\0';

			fclose(fp);
			return strlen(buffer);
		}
	}

	fclose(fp);
	strncpy(buffer, def, buffer_len);
	return strlen(def);
}

int Platform::GetConfigInt(const char *section, const char *entry, int def, const char *file)
{
	char buffer[20];
	memset(buffer, 0x0, 20);

	assert(section);
	assert(entry);
	assert(file);

	GetConfigString(section, entry, "noval", buffer, 20, file);

	if (!strncmp(buffer, "noval", 5))
		return def;

	return atoi(buffer);
}

float Platform::GetConfigFloat(const char *section, const char *entry, float def, const char *file)
{
	char buffer[20];
	memset(buffer, 0x0, 20);

	assert(section);
	assert(entry);
	assert(file);

	GetConfigString(section, entry, "noval", buffer, 20, file);

	if (!strncmp(buffer, "noval", 5))
		return def;

	return (float)atof(buffer);
}

double Platform::GetConfigDouble(const char *section, const char *entry, double def, const char *file)
{
	char buffer[20];
	memset(buffer, 0x0, 20);

	assert(section);
	assert(entry);
	assert(file);

	GetConfigString(section, entry, "noval", buffer, 20, file);

	if (!strncmp(buffer, "noval", 5))
		return def;

	return atof(buffer);
}

size_t Platform::GetConfigSection(const char *section, char *out, size_t size, const char *file)
{
	FILE *fp = fopen(file, "r");
	size_t offset = 0;
	bool found = false;
	char lineBuff[INI_LINE_BUFF], sectionBuff[INI_LINE_BUFF];
	memset(lineBuff, 0x0, INI_LINE_BUFF);
	memset(sectionBuff, 0x0, INI_LINE_BUFF);

	assert(section);
	assert(out);
	assert(size > 0);
	assert(file);

	if (!fp)
		return 0;

	if(snprintf(sectionBuff, INI_LINE_BUFF, "[%s]", section) >= INI_LINE_BUFF)
	{
		fclose(fp);
		return 0;
	}

	size_t len = strlen(sectionBuff);

	while (fgets(lineBuff, INI_LINE_BUFF, fp))
	{
		if (!strncmp(lineBuff, sectionBuff, len))
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		fclose(fp);
		return 0;
	}

	while (fgets(lineBuff, INI_LINE_BUFF, fp))
	{
		strncpy((out + offset), lineBuff, size - offset);
		out[size - 1] = 0x0;

		char *ptr = strchr((out + offset), '\r');
		if (ptr)
			*ptr = 0x0;

		ptr = strchr((out + offset), '\n');
		if (ptr)
			*ptr = 0x0;

		offset += strlen((out + offset)) + 1;
	}

	fclose(fp);
	return offset;
}

int Platform::Rand()
{
	return NE_RANDOM();
}

void Platform::Exit()
{
	_exit = true;
}

void CrashHandler::SaveCrashDump(void *params)
{
	const NString &stackTrace = GetStackTrace();
	const NString &errorString = GetErrorString(params);
	char timestamp[512]{}, fileTimestamp[512]{}, coreDumpFile[512]{};

	time_t currentTime{};
	time(&currentTime);

	struct tm *tmBuff{ localtime(&currentTime) };

	strftime(timestamp, 512, "%d/%m/%Y at %H:%M:%S", tmBuff);
	strftime(fileTimestamp, 512, "%d_%m_%Y_at_%H_%M_%S", tmBuff);
	snprintf(coreDumpFile, 512, "%s_%s.dmp", Engine::GetGameModule() ? Engine::GetGameModule()->GetModuleName() : "NekoEngine", fileTimestamp);

	FILE *fp{ fopen("CrashDump.txt", "a") };

	fprintf(fp, "NekoEngine Crash Dump\n");
	fprintf(fp, "Date: %s\n", timestamp);
	fprintf(fp, "EngineVersion: %s\n", ENGINE_VERSION_STRING);
	fprintf(fp, "GameModule: %s\n", Engine::GetGameModule()->GetModuleName());
	fprintf(fp, "CoreDumpFile: %s\n\n", coreDumpFile);

	if (errorString.Length())
		fprintf(fp, "%s\n\n", *errorString);

	fprintf(fp, "System --------------------------------\n");
	fprintf(fp, "Platform:        %s\n", Platform::GetName());
	fprintf(fp, "PlatformVersion: %s\n", Platform::GetVersion());
	fprintf(fp, "Architecture:    %s\n", Platform::GetMachineArchitecture());
	fprintf(fp, "MachineName:     %s\n\n", Platform::GetMachineName());

	fprintf(fp, "Processor -----------------------------\n");
	fprintf(fp, "CPU: %s\n", Platform::GetProcessorName());
	fprintf(fp, "Frequency: %d MHz\n", Platform::GetProcessorFrequency());
	fprintf(fp, "NumberOfProcessors: %d\n\n", Platform::GetNumberOfProcessors());

	fprintf(fp, "Memory --------------------------------\n");
	fprintf(fp, "TotalSystemMemory: %lu KB\n", Platform::GetTotalSystemMemory());
	fprintf(fp, "FreeSystemMemory:  %lu KB\n", Platform::GetFreeSystemMemory());
	fprintf(fp, "ProcessMemory:     %lu KB\n\n", Platform::GetProcessMemory());

	fprintf(fp, "Graphics Device -----------------------\n");
	fprintf(fp, "Device: %s\n", Renderer::GetInstance()->GetDeviceName());
	fprintf(fp, "API:    %s %s\n\n", Renderer::GetInstance()->GetAPIName(), Renderer::GetInstance()->GetAPIVersion());

	fprintf(fp, "Stack trace ---------------------------\n");
	fprintf(fp, "Current thread: %s\n\n", DBG_GET_THREAD_NAME());
	fprintf(fp, "%s", *stackTrace);
	fprintf(fp, "---------------------------------------\n\n\n");

	fclose(fp);

	SaveCoreDump(coreDumpFile, params);
}
