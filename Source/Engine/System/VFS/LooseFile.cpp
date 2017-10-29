/* NekoEngine
 *
 * LooseFile.cpp
 * Author: Alexandru Naiman
 *
 * Virtual File System - Loose file
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

#include <stdint.h>
#include <string.h>

#include <Engine/Engine.h>
#include <Platform/Compat.h>
#include <System/Logger.h>
#include <System/VFS/LooseFile.h>

#define VFS_LFILE_BUFF_SIZE					2048
#define VFS_LFILE_MODULE					"VFS_LooseFile"

extern int __vfs_getLoosePath(const char *relativePath, char *buff, int32_t buffSize);

LooseFile::LooseFile() :
	VFSFile(FileType::Loose)
{
	_fp = nullptr;
}

LooseFile::LooseFile(VFSArchive *archive) :
	VFSFile(archive)
{
	memset(&_header, 0x0, sizeof(VFSFileHeader));
	_type = FileType::Packed;
	_references = 0;
}

bool LooseFile::IsOpen()
{
	return _fp != nullptr;
}

bool LooseFile::IsReadonly()
{
	return false;
}

int LooseFile::Open()
{
	char buff[VFS_LFILE_BUFF_SIZE]{};

	if (_fp)
		return ENGINE_OK;

	if (__vfs_getLoosePath(_header.name, buff, VFS_LFILE_BUFF_SIZE) != ENGINE_OK)
		return ENGINE_FAIL;

	_fp = fopen(buff, "rb");

	if (!_fp)
		return ENGINE_FAIL;

	++_references;

	return ENGINE_OK;
}

int LooseFile::Create()
{
	char buff[VFS_LFILE_BUFF_SIZE]{};

	if (__vfs_getLoosePath(_header.name, buff, VFS_LFILE_BUFF_SIZE) != ENGINE_OK)
		return ENGINE_FAIL;

	_fp = fopen(buff, "wb");

	if (!_fp)
		return ENGINE_FAIL;

	++_references;

	return ENGINE_OK;
}

size_t LooseFile::Read(void *buffer, size_t size, size_t count)
{
	if (!_fp)
		return 0;

	size_t ret = fread(buffer, size, count, _fp);

	if (ret != count)
	{
		if (feof(_fp))
			return ret;

		Logger::Log(VFS_LFILE_MODULE, LOG_CRITICAL, "Failed to read file %s", _header.name);
		return 0;
	}

	return ret;
}

char *LooseFile::Gets(char *str, int num)
{
	if (!_fp)
		return nullptr;

	return fgets(str, num, _fp);
}

size_t LooseFile::Write(void *buffer, size_t size, size_t count)
{
	if (!_fp || IsReadonly())
		return 0;

	return fwrite(buffer, size, count, _fp);
}

int LooseFile::Seek(size_t offset, int origin)
{
	if (!_fp)
		return ENGINE_FAIL;

	return fseek(_fp, (long)offset, origin);
}

size_t LooseFile::Tell()
{
	if (!_fp)
		return 0;
	
	long size = ftell(_fp);

	if (size < 0)
	{
		Logger::Log(VFS_LFILE_MODULE, LOG_WARNING, "Failed to get file size for: %s", _header.name);
		return 0;
	}

	return size;
}

bool LooseFile::EoF()
{
	if (!_fp)
		return true;

	return (feof(_fp) >= 1);
}

void LooseFile::Close()
{
	--_references;

	if (_references)
		return;

	if (_fp) fclose(_fp);
	_fp = nullptr;
}

LooseFile::~LooseFile()
{
	if (!_references)
		return;

	_references = 0;

	if (_fp) fclose(_fp);
	_fp = nullptr;
}