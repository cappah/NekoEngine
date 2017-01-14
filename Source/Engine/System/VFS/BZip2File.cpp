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
#include <System/VFS/BZip2File.h>
#include <System/VFS/VFSArchive.h>

#define VFS_BZ2FILE_BUFF_SIZE			2048
#define VFS_BZ2FILE_MODULE				"VFS_BZip2File"

extern int __vfs_getLoosePath(const char *relativePath, char *buff, int32_t buffSize);

BZip2File::BZip2File() :
	VFSFile(FileType::Loose),
	_fp(nullptr)
{
}

bool BZip2File::IsOpen()
{
	return _fp != nullptr;
}

bool BZip2File::IsReadonly()
{
	return false;
}

int BZip2File::Open()
{
	char buff[VFS_BZ2FILE_BUFF_SIZE]{};

	if (_fp)
		return ENGINE_OK;

	if (__vfs_getLoosePath(_header.name, buff, VFS_BZ2FILE_BUFF_SIZE) != ENGINE_OK)
		return ENGINE_FAIL;

	_fp = BZ2_bzopen(buff, "rb");

	if (!_fp)
		return ENGINE_FAIL;

	++_references;

	return ENGINE_OK;
}

int BZip2File::Create()
{
	char buff[VFS_BZ2FILE_BUFF_SIZE]{};
	if (__vfs_getLoosePath(_header.name, buff, VFS_BZ2FILE_BUFF_SIZE) != ENGINE_OK)
		return ENGINE_FAIL;

	_fp = BZ2_bzopen(buff, "wb");

	if (!_fp)
		return ENGINE_FAIL;

	++_references;

	return ENGINE_OK;
}

size_t BZip2File::Read(void *buffer, size_t size, size_t count)
{
	if (!_fp)
		return 0;

	return BZ2_bzread(_fp, buffer, (unsigned int)(size * count)) / size;
}

char *BZip2File::Gets(char *str, int num)
{
	if (!_fp)
		return nullptr;

	if (Read(str, num, 1) != 1)
		return nullptr;

	return str;
}

size_t BZip2File::Write(void *buffer, size_t size, size_t count)
{
	if (!_fp || IsReadonly())
		return 0;

	return BZ2_bzwrite(_fp, buffer, (unsigned int)(size * count)) / size;
}

int BZip2File::Seek(size_t offset, int origin)
{
	return ENGINE_FAIL;
}

size_t BZip2File::Tell()
{
	return 0;
}

bool BZip2File::EoF()
{
	return false;
}

void BZip2File::Close()
{
	--_references;

	if (_references)
		return;

	BZ2_bzclose(_fp);
	_fp = nullptr;
}

BZip2File::~BZip2File()
{
	if (!_references)
		return;

	_references = 0;
	BZ2_bzclose(_fp);
	_fp = nullptr;
}
