/* NekoEngine
 *
 * ResourceDatabase.cpp
 * Author: Alexandru Naiman
 *
 * ResourceDatabase class implementation
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

#include <System/Logger.h>
#include <System/VFS/VFS.h>
#include <Engine/ResourceDatabase.h>

#include <vector>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

#include <Resource/MeshResource.h>
#include <Resource/TextureResource.h>
#include <Resource/ShaderModuleResource.h>
#include <Resource/AudioClipResource.h>
#include <Resource/FontResource.h>
#include <Resource/MaterialResource.h>
#include <Resource/AnimationClipResource.h>

#define RD_MODULE				"ResourceDatabase"
#define SQLITE3_VFS_NAME		"NekoEngineVFS"

#define DB_CR_ANIMCLIPS			"CREATE TABLE `animclips` ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `file` TEXT NOT NULL UNIQUE, `comment` TEXT, `name` TEXT NOT NULL UNIQUE );"
#define DB_CR_AUDIOCLIPS		"CREATE TABLE `audioclips` ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `file` TEXT NOT NULL UNIQUE, `comment` TEXT, `name` TEXT NOT NULL UNIQUE );"
#define DB_CR_FONTS				"CREATE TABLE `fonts` ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `name` TEXT NOT NULL UNIQUE, `file` TEXT NOT NULL UNIQUE, `comment` TEXT NOT NULL );"
#define DB_CR_MATERIALS			"CREATE TABLE `materials` ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `file` TEXT NOT NULL UNIQUE, `comment` TEXT, `name` TEXT NOT NULL UNIQUE );"
#define DB_CR_SHADERMODULES		"CREATE TABLE `shadermodules` ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `path` TEXT NOT NULL, `comment` TEXT, `name` TEXT NOT NULL UNIQUE );"
#define DB_CR_SKMESHES			"CREATE TABLE `skmeshes` ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `file` TEXT NOT NULL, `comment` TEXT, `name` TEXT NOT NULL UNIQUE );"
#define DB_CR_STMESHES			"CREATE TABLE `stmeshes` ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `file` TEXT NOT NULL UNIQUE, `comment` TEXT, `name` TEXT NOT NULL UNIQUE );"
#define DB_CR_TEXTURES			"CREATE TABLE `textures` ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `file` TEXT NOT NULL UNIQUE, `type` INTEGER NOT NULL, `comment` TEXT, `name` TEXT NOT NULL UNIQUE );"

#define DB_MC_ANIMCLIPS			"CREATE TABLE memDB.animclips ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `file` TEXT NOT NULL UNIQUE, `comment` TEXT, `name` TEXT NOT NULL UNIQUE );"
#define DB_MC_AUDIOCLIPS		"CREATE TABLE memDB.audioclips ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `file` TEXT NOT NULL UNIQUE, `comment` TEXT, `name` TEXT NOT NULL UNIQUE );"
#define DB_MC_FONTS				"CREATE TABLE memDB.fonts ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `name` TEXT NOT NULL UNIQUE, `file` TEXT NOT NULL UNIQUE, `comment` TEXT NOT NULL );"
#define DB_MC_MATERIALS			"CREATE TABLE memDB.materials ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `file` TEXT NOT NULL UNIQUE, `comment` TEXT, `name` TEXT NOT NULL UNIQUE );"
#define DB_MC_SHADERMODULES		"CREATE TABLE memDB.shadermodules ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `path` TEXT NOT NULL, `comment` TEXT, `name` TEXT NOT NULL UNIQUE );"
#define DB_MC_SKMESHES			"CREATE TABLE memDB.skmeshes ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `file` TEXT NOT NULL, `comment` TEXT, `name` TEXT NOT NULL UNIQUE );"
#define DB_MC_STMESHES			"CREATE TABLE memDB.stmeshes ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `file` TEXT NOT NULL UNIQUE, `comment` TEXT, `name` TEXT NOT NULL UNIQUE );"
#define DB_MC_TEXTURES			"CREATE TABLE memDB.textures ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, `file` TEXT NOT NULL UNIQUE, `type` INTEGER NOT NULL, `comment` TEXT, `name` TEXT NOT NULL UNIQUE );"

#define DB_RB_ANIMCLIPS			"UPDATE memDB.animclips SET id = id + ?;"
#define DB_RB_AUDIOCLIPS		"UPDATE memDB.audioclips SET id = id + ?;"
#define DB_RB_FONTS				"UPDATE memDB.fonts SET id = id + ?;"
#define DB_RB_MATERIALS			"UPDATE memDB.materials SET id = id + ?;"
#define DB_RB_SHADERMODULES		"UPDATE memDB.shadermodules SET id = id + ?;"
#define DB_RB_SKMESHES			"UPDATE memDB.skmeshes SET id = id + ?;"
#define DB_RB_STMESHES			"UPDATE memDB.stmeshes SET id = id + ?;"
#define DB_RB_TEXTURES			"UPDATE memDB.textures SET id = id + ?;"

#define DB_MODULE_BASE_INC		0x01000000
#define DB_NUM_TABLES			8

using namespace std;

typedef struct sqlite3_vfs_file
{
	const struct sqlite3_io_methods *methods;
	VFSFile *file;
} sqlite3_vfs_file;

static char _rdb_resourceToTableMap[10][40]
{
	{ 's', 't', 'm', 'e', 's', 'h', 'e', 's', 0x0 },
	{ 's', 'k', 'm', 'e', 's', 'h', 'e', 's', 0x0 },
	{ 't', 'e', 'x', 't', 'u', 'r', 'e', 's', 0x0 },
	{ 's', 'h', 'a', 'd', 'e', 'r', 'm', 'o', 'd', 'u', 'l', 'e', 's', 0x0 },
	{ 'a', 'u', 'd', 'i', 'o', 'c', 'l', 'i', 'p', 's', 0x0 },
	{ 'f', 'o', 'n', 't', 's', 0x0 },
	{ 'm', 'a', 't', 'e', 'r', 'i', 'a', 'l', 's', 0x0 },
	{ 'a', 'n', 'i', 'm', 'c', 'l', 'i', 'p', 's', 0x0 }
};

static const char *_rdb_createSQL[DB_NUM_TABLES * 2]
{
	DB_CR_ANIMCLIPS,
	DB_CR_AUDIOCLIPS,
	DB_CR_FONTS,
	DB_CR_MATERIALS,
	DB_CR_SHADERMODULES,
	DB_CR_SKMESHES,
	DB_CR_STMESHES,
	DB_CR_TEXTURES,
	DB_MC_ANIMCLIPS,
	DB_MC_AUDIOCLIPS,
	DB_MC_FONTS,
	DB_MC_MATERIALS,
	DB_MC_SHADERMODULES,
	DB_MC_SKMESHES,
	DB_MC_STMESHES,
	DB_MC_TEXTURES
};

static const char *_rdb_rebaseSQL[DB_NUM_TABLES]
{
	DB_RB_ANIMCLIPS,
	DB_RB_AUDIOCLIPS,
	DB_RB_FONTS,
	DB_RB_MATERIALS,
	DB_RB_SHADERMODULES,
	DB_RB_SKMESHES,
	DB_RB_STMESHES,
	DB_RB_TEXTURES
};

/* SQLite VFS info:
 * https://www.sqlite.org/c3ref/vfs.html
 * https://www.sqlite.org/c3ref/file.html
 * https://www.sqlite.org/c3ref/io_methods.html
 * https://www.sqlite.org/vfs.html
 * https://www.sqlite.org/c3ref/vfs_find.html
 * https://www.sqlite.org/rescode.html
 * http://www.sqlite.org/src/doc/trunk/src/test_demovfs.c
 * http://stackoverflow.com/questions/3373816/sqlite-vfs-implementation-guide-lines-with-fopen
 */

