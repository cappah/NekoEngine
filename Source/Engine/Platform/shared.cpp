/* NekoEngine
 *
 * shared.cpp
 * Author: Alexandru Naiman
 *
 * Shared platform functions
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

#include <assert.h>

#include <Engine/Engine.h>
#include <Platform/Compat.h>
#include <Platform/Platform.h>

#define INI_LINE_BUFF	512

bool Platform::_exit = false;

size_t Platform::GetConfigString(const char *section, const char *entry, const char *def, char *buffer, int buffer_len, const char *file)
{
	FILE *fp = fopen(file, "r");
	bool found = false;
	char lineBuff[INI_LINE_BUFF], sectionBuff[INI_LINE_BUFF];
	memset(lineBuff, 0x0, INI_LINE_BUFF);

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
