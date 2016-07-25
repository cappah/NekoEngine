/* Neko Engine
 *
 * VFSFile.cpp
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

#define ENGINE_INTERNAL

#include <stdint.h>
#include <string.h>

#include <Engine/Engine.h>
#include <Platform/Compat.h>
#include <System/VFS/VFSFile.h>
#include <System/VFS/VFSArchive.h>

#define BUFF_SIZE						2048
#define VFS_FILE_DECOMPRESS_BUFF_SIZE	524288
#define VFS_FILE_MODULE					"VFS_File"

VFSFile::VFSFile(FileType type)
{
	memset(&_header, 0x0, sizeof(VFSFileHeader));
	_type = type;
	_references = 0;
	_fp = nullptr;
	_gzfp = nullptr;
	_offset = 0;
	_archive = nullptr;
	_fileData = nullptr;
	_compressed = false;
	_uncompressedSize = 0;
	_decompressing = false;
}

VFSFile::VFSFile(VFSArchive *archive)
{
	memset(&_header, 0x0, sizeof(VFSFileHeader));
	_type = FileType::Packed;
	_references = 0;
	_fp = nullptr;
	_gzfp = nullptr;
	_offset = 0;
	_archive = archive;
	_fileData = nullptr;
	_compressed = false;
	_uncompressedSize = 0;
	_decompressing = false;
}

int VFSFile::Open()
{
	if (_type == FileType::Loose && (!_fp || !_gzfp))
	{
		char buff[BUFF_SIZE];
		memset(buff, 0x0, BUFF_SIZE);
		if (snprintf(buff, BUFF_SIZE, "%s/%s", Engine::GetConfiguration().Engine.DataDirectory, _header.name) >= BUFF_SIZE)
			return ENGINE_FAIL;

		// Try to open normal file
		_fp = fopen(buff, "rb");

		if (!_fp)
			return ENGINE_FAIL;

		unsigned char gzhdr[2];

		if (fread(gzhdr, sizeof(unsigned char) * 2, 1, _fp) != 1)
		{
			fclose(_fp);
			return ENGINE_IO_FAIL;
		}

		// Check for GZIP header
		if (gzhdr[0] != 31 && gzhdr[1] != 139)
		{
			// Header not found; rewind
			fseek(_fp, 0, SEEK_SET);
			return ENGINE_OK;
		}

		// Header found, open as GZIP file
		fclose(_fp);
		_fp = nullptr;

		_compressed = true;

		_gzfp = gzopen(buff, "rb");

		if (!_gzfp)
			return ENGINE_FAIL;

	#if !defined(NE_PLATFORM_OPENBSD) && !defined(NE_PLATFORM_SUNOS)
		gzbuffer(_gzfp, 131072); // 128 kB buffer
	#endif
	}
	else if(_type == FileType::Packed)
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
	}

	_references++;

	return ENGINE_OK;
}

uint64_t VFSFile::Read(void *buffer, uint64_t size, uint64_t count)
{
	if (_type == FileType::Loose)
	{
		if (_fp)
		{
			size_t ret = fread(buffer, size, count, _fp);

			if (ret != count)
			{
				if (feof(_fp))
					return ret;

				Logger::Log(VFS_FILE_MODULE, LOG_CRITICAL, "Failed to read file %s", _header.name);
				return 0;
			}

			return ret;
		}
		else if (_gzfp)
			return gzread(_gzfp, buffer, (unsigned int)(size * count));
		else
			return 0;
	}
	else if(_type == FileType::Packed)
	{
		uint64_t read = 0;
		if (_compressed && !_decompressing)
		{
			if (!_fileData)
				if (_Decompress() != ENGINE_OK)
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

			read = _archive->Read(buffer, _header.start + _offset, size, count);
		}

		_offset += read * size;
		return read;
	}

	return 0;
}

char *VFSFile::Gets(char *str, int num)
{
	if (_type == FileType::Loose)
	{
		if (_fp)
			return fgets(str, num, _fp);
		else if(_gzfp)
			return gzgets(_gzfp, str, num);
		else
			return nullptr;
	}
	else
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
}

int VFSFile::Seek(uint64_t offset, int origin)
{
	if (_type == FileType::Loose)
	{
		if (_fp)
			return fseek(_fp, (long)offset, origin);
		else if(_gzfp)
			return (int)gzseek(_gzfp, (long)offset, origin);
		else
			return ENGINE_FAIL;
	}

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

uint64_t VFSFile::Tell()
{
	if (_type == FileType::Loose)
	{
		if (_fp)
		{
			long size = ftell(_fp);

			if (size < 0)
			{
				Logger::Log(VFS_FILE_MODULE, LOG_WARNING, "Failed to get file size for: %s", _header.name);
				return 0;
			}

			return size;
		}
		else if (_gzfp)
			return gztell(_gzfp);
		else
			return 0;
	}

	return _offset;
}

bool VFSFile::EoF()
{
	if (_type == FileType::Loose)
	{
		if (_fp)
			return (feof(_fp) >= 1);
		else if (_gzfp)
			return (gzeof(_gzfp) >= 1);
		else
			return true;
	}
	else
	{
		uint64_t size = _compressed ? (_decompressing ? _header.size : _uncompressedSize) : _header.size;
		return (_offset == size);
	}
}

void VFSFile::Close()
{
	_references--;

	if (_type == FileType::Loose && !_references)
	{
		if (_fp)
			fclose(_fp);
		else if(_gzfp)
			gzclose(_gzfp);
	}
	else if (!_references)
	{
		free(_fileData);
		_fileData = nullptr;
	}

	_fp = nullptr;
	_gzfp = nullptr;
}

int VFSFile::_Decompress()
{
	uint8_t *in_buff = nullptr, *out_buff = nullptr;
	z_stream zstm = { 0 };
	size_t dataBuffSize = VFS_FILE_DECOMPRESS_BUFF_SIZE, dataWritten = 0;
	int ret = ENGINE_FAIL;

	if (_fileData)
		return ENGINE_OK;

	_decompressing = true;
	
	in_buff = (uint8_t *)malloc(VFS_FILE_DECOMPRESS_BUFF_SIZE);
	out_buff = (uint8_t *)malloc(VFS_FILE_DECOMPRESS_BUFF_SIZE);

	zstm.zalloc = Z_NULL;
	zstm.zfree = Z_NULL;
	zstm.opaque = Z_NULL;

	_fileData = (uint8_t *)reallocarray(_fileData, 1, dataBuffSize);

	int zret = -1;

	if ((zret = inflateInit2(&zstm, (15 + 32))) != Z_OK)
		goto exit;

	do
	{
		zstm.avail_in = (uint)Read(in_buff, 1, VFS_FILE_DECOMPRESS_BUFF_SIZE);
		zstm.next_in = in_buff;

		if (zstm.avail_in == 0)
			break;

		do
		{
			memset(out_buff, 0x0, VFS_FILE_DECOMPRESS_BUFF_SIZE);

			zstm.avail_out = VFS_FILE_DECOMPRESS_BUFF_SIZE;
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

			size_t dataSize = (VFS_FILE_DECOMPRESS_BUFF_SIZE - zstm.avail_out);

			if (dataBuffSize < dataWritten + dataSize)
			{
				uint8_t *temp = _fileData;
				dataBuffSize += VFS_FILE_DECOMPRESS_BUFF_SIZE;

				_fileData = (uint8_t *)reallocarray(_fileData, 1, dataBuffSize);

				if (!_fileData)
				{
                    free(temp);
					Logger::Log(VFS_FILE_MODULE, LOG_CRITICAL, "reallocarray() failed");
					ret = ENGINE_OUT_OF_RESOURCES;
					goto exit;
				}
			}

			memmove(_fileData + dataWritten, out_buff, dataSize);
			dataWritten += dataSize;
			_fileData[dataWritten] = 0x0;
		}
		while (zstm.avail_out == 0);
	}
	while (zret != Z_STREAM_END);

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

VFSFile::~VFSFile()
{
	if (_references)
		_references = 0;

	if (_fp)
		fclose(_fp);
	else if (_gzfp)
		gzclose(_gzfp);

	free(_fileData);
}