static sqlite3_vfs _vfs;
static sqlite3_io_methods _methods;

static int _sq3_vfs_xClose(sqlite3_file *file)
{
	((sqlite3_vfs_file *)file)->file->Close();
	return SQLITE_OK;
}

static int _sq3_vfs_xRead(sqlite3_file *file, void *buff, int amount, sqlite3_int64 offset)
{
	((sqlite3_vfs_file *)file)->file->Seek(offset, SEEK_SET);
	((sqlite3_vfs_file *)file)->file->Read(buff, 1, amount);
	
	return SQLITE_OK;
}

static int _sq3_vfs_xFileSize(sqlite3_file *file, sqlite3_int64 *size)
{
	((sqlite3_vfs_file *)file)->file->Seek(0, SEEK_END);
	*size = ((sqlite3_vfs_file *)file)->file->Tell();
	
	return SQLITE_OK;
}

static int _sq3_vfs_xOpen(sqlite3_vfs *vfs, const char *zName, sqlite3_file *file, int flags, int *outFlags)
{
	(void)vfs;
	(void)flags;
	
	sqlite3_vfs_file *vfsFile = (sqlite3_vfs_file *)file;
	vfsFile->methods = &_methods;
	NString path(zName);
	
	if ((vfsFile->file = VFS::Open(path)) == nullptr)
	{
		vfsFile->methods = nullptr;
		Logger::Log(RD_MODULE, LOG_CRITICAL, "Failed to open file %s", zName);
		return SQLITE_CANTOPEN;
	}
	
	// The engine has read-only access to the database
	*outFlags = SQLITE_OPEN_READONLY;
	
	return SQLITE_OK;
}

