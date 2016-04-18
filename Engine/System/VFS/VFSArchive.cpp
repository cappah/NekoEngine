/* Neko Engine
 *
 * VFSArchive.cpp
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

#include <Engine/Engine.h>
#include <System/VFS/VFSArchive.h>

#define VFS_AR_MODULE	"VFS_Archive"

using namespace std;

VFSArchive::VFSArchive(string &path)
{
	_path = path;
	memset(&_header, 0x0, sizeof(VFSArchiveHeader));
}

int VFSArchive::Load()
{
	return ENGINE_OK;
}

void VFSArchive::Unload()
{
	//
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

VFSArchive::~VFSArchive()
{
	for (VFSFile &file : _files)
	{
		if (file.IsOpen())
			Logger::Log(VFS_AR_MODULE, LOG_WARNING, "Archive closed with open files");
	}
}
