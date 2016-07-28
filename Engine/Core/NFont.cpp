/* Neko Engine
 *
 * NFont.cpp
 * Author: Alexandru Naiman
 *
 * FreeType font renderer implementation
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

#include <algorithm>

#include <Engine/Engine.h>
#include <Engine/Vertex.h>
#include <Engine/Texture.h>
#include <Engine/NFont.h>
#include <System/VFS/VFS.h>

#define NFONT_MODULE	"NFont"

using namespace std;

NFont::NFont(FontResource *res)
{
	_resourceInfo = res;

	memset(_characterInfo, 0x0, sizeof(NFontCharacterInfo) * NFONT_NUM_CHARS);
	_vboSize = 0;
	_iboSize = 0;
	_texWidth = 0;
	_texHeight = 0;
	_texture = nullptr;
	_vertexBuffer = nullptr;
	_indexBuffer = nullptr;
	_arrayBuffer = nullptr;
}

int NFont::Load()
{
	FT_Face face;
	FT_GlyphSlot glyph;
	VFSFile *file = nullptr;
	uint64_t size = 0;
	uint8_t *mem = nullptr;
	uint32_t x = 0;

	if ((file = VFS::Open(GetResourceInfo()->filePath)) == nullptr)
	{
		Logger::Log(NFONT_MODULE, LOG_CRITICAL, "Failed to open file for font id %d, file name [%s].", GetResourceInfo()->id, GetResourceInfo()->filePath.c_str());
		return ENGINE_FAIL;
	}

	if (file->Seek(0, SEEK_END) == ENGINE_FAIL)
	{
		file->Close();
		Logger::Log(NFONT_MODULE, LOG_CRITICAL, "Seek failed for file [%].", GetResourceInfo()->filePath.c_str());
		return ENGINE_FAIL;
	}

	size = file->Tell();

	if (file->Seek(0, SEEK_SET) == ENGINE_FAIL)
	{
		file->Close();
		Logger::Log(NFONT_MODULE, LOG_CRITICAL, "Seek failed for file [%].", GetResourceInfo()->filePath.c_str());
		return ENGINE_FAIL;
	}

	mem = (uint8_t*)calloc((size_t)size, sizeof(uint8_t));
	if (file->Read(mem, sizeof(uint8_t), size) == 0)
	{
		file->Close();
		free(mem);
		Logger::Log(NFONT_MODULE, LOG_CRITICAL, "Failed to read file [%].", GetResourceInfo()->filePath.c_str());
		return ENGINE_FAIL;
	}

	if (FT_New_Memory_Face(Engine::GetFTLibrary(), mem, size, 0, &face))
	{
		file->Close();
		free(mem);
		Logger::Log(NFONT_MODULE, LOG_CRITICAL, "Failed to load font face for id %s.", GetResourceInfo()->name.c_str());
		return ENGINE_FAIL;
	}

	glyph = face->glyph;

	FT_Set_Pixel_Sizes(face, 0, 15);

	for (int i = NFONT_START_CHAR; i < NFONT_NUM_CHARS; ++i)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
		{
			Logger::Log(NFONT_MODULE, LOG_WARNING, "Failed to load character %c for font %s", (char)i, GetResourceInfo()->name.c_str());
			continue;
		}

		_texWidth += glyph->bitmap.width;
		_texHeight = max(_texHeight, glyph->bitmap.rows);
	}

	if ((_texture = Engine::GetRenderer()->CreateTexture(TextureType::Tex2D)) == nullptr)
	{ DIE("Out of resources"); }
	_texture->SetStorage2D(1, TextureSizedFormat::R_8U, _texWidth, _texHeight);

	for (int i = NFONT_START_CHAR; i < NFONT_NUM_CHARS; ++i)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER)) continue;
		_texture->SetSubImage2D(0, x, 0, glyph->bitmap.width, glyph->bitmap.rows, TextureFormat::RED, TextureInternalType::UnsignedByte, glyph->bitmap.buffer);
		x += glyph->bitmap.width;
	}

	/*if ((_vertexBuffer = Engine::GetRenderer()->CreateBuffer(BufferType::Vertex, false, true)) == nullptr)
	{ DIE("Out of resources"); }
	_vertexBuffer->SetNumBuffers(1);
	//_vertexBuffer->SetStorage(sizeof(Vertex) * vboSize, nullptr);

	if ((_indexBuffer = Engine::GetRenderer()->CreateBuffer(BufferType::Index, false, true)) == nullptr)
	{ DIE("Out of resources"); }
	_indexBuffer->SetNumBuffers(1);
	//_indexBuffer->SetStorage(sizeof(uint32_t) * iboSize, nullptr);*/

	FT_Done_Face(face);
	free(mem);

	Logger::Log(NFONT_MODULE, LOG_DEBUG, "Loaded font %s [%d] from %s", GetResourceInfo()->name.c_str(), _resourceInfo->id, GetResourceInfo()->filePath.c_str());

	return ENGINE_OK;
}

void NFont::Draw(std::string text, glm::vec2& pos) noexcept
{
	//
}

void NFont::Draw(std::string text, glm::vec2& pos, glm::vec3& color) noexcept
{
	//
}

void NFont::Render()
{
	//
}

NFont::~NFont()
{
	//
}