static int _sq3_vfs_xAccess(sqlite3_vfs *vfs, const char *zName, int flags, int *resOut)
{
	(void)vfs;
	
	NString path(zName);
	*resOut = flags != SQLITE_ACCESS_READWRITE ? VFS::Exists(path) : 0;
	
	return SQLITE_OK;
}

static int _sq3_vfs_xFullPathname(sqlite3_vfs *vfs, const char *zName, int nOut, char *zOut)
{
	(void)vfs;
	
	if (zName[0] == '/')
		(void)snprintf(zOut, nOut, "%s", zName);
	else
		(void)snprintf(zOut, nOut, "/%s", zName);
	
	return ENGINE_OK;
}

static int _sq3_vfs_xRandomness(sqlite3_vfs *vfs, int nByte, char *zOut)
{
	(void)vfs;
	
	for(int i = 0; i < nByte; ++i)
		zOut[i] = Platform::Rand() % 255;
	
	return SQLITE_OK;
}

static int _sq3_vfs_xSleep(sqlite3_vfs *vfs, int microseconds)
{
	(void)vfs;
	
	Platform::Sleep(microseconds / 1000000);
	Platform::USleep(microseconds % 1000000);
	
	return microseconds;
}

static int _sq3_vfs_xCurrentTime(sqlite3_vfs *vfs, double *time)
{
	(void)vfs;
	*time = Engine::GetTime();
	return SQLITE_OK;
}

static int _sq3_vfs_xGetLastError(sqlite3_vfs *vfs, int num, char *buff)
{
	(void)vfs;
	(void)num;
	(void)buff;
	return SQLITE_OK;
}

static int _sq3_vfs_xWrite(sqlite3_file *file, const void *buff, int amount, sqlite3_int64 offset) { (void)file; (void)buff; (void)amount; (void) offset; return SQLITE_ERROR; }
static int _sq3_vfs_xTruncate(sqlite3_file *file, sqlite3_int64 size) { (void)file; (void)size; return SQLITE_ERROR; }
static int _sq3_vfs_xSync(sqlite3_file *file, int flags) { (void)file; (void)flags; return SQLITE_OK; }
static int _sq3_vfs_xLock(sqlite3_file *file, int lock) { (void)file; (void)lock; return SQLITE_OK; }
static int _sq3_vfs_xUnlock(sqlite3_file *file, int lock) { (void)file; (void)lock; return SQLITE_OK; }
static int _sq3_vfs_xCheckReservedLock(sqlite3_file *file, int *lock) { (void)file; *lock = SQLITE_LOCK_NONE; return SQLITE_OK; }
static int _sq3_vfs_xSectorSize(sqlite3_file *file) { (void)file; return 1; }
static int _sq3_vfs_xDeviceCharacteristics(sqlite3_file *file) { (void)file; return SQLITE_IOCAP_IMMUTABLE; }
static int _sq3_vfs_xFileControl(sqlite3_file *file, int op, void *arg) { (void)file; (void)op; (void)arg; return SQLITE_NOTFOUND; }
static int _sq3_vfs_xDelete(sqlite3_vfs *vfs, const char *zName, int syncDir) { (void)vfs; (void)zName; (void)syncDir; return SQLITE_IOERR_DELETE; }
static void *_sq3_vfs_xDlOpen(sqlite3_vfs *vfs, const char *zName) { (void)vfs; (void)zName; return nullptr; }
static void _sq3_vfs_xDlError(sqlite3_vfs *vfs, int nByte, char *zErrMsg) { (void)vfs; (void)nByte; (void)zErrMsg; }
static void (*(_sq3_vfs_xDlSym(sqlite3_vfs *vfs, void *dl, const char *zSymbol)))(void) { (void)vfs; (void)dl; (void)zSymbol; return nullptr; }
static void _sq3_vfs_xDlClose(sqlite3_vfs *vfs, void *dl) { (void)vfs; (void)dl; }

