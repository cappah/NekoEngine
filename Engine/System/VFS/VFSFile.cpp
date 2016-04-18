/* Neko Engine
 *
 * VFSFile.cpp
 * Author: Alexandru Naiman
 *
 * Virtual File System
 *
 * ----------------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman <alexandru dot naiman at icloud dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ALEXANDRU NAIMAN
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define ENGINE_INTERNAL

#include <stdint.h>
#include <string.h>

#include <Engine/Engine.h>
#include <System/VFS/VFSFile.h>

#define BUFF_SIZE	2048
#define VFS_FILE_MODULE	"VFS_File"

VFSFile::VFSFile(FileType type)
{
	memset(&_header, 0x0, sizeof(VFSFileHeader));
	_type = type;
	_references = 0;
	_fp = nullptr;
	_gzfp = nullptr;
	_offset = 0;
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

		_gzfp = gzopen(buff, "rb");

		if (!_gzfp)
			return ENGINE_FAIL;

		gzbuffer(_gzfp, 131072); // 128 kB buffer
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
		else if(_gzfp)
			return gzread(_gzfp, buffer, (unsigned int)(size * count));
		else	
			return ENGINE_FAIL;
	}
	else
	{
	//	memcpy(buffer, (unsigned char*)_header.start + _offset, size * count);
	}

	return ENGINE_FAIL;
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
		return nullptr;
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
			return (feof(_fp) == 1);
		else if (_gzfp)
			return (gzeof(_gzfp) == 1);
		else
			return true;
	}
	else
		return (_offset == _header.size);
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

	_fp = nullptr;
	_gzfp = nullptr;
}

VFSFile::~VFSFile()
{
	if (_references)
		_references = 0;

	if (_fp)
		fclose(_fp);
	else if (_gzfp)
		gzclose(_gzfp);
}
