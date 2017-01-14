/* NekoEngine
 *
 * VFSArchive.h
 * Author: Alexandru Naiman
 *
 * Virtual File System
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

#pragma once

#include <string>
#include <vector>
#include <stdint.h>
#include <stdlib.h>

#include <Engine/Engine.h>
#include <Runtime/Runtime.h>
#include <System/VFS/VFSFile.h>

#define VFS_MAGIC			0xB16B00B5
#define VFS_AR_VERSION		0x00000001
#define VFS_MAX_FILES		10000

typedef struct VFS_ARCHIVE_HEADER
{
	uint32_t magic;
	uint32_t version;
	uint32_t num_files;
} VFSArchiveHeader;

class VFSArchive
{
public:
	ENGINE_API VFSArchive(NString &path);
	
	ENGINE_API int Load();
	ENGINE_API void Unload();

	ENGINE_API int MakeResident();
	ENGINE_API void MakeNonResident() { free(_data); _data = nullptr; }
	ENGINE_API bool IsResident() { return _data ? true : false; }
	
	ENGINE_API VFSFile* Open(NString &path);
	ENGINE_API size_t Read(void *buffer, size_t offset, size_t size, size_t count);

	ENGINE_API void GetFilesInDirectory(const NString &directory, NArray<VFSFile *> files);

	ENGINE_API virtual ~VFSArchive();

private:
	VFSArchiveHeader _header;
	NString _path;
	std::vector<VFSFile *> _files;
	FILE *_fp;
	uint8_t *_data;
	size_t _dataSize;
};