static inline bool _rdb_InitDB(sqlite3 *db, bool mem) noexcept
{
	char *err{ nullptr };

	for (uint8_t i = 0; i < DB_NUM_TABLES; ++i)
	{
		if (sqlite3_exec(db, _rdb_createSQL[mem ? DB_NUM_TABLES + i : i], NULL, 0, &err) != SQLITE_OK)
		{
			Logger::Log(RD_MODULE, LOG_CRITICAL, "Failed to create table: %s", err);
			return false;
		}
	}

	return true;
}

static inline bool _rdb_TableExists(sqlite3 *db, const char *table) noexcept
{
	if (table == nullptr)
		return false;

	sqlite3_stmt *stmt{ nullptr };

	if (sqlite3_prepare(db, "SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name = ?;", -1, &stmt, nullptr) != SQLITE_OK)
		return false;

	if (sqlite3_bind_text(stmt, 1, table, (int)strlen(table), SQLITE_STATIC) != SQLITE_OK)
		return false;

	if (sqlite3_step(stmt) != SQLITE_ROW)
	{
		sqlite3_finalize(stmt);
		return false;
	}

	if (sqlite3_column_int(stmt, 0) != 1)
	{
		sqlite3_finalize(stmt);
		return false;
	}

	sqlite3_finalize(stmt);

	return true;
}

static inline bool _rdb_CheckDB(sqlite3 *db) noexcept
{
	for (uint8_t i = 0; i < DB_NUM_TABLES; ++i)
		if (!_rdb_TableExists(db, _rdb_resourceToTableMap[i]))
			return false;
	
	return true;
}

static inline bool _rdb_RebaseDB(sqlite3 *db, uint32_t newBaseId) noexcept
{
	int err{ 0 };
	for (uint8_t i = 0; i < DB_NUM_TABLES; ++i)
	{
		sqlite3_stmt *stmt{ nullptr };

		if ((err = sqlite3_prepare(db, _rdb_rebaseSQL[i], -1, &stmt, nullptr)) != SQLITE_OK)
		{
			Logger::Log(RD_MODULE, LOG_CRITICAL, "sqlite3_prepare() failed code = %d, msg = %s", err, sqlite3_errmsg(db));
			return false;
		}

		if ((err = sqlite3_bind_int(stmt, 1, newBaseId)) != SQLITE_OK)
		{
			Logger::Log(RD_MODULE, LOG_CRITICAL, "sqlite3_bind_int() failed code = %d, msg = %s", err, sqlite3_errmsg(db));
			return false;
		}

		if ((err = sqlite3_step(stmt)) != SQLITE_DONE)
		{
			sqlite3_finalize(stmt);
			Logger::Log(RD_MODULE, LOG_CRITICAL, "sqlite3_step() failed code = %d, msg = %s", err, sqlite3_errmsg(db));
			return false;
		}

		sqlite3_finalize(stmt);
	}

	return true;
}

ResourceDatabase::ResourceDatabase() noexcept : _db(nullptr), _nextModuleBaseId(0) {}

