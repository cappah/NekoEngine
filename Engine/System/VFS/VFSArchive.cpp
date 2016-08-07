/* NekoEngine
 *
 * VFSArchive.cpp
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

#include <Engine/Engine.h>
#include <System/VFS/VFSArchive.h>

#define VFS_AR_MODULE	"VFS_Archive"

using namespace std;

VFSArchive::VFSArchive(string &path)
{
	_path = path;
	_data = nullptr;
	_fp = nullptr;
	_dataSize = 0;
	memset(&_header, 0x0, sizeof(VFSArchiveHeader));
}

int VFSArchive::Load()
{
	_fp = fopen(_path.c_str(), "rb");
	if (!_fp)
		return ENGINE_IO_FAIL;

	if(fread(&_header, sizeof(VFSArchiveHeader), 1, _fp) != 1)
	{
		Logger::Log(VFS_AR_MODULE, LOG_CRITICAL, "failed to read archive header from %s", _path.c_str());
		return ENGINE_IO_FAIL;
	}

	if(_header.magic != VFS_MAGIC)
	{
		Logger::Log(VFS_AR_MODULE, LOG_CRITICAL, "%s is not a NekoEngine archive file", _path.c_str());
		return ENGINE_IO_FAIL;
	}

	if(_header.version != VFS_AR_VERSION)
	{
		Logger::Log(VFS_AR_MODULE, LOG_CRITICAL, "Archive version missmatch for %s", _path.c_str());
		return ENGINE_IO_FAIL;
	}
	
	if(_header.num_files <= 0 || _header.num_files > VFS_MAX_FILES)
	{
		Logger::Log(VFS_AR_MODULE, LOG_CRITICAL, "Invalid number of files for %s", _path.c_str());
		return ENGINE_FAIL;
	}

	_files.reserve(_header.num_files);

	vector<VFSFileHeader> fileHeaders(_header.num_files);
	if(fread(fileHeaders.data(), sizeof(VFSFileHeader), _header.num_files, _fp) != _header.num_files)
	{
		Logger::Log(VFS_AR_MODULE, LOG_CRITICAL, "failed to read archive header from %s", _path.c_str());
		return ENGINE_IO_FAIL;
	}

	for (VFSFileHeader &fileHeader : fileHeaders)
	{
		VFSFile f(this);

		f.GetHeader().start = fileHeader.start;
		f.GetHeader().size = fileHeader.size;
		_dataSize += fileHeader.size;

		if (snprintf(f.GetHeader().name, VFS_MAX_FILE_NAME, "%s", fileHeader.name) >= VFS_MAX_FILE_NAME)
		{
			Logger::Log(VFS_AR_MODULE, LOG_CRITICAL, "snprintf() call failed");
			return ENGINE_FAIL;
		}

		_files.push_back(f);
	}

	return ENGINE_OK;
}

void VFSArchive::Unload()
{
	for (VFSFile &file : _files)
		file.Close();
	_files.clear();

	free(_data);

	if (_fp) fclose(_fp);
}

int VFSArchive::MakeResident()
{
	if (_data)
		return ENGINE_OK;

	if ((_data = (uint8_t *)calloc(1, _dataSize)) == nullptr)
	{
		Logger::Log(VFS_AR_MODULE, LOG_CRITICAL, "Failed to allocate memory for the archive");
		return ENGINE_OUT_OF_RESOURCES;
	}

	if (fseek(_fp, (long)sizeof(VFSArchiveHeader) + (long)sizeof(VFSFileHeader) * _header.num_files, SEEK_SET))
	{
		free(_data);
		_data = nullptr;
		return ENGINE_IO_FAIL;
	}

	if (fread(_data, _dataSize, 1, _fp) != 1)
	{
		free(_data);
		_data = nullptr;
		return ENGINE_IO_FAIL;
	}

	return ENGINE_OK;
}

VFSFile *VFSArchive::Open(string &path)
{
	const char *str = path.c_str();
	size_t len = strlen(str);

	for (VFSFile &file : _files)
	{
		if (!strncmp(str, file.GetHeader().name, len))
		{
			file.Open();
			return &file;
		}
	}

	return nullptr;
}

uint64_t VFSArchive::Read(void *buffer, uint64_t offset, uint64_t size, uint64_t count)
{
	if (_data)
	{
		memcpy(buffer, (_data + offset), size * count);
		return count;
	}

	// Archive is not resident
	if(fseek(_fp, (long)sizeof(VFSArchiveHeader) + (long)sizeof(VFSFileHeader) * _header.num_files + (long)offset, SEEK_SET))
		Logger::Log(VFS_AR_MODULE, LOG_CRITICAL, "Failed to read archive file %s", _path.c_str());

	size_t ret = fread(buffer, size, count, _fp);
	if (!ret)
		Logger::Log(VFS_AR_MODULE, LOG_CRITICAL, "Failed to read archive file %s", _path.c_str());

	return ret;
}

VFSArchive::~VFSArchive()
{
	Unload();
}
