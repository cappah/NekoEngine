/* Neko Engine Archive Tool
 *
 * nar.cpp
 * Author: Alexandru Naiman
 *
 * Neko Engine Tools
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stack>
#include <string>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#define stat _stat
#endif

#define VFS_MAGIC			0xB16B00B5
#define VFS_AR_VERSION		0x00000001
#define VFS_MAX_FILES		10000

#define VFS_MAX_FILE_NAME		1024
#define VFS_MAX_DIR_NAME		1024

#define COPY_BUFF_SIZE	524288	// 512k

using namespace std;

typedef struct VFS_FILE_HEADER
{
	char name[VFS_MAX_FILE_NAME];
	char dir[VFS_MAX_DIR_NAME];
	uint64_t start;
	uint64_t size;
} VFSFileHeader;

typedef struct VFS_ARCHIVE_HEADER
{
	int32_t magic;
	int32_t version;
	uint32_t num_files;
} VFSArchiveHeader;

typedef struct DIR_INFO
{
	string path;
	string prefix;
} DirInfo;

void inline usage(const char *name)
{
	printf("usage:\n\t%s create <input directory> <output file>\n\t%s list <archive file>\n\t%s extract <archive file> <output directory>\n", name, name, name);
	exit(0);
}

int inline copy_file(FILE *in, FILE *out, size_t size)
{
	uint8_t *buff = (uint8_t *)calloc(1, COPY_BUFF_SIZE);
	size_t remain = size;
	size_t copy_size = COPY_BUFF_SIZE > size ? size : COPY_BUFF_SIZE;

	while (remain)
	{
		size_t read = fread(buff, 1, copy_size, in);
		fwrite(buff, 1, read, out);
		remain -= read;
		copy_size = COPY_BUFF_SIZE > remain ? remain : COPY_BUFF_SIZE;
	}

	free(buff);

	return 0;
}

void inline create_directory(const char *dir)
{
#ifdef _WIN32
	_mkdir(dir);
#else
	mkdir(dir, 0777);
#endif
}

int inline create_archive(const char *root_dir, const char *archive_file)
{
	VFSArchiveHeader archiveHeader;
	DIR *dir;
	struct dirent *ent;
	stack<DirInfo> directories;
	vector<VFSFileHeader> fileHeaders;
	vector<string> filePaths;
	struct stat st;
	uint64_t fileOffset = 0;

	archiveHeader.magic = VFS_MAGIC;
	archiveHeader.version = VFS_AR_VERSION;

	// Build list of files
	directories.push({ root_dir, "" });

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
					fprintf(stderr, "file %s does not exist", path.c_str());
					closedir(dir);
					return -1;
				}

				if (S_ISDIR(st.st_mode))
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
					VFSFileHeader h = { 0 };

					h.size = st.st_size;
					h.start = fileOffset;
					fileOffset += h.size;

					if (snprintf(h.dir, VFS_MAX_FILE_NAME, "%s", info.prefix.c_str()) >= VFS_MAX_DIR_NAME)
					{
						fprintf(stderr, "snprintf() call failed");
						closedir(dir);
						return -1;
					}

					if (snprintf(h.name, VFS_MAX_FILE_NAME, "%s/%s", info.prefix.c_str(), ent->d_name) >= VFS_MAX_FILE_NAME)
					{
						fprintf(stderr, "snprintf() call failed");
						closedir(dir);
						return -1;
					}

					fileHeaders.push_back(h);
					filePaths.push_back(path);
				}
			}

			closedir(dir);
		}
		else
		{
			fprintf(stderr, "failed to open directory: %s", info.path.c_str());
			return -1;
		}
	}
	
	if(fileHeaders.size() > VFS_MAX_FILES)
	{
		fprintf(stderr, "the maximum number of files supported by a VFS acrhive is %d. You are trying to create an archive with %lu files\n", VFS_MAX_FILES, fileHeaders.size());
		return -1;
	}

	// Create archive

	printf("Packing %lu %s in %s:\n", fileHeaders.size(), fileHeaders.size() > 1 ? "files" : "file", archive_file);

	FILE *fp = fopen(archive_file, "wb");
	if (!fp)
	{
		fprintf(stderr, "failed to open file for writing\n");
		return -1;
	}

	archiveHeader.num_files = fileHeaders.size();

	fwrite(&archiveHeader, sizeof(VFSArchiveHeader), 1, fp);

	for (VFSFileHeader h : fileHeaders)
		fwrite(&h, sizeof(VFSFileHeader), 1, fp);

	for (size_t i = 0; i < filePaths.size(); ++i)
	{
		FILE *in = fopen(filePaths[i].c_str(), "rb");

		if (copy_file(in, fp, fileHeaders[i].size))
		{
			fprintf(stderr, "failed to copy file: %s", filePaths[i].c_str());
			return -1;
		}

		fclose(in);

		printf("%s\n", fileHeaders[i].name);
	}

	fclose(fp);

	printf("Archive %s created.\n", archive_file);
    
    return 0;
}

int inline list_files(const char *archive_file)
{
	VFSArchiveHeader archiveHeader;
	FILE *fp = fopen(archive_file, "rb");
	if (!fp)
	{
		fprintf(stderr, "failed to open file\n");
		return -1;
	}

	memset(&archiveHeader, 0x0, sizeof(VFSArchiveHeader));
	fread(&archiveHeader, sizeof(VFSArchiveHeader), 1, fp);

	printf("File listing for %s:\n", archive_file);

	for (uint32_t i = 0; i < archiveHeader.num_files; ++i)
	{
		VFSFileHeader fileHeader;
		fread(&fileHeader, sizeof(VFSFileHeader), 1, fp);

		printf("%s\t%lld\n", fileHeader.name, fileHeader.size);
	}

	printf("%d %s in archive.\n", archiveHeader.num_files, archiveHeader.num_files > 1 ? "files" : "file");

	return 0;
}

int inline extract_archive(const char *archive_file, const char *out_dir)
{
	char path[VFS_MAX_FILE_NAME];
	uint32_t headerOffset = sizeof(VFSArchiveHeader);
	VFSArchiveHeader archiveHeader;
	FILE *fp = fopen(archive_file, "rb");
	if (!fp)
	{
		fprintf(stderr, "failed to open file\n");
		return -1;
	}

	memset(&archiveHeader, 0x0, sizeof(VFSArchiveHeader));
	fread(&archiveHeader, sizeof(VFSArchiveHeader), 1, fp);

	printf("Extracting %s:\n", archive_file);

	create_directory(out_dir);

	for (uint32_t i = 0; i < archiveHeader.num_files; ++i)
	{
		VFSFileHeader fileHeader;
		fseek(fp, headerOffset, SEEK_SET);
		fread(&fileHeader, sizeof(VFSFileHeader), 1, fp);

		fseek(fp, sizeof(VFSArchiveHeader) + sizeof(VFSFileHeader) * archiveHeader.num_files + fileHeader.start, SEEK_SET);

		memset(path, 0x0, VFS_MAX_FILE_NAME);
		snprintf(path, VFS_MAX_FILE_NAME, "%s%s", out_dir, fileHeader.dir);
		create_directory(path);

		memset(path, 0x0, VFS_MAX_FILE_NAME);
		snprintf(path, VFS_MAX_FILE_NAME, "%s%s", out_dir, fileHeader.name);

		FILE *out = fopen(path, "wb");
		copy_file(fp, out, fileHeader.size);
		fclose(out);

		printf("%s\n", path);

		headerOffset += sizeof(VFSFileHeader);
	}

	printf("Extracted %d %s.\n", archiveHeader.num_files, archiveHeader.num_files > 1 ? "files" : "file");

	return 0;
}

int main(int argc, char *argv[])
{
    printf("NekoEngine Archive Tool\nVersion: 0.3.0a\n(C) 2016 Alexandru Naiman. All rights reserved.\n\n");
	if (argc < 3)
		usage(argv[0]);

	size_t len = strlen(argv[1]);

	if (!strncmp("create", argv[1], len))
	{
		if (argc != 4)
			usage(argv[0]);

		return create_archive(argv[2], argv[3]);
	}
	else if (!strncmp("list", argv[1], len))
	{
		if (argc != 3)
			usage(argv[0]);

		return list_files(argv[2]);
	}
	else if (!strncmp("extract", argv[1], len))
	{
		if (argc != 4)
			usage(argv[0]);

		return extract_archive(argv[2], argv[3]);
	}
	else
		usage(argv[0]);

	return 0;
}
