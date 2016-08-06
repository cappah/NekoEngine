/* Neko Engine
 *
 * VFS.cpp
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

#include <dirent.h>
#include <sys/stat.h>
#include <stack>

#include <Engine/Engine.h>
#include <System/VFS/VFS.h>

#define VFS_MODULE	"VFS"

#ifdef NE_PLATFORM_WINDOWS
// Really, M$ ?
#define stat _stat
#endif

using namespace std;

typedef struct DIR_INFO
{
	string path;
	string prefix;
} DirInfo;

vector<VFSFile> VFS::_looseFiles;
vector<VFSArchive *> VFS::_archives;

int VFS::Initialize()
{
	if (Engine::GetConfiguration().Engine.LoadLooseFiles)
	{
		DIR *dir;
		struct dirent *ent;
		VFSFile f(FileType::Loose);
		stack<DirInfo> directories;
		struct stat st;

		directories.push({ Engine::GetConfiguration().Engine.DataDirectory, "" });

		// Recursion is evil
		while (!directories.empty())
		{
			DirInfo info = directories.top();
			directories.pop();

			if ((dir = opendir(info.path.c_str())) != NULL)
			{
				while ((ent = readdir(dir)) != NULL)
				{
					string path = info.path;
					path.append("/");
					path.append(ent->d_name);
					
					if (stat(path.c_str(), &st) < 0)
					{
						Logger::Log(VFS_MODULE, LOG_CRITICAL, "File %s does not exist", path.c_str());
						closedir(dir);
						return ENGINE_FAIL;
					}

					if(S_ISDIR(st.st_mode))
					{
						if (!strncmp(ent->d_name, ".", 1) || !strncmp(ent->d_name, "..", 2))
							continue;

						string prefix = info.prefix;
						prefix.append("/");
						prefix.append(ent->d_name);

						directories.push({ path, prefix });
					}
					else if (S_ISREG(st.st_mode))
					{
						if (snprintf(f.GetHeader().name, VFS_MAX_FILE_NAME, "%s/%s", info.prefix.c_str(), ent->d_name) >= VFS_MAX_FILE_NAME)
						{
							Logger::Log(VFS_MODULE, LOG_CRITICAL, "snprintf() call failed");
							closedir(dir);
							return ENGINE_FAIL;
						}
						_looseFiles.push_back(f);
					}
				}

				closedir(dir);
			}
			else
			{ 
				Logger::Log(VFS_MODULE, LOG_CRITICAL, "Failed to open directory: %s", info.path.c_str());
				DIE("Failed to open directory");
			}
		}
	}
	
	Logger::Log(VFS_MODULE, LOG_INFORMATION, "Initialized");

	return ENGINE_OK;
}

int VFS::LoadArchive(string path)
{
	int ret = ENGINE_FAIL;
	VFSArchive *archive = new VFSArchive(path);

	if ((ret = archive->Load()) != ENGINE_OK)
	{
		delete archive;
		return ret;
	}

	_archives.push_back(archive);

	return ENGINE_OK;
}

VFSFile *VFS::Open(string &path)
{
	if (Engine::GetConfiguration().Engine.LoadLooseFiles)
	{
		const char *str = path.c_str();
		size_t len = strlen(str);

		for (VFSFile &file : _looseFiles)
		{
			if (!strncmp(str, file.GetHeader().name, len))
			{
				if (file.Open() != ENGINE_OK)
					return nullptr;

				return &file;
			}
		}
	}

	for (VFSArchive *archive : _archives)
	{
		VFSFile *file = archive->Open(path);
		if (file)
			return file;
	}

	return nullptr;
}

void VFS::Release()
{
	for (VFSArchive *archive : _archives)
		delete archive;
	_archives.clear();
	
	Logger::Log(VFS_MODULE, LOG_INFORMATION, "Released");
}
