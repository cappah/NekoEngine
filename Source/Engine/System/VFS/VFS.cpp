/* NekoEngine
 *
 * VFS.cpp
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

#include <sys/stat.h>
#include <dirent.h>
#include <zlib.h>
#include <stack>

#include <Engine/Engine.h>
#include <System/Logger.h>
#include <System/VFS/VFS.h>
#include <System/VFS/GZipFile.h>
#include <System/VFS/BZip2File.h>
#include <System/VFS/LooseFile.h>

#define COMPRESSED_GZIP		1
#define COMPRESSED_BZIP2	2

#define VFS_MODULE					"VFS"
#define VFS_DECOMPRESS_BUFF_SIZE	524288

#if defined(NE_PLATFORM_WINDOWS)
// Really, M$ ?
#define stat _stat
#endif

using namespace std;

typedef struct DIR_INFO
{
	NString path;
	NString prefix;
} DirInfo;

static vector<VFSFile *> _looseFiles;
static vector<VFSArchive *> _archives;

static map<NString, NString> _mountPoints{};

bool __vfs_isCompressed(const char *path, uint8_t *type = NULL)
{
	// Try to open normal file
	FILE *fp{ fopen(path, "rb") };

	if (!fp)
		return false;

	unsigned char hdr[2];

	if (fread(hdr, sizeof(unsigned char) * 2, 1, fp) != 1)
	{
		fclose(fp);
		return false;
	}

	fclose(fp);
	fp = nullptr;

	// Check for GZip header
	if (hdr[0] == 31 && hdr[1] == 139)
	{
		if (type)
			*type = COMPRESSED_GZIP;
		return true;
	}

	// Check for BZip2 header
	if (hdr[0] == 66 && hdr[1] == 90)
	{
		if (type)
			*type = COMPRESSED_BZIP2;
		return true;
	}

	return false;
}

int __vfs_getLoosePath(const char *vfsPath, char *buff, int32_t buffSize)
{
	NString vfsPathStr(vfsPath);
	char *realDirectory = Engine::GetConfiguration().Engine.DataDirectory;

	memset(buff, 0x0, buffSize);

	for (pair<const NString, NString> &kvp : _mountPoints)
	{
		if (vfsPathStr.Contains(kvp.first))
		{
			realDirectory = *kvp.second;
			vfsPathStr = vfsPathStr.Substring(kvp.first.Length());
			break;
		}
	}

	if (snprintf(buff, buffSize, "%s%s", realDirectory, *vfsPathStr) >= buffSize)
		return ENGINE_FAIL;

	return ENGINE_OK;
}

int VFS::Initialize()
{
	char buff[VFS_MAX_FILE_NAME]{};

	{ // Special directory mount points
		if (Platform::GetSpecialDirectoryPath(SpecialDirectory::ApplicationData, buff, VFS_MAX_FILE_NAME) != ENGINE_OK)
		{
			Logger::Log(VFS_MODULE, LOG_CRITICAL, "Failed to get ApplicationData path");
			return ENGINE_FAIL;
		}
		_mountPoints.insert({ NString("/AppData"), buff });
		memset(buff, 0x0, VFS_MAX_FILE_NAME);

		if (Platform::GetSpecialDirectoryPath(SpecialDirectory::Documents, buff, VFS_MAX_FILE_NAME) != ENGINE_OK)
		{
			Logger::Log(VFS_MODULE, LOG_CRITICAL, "Failed to get Documents path");
			return ENGINE_FAIL;
		}
		_mountPoints.insert({ NString("/Home/Documents"), buff });
		memset(buff, 0x0, VFS_MAX_FILE_NAME);

		if (Platform::GetSpecialDirectoryPath(SpecialDirectory::Pictures, buff, VFS_MAX_FILE_NAME) != ENGINE_OK)
		{
			Logger::Log(VFS_MODULE, LOG_CRITICAL, "Failed to get Pictures path");
			return ENGINE_FAIL;
		}
		_mountPoints.insert({ NString("/Home/Pictures"), buff });
		memset(buff, 0x0, VFS_MAX_FILE_NAME);

		if (Platform::GetSpecialDirectoryPath(SpecialDirectory::Music, buff, VFS_MAX_FILE_NAME) != ENGINE_OK)
		{
			Logger::Log(VFS_MODULE, LOG_CRITICAL, "Failed to get Music path");
			return ENGINE_FAIL;
		}
		_mountPoints.insert({ NString("/Home/Music"), buff });
		memset(buff, 0x0, VFS_MAX_FILE_NAME);

		if (Platform::GetSpecialDirectoryPath(SpecialDirectory::Home, buff, VFS_MAX_FILE_NAME) != ENGINE_OK)
		{
			Logger::Log(VFS_MODULE, LOG_CRITICAL, "Failed to get Home path");
			return ENGINE_FAIL;
		}
		_mountPoints.insert({ NString("/Home"), buff });
		memset(buff, 0x0, VFS_MAX_FILE_NAME);

		if (Platform::GetSpecialDirectoryPath(SpecialDirectory::Temp, buff, VFS_MAX_FILE_NAME) != ENGINE_OK)
		{
			Logger::Log(VFS_MODULE, LOG_CRITICAL, "Failed to get Temp path");
			return ENGINE_FAIL;
		}
		_mountPoints.insert({ NString("/Temp"), buff });
		memset(buff, 0x0, VFS_MAX_FILE_NAME);
	}

	memset(buff, 0x0, VFS_MAX_FILE_NAME);

	if (Engine::GetConfiguration().Engine.LoadLooseFiles)
	{
		DIR *dir{ nullptr };
		struct dirent *ent{ nullptr };
		VFSFile *f{ nullptr };
		stack<DirInfo> directories{};
		struct stat st{};

		directories.push({ Engine::GetConfiguration().Engine.DataDirectory, "" });

		// Recursion is evil
		while (!directories.empty())
		{
			DirInfo info = directories.top();
			directories.pop();

			if ((dir = opendir(*info.path)) != NULL)
			{
				while ((ent = readdir(dir)) != NULL)
				{
					NString path = info.path;
					path.Append("/");
					path.Append(ent->d_name);

					if (stat((const char *)*path, &st) < 0)
					{
						Logger::Log(VFS_MODULE, LOG_CRITICAL, "File %s does not exist", *path);
						closedir(dir);
						return ENGINE_FAIL;
					}

					if(S_ISDIR(st.st_mode))
					{
						if (!strncmp(ent->d_name, ".", 1) || !strncmp(ent->d_name, "..", 2))
							continue;

						NString prefix = info.prefix;
						prefix.Append("/");
						prefix.Append(ent->d_name);

						directories.push({ path, prefix });
					}
					else if (S_ISREG(st.st_mode))
					{
						// check if file is compressed		
						uint8_t compressedType = 0;

						memset(buff, 0x0, VFS_MAX_FILE_NAME);
						if (snprintf(buff, VFS_MAX_FILE_NAME, "%s/%s/%s", Engine::GetConfiguration().Engine.DataDirectory, *info.prefix, ent->d_name) >= VFS_MAX_FILE_NAME)
							return ENGINE_FAIL;

						if (__vfs_isCompressed(buff, &compressedType))
						{
							if (compressedType == COMPRESSED_GZIP)
								f = new GZipFile();
							else if (compressedType == COMPRESSED_BZIP2)
								f = new BZip2File();
							else
							{
								Logger::Log(VFS_MODULE, LOG_CRITICAL, "Unknown compression type");
								closedir(dir);
								return ENGINE_FAIL;
							}
						}
						else
							f = new LooseFile();

						if (snprintf(f->GetHeader().name, VFS_MAX_FILE_NAME, "%s/%s", *info.prefix, ent->d_name) >= VFS_MAX_FILE_NAME)
						{
							Logger::Log(VFS_MODULE, LOG_CRITICAL, "snprintf() call failed");
							closedir(dir);
							return ENGINE_FAIL;
						}

						_looseFiles.push_back(f);

						f = nullptr;
					}
				}

				closedir(dir);
			}
			else
			{
				Logger::Log(VFS_MODULE, LOG_CRITICAL, "Failed to open directory: %s", *info.path);
				DIE("Failed to open directory");
			}
		}
	}

	Logger::Log(VFS_MODULE, LOG_INFORMATION, "Initialized");

	return ENGINE_OK;
}

int VFS::LoadArchive(NString path)
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

VFSFile *VFS::Open(NString &path)
{
	if (Engine::GetConfiguration().Engine.LoadLooseFiles)
	{
		size_t len = strlen(*path);

		for (VFSFile *file : _looseFiles)
		{
			if (!strncmp(*path, file->GetHeader().name, len))
			{
				if (file->Open() != ENGINE_OK)
					return nullptr;

				return file;
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

VFSFile *VFS::Create(NString &path, bool compress)
{
	VFSFile *f{ compress ? (VFSFile *)new GZipFile() : (VFSFile *)new LooseFile() };

	if (snprintf(f->GetHeader().name, VFS_MAX_FILE_NAME, "%s", *path) >= VFS_MAX_FILE_NAME)
	{
		Logger::Log(VFS_MODULE, LOG_CRITICAL, "snprintf() call failed");
		delete f;
		return nullptr;
	}

	if (f->Create() != ENGINE_OK)
	{
		Logger::Log(VFS_MODULE, LOG_CRITICAL, "Failed to create file [%s]%s", *path, compress ? " (compressed)" : "");
		delete f;
		return nullptr;
	}

	return f;
}

bool VFS::Exists(NString &path)
{
	VFSFile *f = nullptr;
	if ((f = Open(path)) != nullptr)
	{
		f->Close();
		return true;
	}
	
	return false;
}

void VFS::GetFilesInDirectory(const NString &directory, NArray<VFSFile *> files)
{
	for (VFSFile *file : _looseFiles)
		if (!strncmp(file->GetHeader().name, *directory, directory.Length()))
			files.Add(file);

	for (VFSArchive *archive : _archives)
		archive->GetFilesInDirectory(directory, files);
}

void VFS::Release()
{
	for (VFSFile *file : _looseFiles)
		delete file;
	_looseFiles.clear();

	for (VFSArchive *archive : _archives)
		delete archive;
	_archives.clear();

	Logger::Log(VFS_MODULE, LOG_INFORMATION, "Released");
}