int ResourceDatabase::Initialize() noexcept
{
	memset(&_vfs, 0x0, sizeof(sqlite3_vfs));
	memset(&_methods, 0x0, sizeof(sqlite3_io_methods));

	_vfs.iVersion = 1;
	_vfs.szOsFile = sizeof(sqlite3_vfs_file);
	_vfs.mxPathname = VFS_MAX_FILE_NAME;
	_vfs.pNext = nullptr;
	_vfs.zName = SQLITE3_VFS_NAME;
	_vfs.pAppData = nullptr;
	_vfs.xOpen = _sq3_vfs_xOpen;
	_vfs.xDelete = _sq3_vfs_xDelete;
	_vfs.xAccess = _sq3_vfs_xAccess;
	_vfs.xFullPathname = _sq3_vfs_xFullPathname;
	_vfs.xDlOpen = _sq3_vfs_xDlOpen;
	_vfs.xDlError = _sq3_vfs_xDlError;
	_vfs.xDlSym = _sq3_vfs_xDlSym;
	_vfs.xDlClose = _sq3_vfs_xDlClose;
	_vfs.xRandomness = _sq3_vfs_xRandomness;
	_vfs.xSleep = _sq3_vfs_xSleep;
	_vfs.xCurrentTime = _sq3_vfs_xCurrentTime;
	_vfs.xGetLastError = _sq3_vfs_xGetLastError;

	_methods.iVersion = 1;
	_methods.xClose = _sq3_vfs_xClose;
	_methods.xRead = _sq3_vfs_xRead;
	_methods.xWrite = _sq3_vfs_xWrite;
	_methods.xTruncate = _sq3_vfs_xTruncate;
	_methods.xSync = _sq3_vfs_xSync;
	_methods.xFileSize = _sq3_vfs_xFileSize;
	_methods.xLock = _sq3_vfs_xLock;
	_methods.xUnlock = _sq3_vfs_xUnlock;
	_methods.xCheckReservedLock = _sq3_vfs_xCheckReservedLock;
	_methods.xFileControl = _sq3_vfs_xFileControl;
	_methods.xSectorSize = _sq3_vfs_xSectorSize;
	_methods.xDeviceCharacteristics = _sq3_vfs_xDeviceCharacteristics;

	if (sqlite3_vfs_register(&_vfs, true) != SQLITE_OK)
	{
		Logger::Log(RD_MODULE, LOG_CRITICAL, "Failed to register the SQLite VFS module");
		return ENGINE_FAIL;
	}

	sqlite3_open(":memory:", &_db);
	
	if (!_rdb_InitDB(_db, false))
		return ENGINE_DB_INIT_FAIL;

	return ENGINE_OK;
}

