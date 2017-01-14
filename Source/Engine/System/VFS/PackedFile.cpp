/* NekoEngine
 *
 * PackedFile.cpp
 * Author: Alexandru Naiman
 *
 * Virtual File System - Packed file
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

#include <zlib.h>
#include <stdint.h>
#include <string.h>

#include <Engine/Engine.h>
#include <Platform/Compat.h>
#include <System/Logger.h>
#include <System/VFS/PackedFile.h>
#include <System/VFS/VFSArchive.h>

#define VFS_PFILE_BUFF_SIZE					2048
#define VFS_PFILE_DECOMPRESS_BUFF_SIZE		524288
#define VFS_PFILE_MODULE					"VFS_PackedFile"

PackedFile::PackedFile() :
	VFSFile(FileType::Packed)
{
	_offset = 0;
	_archive = nullptr;
}

PackedFile::PackedFile(VFSArchive *archive) :
	VFSFile(archive)
{
	memset(&_header, 0x0, sizeof(VFSFileHeader));
	_type = FileType::Packed;
	_references = 0;
	_offset = 0;
	_archive = archive;
	_fileData = nullptr;
	_compressed = false;
	_uncompressedSize = 0;
	_decompressing = false;
}

bool PackedFile::IsOpen()
{
	return _references > 0;
}

bool PackedFile::IsReadonly()
{
	return true;
}

int PackedFile::Open()
{
	char hdr[2];
		
	if (Read(hdr, 1, 2) != 2)
		return ENGINE_FAIL;

	Seek(0, SEEK_SET);

	if (hdr[0] == 0x1F)
	{
		_Decompress();
		_compressed = true;
	}
	
	++_references;

	return ENGINE_OK;
}

size_t PackedFile::Read(void *buffer, size_t size, size_t count)
{
	size_t read = 0;
	if (_compressed && !_decompressing)
	{
		if (!_fileData)
			if (_Decompress() != ENGINE_OK || !_fileData)
				return 0;

		if (_offset >= _uncompressedSize)
			return EOF;

		while (_offset + size * count > _uncompressedSize)
			count--;

		memcpy(buffer, (_fileData + _offset), size * count);
		read = count;
	}
	else
	{
		if (_offset >= _header.size)
			return EOF;

		while (_offset + size * count > _header.size)
			count--;

		read = _archive->Read(buffer, size_t(_header.start + _offset), size, count);
	}

	_offset += read * size;
	return read;
}

char *PackedFile::Gets(char *str, int num)
{
	char c = 0x0, *ptr = nullptr;

	for (ptr = str, num--; num > 0; num--)
	{
		if (Read(&c, 1, 1) != 1)
			return nullptr;

		if (c == EOF)
			break;

		*ptr++ = c;

		if (c == '\n')
			break;
	}

	*ptr = 0x0;

	if (ptr == str || c == EOF)
		return nullptr;

	return ptr;
}

size_t PackedFile::Write(void *buffer, size_t size, size_t count)
{
	return 0;
}

int PackedFile::Seek(size_t offset, int origin)
{
	switch (origin)
	{
		case SEEK_SET:
			_offset = offset;
		break;
		case SEEK_CUR:
			_offset += offset;
		break;
		case SEEK_END:
			_offset = _header.size;
		break;
		default:
			return ENGINE_FAIL;
	}

	return ENGINE_OK;
}

size_t PackedFile::Tell()
{
	return (size_t)_offset;
}

bool PackedFile::EoF()
{
	uint64_t size = _compressed ? (_decompressing ? _header.size : _uncompressedSize) : _header.size;
	return (_offset == size);
}

void PackedFile::Close()
{
	--_references;

	if (!_references)
	{
		free(_fileData);
		_fileData = nullptr;
	}
}

int PackedFile::_Decompress()
{
	uint8_t *in_buff = nullptr, *out_buff = nullptr;
	z_stream zstm = { 0 };
	size_t dataBuffSize = VFS_PFILE_DECOMPRESS_BUFF_SIZE, dataWritten = 0;
	int ret = ENGINE_FAIL;

	if (_fileData)
		return ENGINE_OK;

	_decompressing = true;
	
	if ((in_buff = (uint8_t *)malloc(VFS_PFILE_DECOMPRESS_BUFF_SIZE)) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;

	if ((out_buff = (uint8_t *)malloc(VFS_PFILE_DECOMPRESS_BUFF_SIZE)) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;

	zstm.zalloc = Z_NULL;
	zstm.zfree = Z_NULL;
	zstm.opaque = Z_NULL;

	_fileData = (uint8_t *)reallocarray(_fileData, 1, dataBuffSize);

	int zret = -1;

	if ((zret = inflateInit2(&zstm, (15 + 32))) != Z_OK)
		goto exit;

	do
	{
		zstm.avail_in = (uint32_t)Read(in_buff, 1, VFS_PFILE_DECOMPRESS_BUFF_SIZE);
		zstm.next_in = in_buff;

		if (zstm.avail_in == 0)
			break;

		do
		{
			memset(out_buff, 0x0, VFS_PFILE_DECOMPRESS_BUFF_SIZE);

			zstm.avail_out = VFS_PFILE_DECOMPRESS_BUFF_SIZE;
			zstm.next_out = out_buff;

			zret = inflate(&zstm, Z_NO_FLUSH);

			switch (zret)
			{
				case Z_STREAM_ERROR:
				case Z_NEED_DICT:
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					goto exit;					
			}

			size_t dataSize = (VFS_PFILE_DECOMPRESS_BUFF_SIZE - zstm.avail_out);

			if (dataBuffSize < dataWritten + dataSize)
			{
				uint8_t *temp = _fileData;
				dataBuffSize += VFS_PFILE_DECOMPRESS_BUFF_SIZE;

				_fileData = (uint8_t *)reallocarray(_fileData, 1, dataBuffSize);

				if (!_fileData)
				{
                    free(temp);
					Logger::Log(VFS_PFILE_MODULE, LOG_CRITICAL, "reallocarray() failed");
					ret = ENGINE_OUT_OF_RESOURCES;
					goto exit;
				}
			}

			memmove(_fileData + dataWritten, out_buff, dataSize);
            dataWritten += dataSize;
		}
		while (zstm.avail_out == 0);
	}
	while (zret != Z_STREAM_END);
    
	_fileData[dataWritten] = 0x0;
	_uncompressedSize = dataWritten;

	ret = ENGINE_OK;

exit:
	Seek(0, SEEK_SET);

	free(in_buff);
	free(out_buff);
	
	if(ret != ENGINE_OK)
		free(_fileData);

	inflateEnd(&zstm);
	
	_decompressing = false;

	return ret;
}

PackedFile::~PackedFile()
{
	if (_references)
		_references = 0;

	free(_fileData);
}