/* NekoEngine
 *
 * VFSFile.h
 * Author: Alexandru Naiman
 *
 * Virtual File System
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

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <zlib.h>

#include <Runtime/Runtime.h>

enum class FileType : unsigned
{
	Loose = 0,
	Packed
};

#define VFS_MAX_FILE_NAME		1024
#define VFS_MAX_DIR_NAME		1024

typedef struct VFS_FILE_HEADER
{
	char name[VFS_MAX_FILE_NAME];
	char dir[VFS_MAX_DIR_NAME];
	uint64_t start;
	uint64_t size;
} VFSFileHeader;

class VFSFile
{

public:
	VFSFile(FileType type);
	VFSFile(class VFSArchive *archive);

	VFSFileHeader& GetHeader() { return _header; }
	FILE* GetNativeHandle() { return _fp; }

	bool IsOpen() { return _type == FileType::Loose ? _fp != nullptr : _references > 0; }

	int Open();
	uint64_t Read(void *buffer, uint64_t size, uint64_t count);
	char* Gets(char* str, int num);
	int Seek(uint64_t offset, int origin);
	uint64_t Tell();
	bool EoF();
	void Close();

	~VFSFile();

private:
	VFSFileHeader _header;
	FileType _type;
	unsigned int _references;
	uint64_t _offset;
	class VFSArchive *_archive;
	bool _compressed;
	uint8_t *_fileData;
	uint64_t _uncompressedSize;
	bool _decompressing;

	int _Decompress();

	// Only for loose files
	FILE* _fp;
	gzFile _gzfp;
};

//template ENGINE_API class NArray<VFSFile>;