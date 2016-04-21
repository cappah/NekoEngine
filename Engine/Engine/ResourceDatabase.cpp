/* Neko Engine
 *
 * ResourceDatabase.cpp
 * Author: Alexandru Naiman
 *
 * ResourceDatabase class implementation
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

#include <Engine/ResourceDatabase.h>

#include <vector>
#include <stdio.h>
#include <string.h>

#include <Resource/MeshResource.h>
#include <Resource/TextureResource.h>
#include <Resource/ShaderResource.h>
#include <Resource/AudioClipResource.h>
#include <Resource/TextureFontResource.h>
#include <Resource/MaterialResource.h>

using namespace std;

char resource_to_table_map[10][40] =
{
	{ 'm', 'e', 's', 'h', 'e', 's', 0x0 },
	{ 's', 'k', 'm', 'e', 's', 'h', 'e', 's', 0x0 },
	{ 't', 'e', 'x', 't', 'u', 'r', 'e', 's', 0x0 },
	{ 's', 'h', 'a', 'd', 'e', 'r', 's', 0x0 },
	{ 'a', 'u', 'd', 'i', 'o', 'c', 'l', 'i', 'p', 's', 0x0 },
	{ 'f', 'o', 'n', 't', 's', 0x0 },
	{ 'm', 'a', 't', 'e', 'r', 'i', 'a', 'l', 's', 0x0 }
};

bool ResourceDatabase::Open(const char *file) noexcept
{
	if (sqlite3_open_v2(file, &_db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK)
		return false;

	return _CheckDatabase();
}

bool ResourceDatabase::GetResources(vector<ResourceInfo *> &vec)
{
	sqlite3_stmt *stmt;
	char buff[80];

	for (unsigned int i = 0; i < (unsigned int)ResourceType::RES_END; i++)
	{
		// SQLite3 does not allow passing the table name as a parameter
		if (snprintf(buff, 80, "SELECT * FROM %s", resource_to_table_map[i]) >= 80)
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
			case ResourceType::RES_SHADER:
			{
				while (sqlite3_step(stmt) == SQLITE_ROW)
				{
					ShaderResource *res = new ShaderResource();
					res->id = sqlite3_column_int(stmt, 0);
					res->vsFilePath = (const char *)sqlite3_column_text(stmt, 1);
					res->fsFilePath = (const char *)sqlite3_column_text(stmt, 2);

					const unsigned char *ptr = sqlite3_column_text(stmt, 5);
					if(ptr)
						res->gsFilePath = (const char *)ptr;

					res->numTextures = sqlite3_column_int(stmt, 3);

					ptr = sqlite3_column_text(stmt, 4);
					if (ptr)
						res->comment = (const char *)ptr;

					ptr = sqlite3_column_text(stmt, 6);
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
					TextureFontResource *res = new TextureFontResource();
					res->id = sqlite3_column_int(stmt, 0);
					res->textureId = sqlite3_column_int(stmt, 1);
					res->shaderId = sqlite3_column_int(stmt, 2);
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
			default:
				continue;
		}

		sqlite3_finalize(stmt);
	}
	
	return true;
}

bool ResourceDatabase::_CheckDatabase() noexcept
{
	if (!_TableExists("meshes"))
		return false;
	
	if (!_TableExists("skmeshes"))
		return false;

	if (!_TableExists("textures"))
		return false;

	if (!_TableExists("shaders"))
		return false;

	if (!_TableExists("audioclips"))
		return false;

	if (!_TableExists("fonts"))
		return false;

	if (!_TableExists("materials"))
		return false;

	return true;
}

bool ResourceDatabase::_TableExists(const char *table) noexcept
{
	if (table == nullptr)
		return false;

	sqlite3_stmt *stmt;

	if (sqlite3_prepare(_db, "SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name = ?;", -1, &stmt, nullptr) != SQLITE_OK)
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

ResourceDatabase::~ResourceDatabase() noexcept
{
	sqlite3_close(_db);
}