bool ResourceDatabase::Load(const char *file) noexcept
{
	char sql[4096]{};
	sqlite3 *filedb{ nullptr };
	char *err{ nullptr };

	if (!sqlite3_vfs_find(SQLITE3_VFS_NAME))
	{
		Logger::Log(RD_MODULE, LOG_CRITICAL, "SQLite3 VFS module not found. Is the database initialized ?");
		return false;
	}

	if (sqlite3_open_v2(file, &filedb, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK)
	{
		Logger::Log(RD_MODULE, LOG_CRITICAL, "Failed to open file %s", file);
		return false;
	}

	if (!_rdb_CheckDB(filedb))
	{
		Logger::Log(RD_MODULE, LOG_CRITICAL, "%s is not a valid database", file);
		return false;
	}

	snprintf(sql, 4096, "attach ':memory:' as memDB;");
	if (sqlite3_exec(_db, sql, NULL, 0, &err) != SQLITE_OK)
	{
		Logger::Log(RD_MODULE, LOG_CRITICAL, "Failed to create temporary database: %s", err);
		return false;
	}

	if (!_rdb_InitDB(_db, true))
		return false;

	memset(sql, 0x0, 4096);
	snprintf(sql, 4096, "attach '%s' as mergeDB;", file);
	if (sqlite3_exec(_db, sql, NULL, 0, &err) != SQLITE_OK)
	{
		Logger::Log(RD_MODULE, LOG_CRITICAL, "Failed to attach database: %s", err);
		return false;
	}

	memset(sql, 0x0, 4096);
	snprintf(sql, 4096, "BEGIN;\ninsert into memDB.stmeshes select * from mergeDB.stmeshes;\ninsert into memDB.skmeshes select * from mergeDB.skmeshes;\ninsert into memDB.textures select * from mergeDB.textures;\ninsert into memDB.shadermodules select * from mergeDB.shadermodules;\ninsert into memDB.audioclips select * from mergeDB.audioclips;\ninsert into memDB.fonts select * from mergeDB.fonts;\ninsert into memDB.materials select * from mergeDB.materials;\ninsert into memDB.animclips select * from mergeDB.animclips;\nCOMMIT;\ndetach mergeDB;");
	if (sqlite3_exec(_db, sql, NULL, 0, &err) != SQLITE_OK)
	{
		Logger::Log(RD_MODULE, LOG_CRITICAL, "Failed to clone %s database: %s", file, err);
		return false;
	}

	if (!_rdb_RebaseDB(_db, _nextModuleBaseId))
	{
		Logger::Log(RD_MODULE, LOG_CRITICAL, "Failed to rebase %s database", file);
		return false;
	}

	_nextModuleBaseId += DB_MODULE_BASE_INC;

	memset(sql, 0x0, 4096);
	snprintf(sql, 4096, "BEGIN;\ninsert into stmeshes select * from memDB.stmeshes;\ninsert into skmeshes select * from memDB.skmeshes;\ninsert into textures select * from memDB.textures;\ninsert into shadermodules select * from memDB.shadermodules;\ninsert into audioclips select * from memDB.audioclips;\ninsert into fonts select * from memDB.fonts;\ninsert into materials select * from memDB.materials;\ninsert into animclips select * from memDB.animclips;\nCOMMIT;\ndetach memDB;");
	if (sqlite3_exec(_db, sql, NULL, 0, &err) != SQLITE_OK)
	{
		Logger::Log(RD_MODULE, LOG_CRITICAL, "Failed to merge %s database: %s", file, err);
		return false;
	}
	sqlite3_close(filedb);
	return true;
}

bool ResourceDatabase::GetResources(vector<ResourceInfo *> &vec)
{
	sqlite3_stmt *stmt = nullptr;
	char buff[80];
	memset(buff, 0x0, sizeof(buff));

	for (unsigned int i = 0; i < (unsigned int)ResourceType::RES_END; i++)
	{
		// SQLite3 does not allow passing the table name as a parameter
		if (snprintf(buff, 80, "SELECT * FROM %s", _rdb_resourceToTableMap[i]) >= 80)
			return false;

		if (sqlite3_prepare(_db, buff, -1, &stmt, nullptr) != SQLITE_OK)
			return false;

		switch ((ResourceType)i)
		{
			case ResourceType::RES_STATIC_MESH:
			{
				while (sqlite3_step(stmt) == SQLITE_ROW)
				{
					MeshResource *res = new MeshResource();
					res->id = sqlite3_column_int(stmt, 0);
					res->filePath = (const char *)sqlite3_column_text(stmt, 1);					
					const unsigned char *ptr = sqlite3_column_text(stmt, 2);
					if (ptr)
						res->comment = (const char *)ptr;
					ptr = sqlite3_column_text(stmt, 3);
					if (ptr)
						res->name = (const char *)ptr;
					vec.push_back(res);
				}
			}
			break;
			case ResourceType::RES_SKELETAL_MESH:
			{
				while (sqlite3_step(stmt) == SQLITE_ROW)
				{
					MeshResource *res = new MeshResource();
					res->meshType = MeshType::Skeletal;
					res->type = ResourceType::RES_SKELETAL_MESH;
					res->id = sqlite3_column_int(stmt, 0);
					res->filePath = (const char *)sqlite3_column_text(stmt, 1);
					const unsigned char *ptr = sqlite3_column_text(stmt, 2);
					if (ptr)
						res->comment = (const char *)ptr;
					ptr = sqlite3_column_text(stmt, 3);
					if (ptr)
						res->name = (const char *)ptr;
					vec.push_back(res);
				}
			}
			break;
			case ResourceType::RES_TEXTURE:
			{
				while (sqlite3_step(stmt) == SQLITE_ROW)
				{
					TextureResource *res = new TextureResource();
					res->id = sqlite3_column_int(stmt, 0);
					res->filePath = (const char *)sqlite3_column_text(stmt, 1);
					res->textureType = (TextureResourceType)sqlite3_column_int(stmt, 2);
					const unsigned char *ptr = sqlite3_column_text(stmt, 3);
					if (ptr)
						res->comment = (const char *)ptr;
					ptr = sqlite3_column_text(stmt, 4);
					if (ptr)
						res->name = (const char *)ptr;
					vec.push_back(res);
				}
			}
			break;
			case ResourceType::RES_SHADERMODULE:
			{
				while (sqlite3_step(stmt) == SQLITE_ROW)
				{
					ShaderModuleResource *res = new ShaderModuleResource();
					res->id = sqlite3_column_int(stmt, 0);
					res->filePath = (const char *)sqlite3_column_text(stmt, 1);

					const unsigned char *ptr = sqlite3_column_text(stmt, 2);
					if (ptr)
						res->comment = (const char *)ptr;

					ptr = sqlite3_column_text(stmt, 3);
					if (ptr)
						res->name = (const char *)ptr;

					vec.push_back(res);
				}
			}
			break;
			case ResourceType::RES_AUDIOCLIP:
			{
				while (sqlite3_step(stmt) == SQLITE_ROW)
				{
					AudioClipResource *res = new AudioClipResource();
					res->id = sqlite3_column_int(stmt, 0);
					res->filePath = (const char *)sqlite3_column_text(stmt, 1);
					const unsigned char *ptr = sqlite3_column_text(stmt, 2);
					if (ptr)
						res->comment = (const char *)ptr;
					ptr = sqlite3_column_text(stmt, 3);
					if (ptr)
						res->name = (const char *)ptr;
					vec.push_back(res);
				}
			}
			break;
			case ResourceType::RES_FONT:
			{
				while (sqlite3_step(stmt) == SQLITE_ROW)
				{
					FontResource *res = new FontResource();
					res->id = sqlite3_column_int(stmt, 0);
					const unsigned char *ptr = sqlite3_column_text(stmt, 1);
					if (ptr)
						res->name = (const char *)ptr;
					ptr = sqlite3_column_text(stmt, 2);
					if (ptr)
						res->filePath = (const char *)ptr;
					ptr = sqlite3_column_text(stmt, 3);
					if (ptr)
						res->comment = (const char *)ptr;
					vec.push_back(res);
				}
			}
			break;
			case ResourceType::RES_MATERIAL:
			{
				while (sqlite3_step(stmt) == SQLITE_ROW)
				{
					MaterialResource *res = new MaterialResource();
					res->id = sqlite3_column_int(stmt, 0);
					res->filePath = (const char *)sqlite3_column_text(stmt, 1);
					const unsigned char *ptr = sqlite3_column_text(stmt, 2);
					if (ptr)
						res->comment = (const char *)ptr;
					ptr = sqlite3_column_text(stmt, 3);
					if (ptr)
						res->name = (const char *)ptr;
					vec.push_back(res);
				}
			}
			break;
			case ResourceType::RES_ANIMCLIP:
			{
				while (sqlite3_step(stmt) == SQLITE_ROW)
				{
					AnimationClipResource *res = new AnimationClipResource();
					res->id = sqlite3_column_int(stmt, 0);
					res->filePath = (const char *)sqlite3_column_text(stmt, 1);
					const unsigned char *ptr = sqlite3_column_text(stmt, 2);
					if (ptr)
						res->comment = (const char *)ptr;
					ptr = sqlite3_column_text(stmt, 3);
					if (ptr)
						res->name = (const char *)ptr;
					vec.push_back(res);
				}
			}
			break;
			default:
				continue;
		}

		sqlite3_finalize(stmt);
	}
	
	return true;
}

ResourceDatabase::~ResourceDatabase() noexcept
{
	sqlite3_close(_db);
	sqlite3_vfs_unregister(&_vfs);
}
