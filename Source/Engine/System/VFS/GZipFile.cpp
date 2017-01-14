/* NekoEngine
 *
 * GZipFile.cpp
 * Author: Alexandru Naiman
 *
 * Virtual File System - GZip compressed file
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
#include <System/VFS/GZipFile.h>
#include <System/VFS/VFSArchive.h>

#define VFS_GZFILE_BUFF_SIZE			2048
#define VFS_GZFILE_MODULE				"VFS_GZipFile"

extern int __vfs_getLoosePath(const char *relativePath, char *buff, int32_t buffSize);

GZipFile::GZipFile() :
	VFSFile(FileType::Loose),
	_fp(nullptr)
{
}

bool GZipFile::IsOpen()
{
	return _fp != nullptr;
}

bool GZipFile::IsReadonly()
{
	return false;
}

int GZipFile::Open()
{
	char buff[VFS_GZFILE_BUFF_SIZE]{};

	if (_fp)
		return ENGINE_OK;

	if (__vfs_getLoosePath(_header.name, buff, VFS_GZFILE_BUFF_SIZE) != ENGINE_OK)
		return ENGINE_FAIL;

	_fp = gzopen(buff, "rb");

	if (!_fp)
		return ENGINE_FAIL;

#if !defined(NE_PLATFORM_OPENBSD) && !defined(NE_PLATFORM_SUNOS)
	gzbuffer(_fp, 131072); // 128 kB buffer
#endif

	++_references;

	return ENGINE_OK;
}

int GZipFile::Create()
{
	char buff[VFS_GZFILE_BUFF_SIZE]{};
	if (__vfs_getLoosePath(_header.name, buff, VFS_GZFILE_BUFF_SIZE) != ENGINE_OK)
		return ENGINE_FAIL;

	_fp = gzopen(buff, "wb");

	if (!_fp)
		return ENGINE_FAIL;

#if !defined(NE_PLATFORM_OPENBSD) && !defined(NE_PLATFORM_SUNOS)
	gzbuffer(_fp, 131072); // 128 kB buffer
#endif

	++_references;

	return ENGINE_OK;
}

size_t GZipFile::Read(void *buffer, size_t size, size_t count)
{
	if (!_fp)
		return 0;

	return gzread(_fp, buffer, (unsigned int)(size * count)) / size;
}

char *GZipFile::Gets(char *str, int num)
{
	if (!_fp)
		return nullptr;

	return gzgets(_fp, str, num);
}

size_t GZipFile::Write(void *buffer, size_t size, size_t count)
{
	if (!_fp || IsReadonly())
		return 0;

	return gzwrite(_fp, buffer, (unsigned int)(size * count)) / size;
}

int GZipFile::Seek(size_t offset, int origin)
{
	if (!_fp)
		return ENGINE_FAIL;

	return (int)gzseek(_fp, (long)offset, origin);
}

size_t GZipFile::Tell()
{
	if (!_fp)
		return 0;
		
	return gztell(_fp);
}

bool GZipFile::EoF()
{
	if (!_fp)
		return true;

	return (gzeof(_fp) >= 1);
}

void GZipFile::Close()
{
	--_references;

	if (_references)
		return;

	gzclose(_fp);
	_fp = nullptr;
}

GZipFile::~GZipFile()
{
	if (!_references)
		return;

	_references = 0;
	gzclose(_fp);
	_fp = nullptr;
}
