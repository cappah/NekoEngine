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

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Engine/Engine.h>
#include <Engine/Vertex.h>
#include <Engine/Shader.h>
#include <Engine/Texture.h>
#include <Engine/NFont.h>
#include <System/VFS/VFS.h>
#include <Engine/ResourceManager.h>

#define NFONT_MODULE	"NFont"

using namespace std;
using namespace glm;

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
	_uniformBuffer = nullptr;
	_arrayBuffer = nullptr;
	_shader = nullptr;
	_pixelSize = 16;
}

int NFont::Load()
{
	int ret = ENGINE_FAIL;

	if ((_shader = (Shader *)ResourceManager::GetResourceByName("sh_font", ResourceType::RES_SHADER)) == nullptr)
	{
		Logger::Log(NFONT_MODULE, LOG_CRITICAL, "Failed to load font shader");
		return ENGINE_FAIL;
	}

	if ((ret = _BuildAtlas()) != ENGINE_OK)
		return ret;
	
	if ((_arrayBuffer = Engine::GetRenderer()->CreateArrayBuffer()) == nullptr)
	{ DIE("Out of resources"); }
	_arrayBuffer->SetVertexBuffer(_vertexBuffer);
	_arrayBuffer->SetIndexBuffer(_indexBuffer);
	_arrayBuffer->CommitBuffers();

	_projection = ortho(0.f, (float)Engine::GetScreenWidth(), 0.f, (float)Engine::GetScreenHeight());

	_uniformBuffer = Engine::GetRenderer()->CreateBuffer(BufferType::Uniform, true, false);
	_uniformBuffer->SetNumBuffers(1);
	_uniformBuffer->SetStorage(sizeof(mat4), value_ptr(_projection));

	_shader->GetRShader()->VSUniformBlockBinding(0, "DataBlock");
	_shader->GetRShader()->VSSetUniformBuffer(0, 0, sizeof(mat4), _uniformBuffer);

	Logger::Log(NFONT_MODULE, LOG_DEBUG, "Loaded font %s [%d] from %s", GetResourceInfo()->name.c_str(), _resourceInfo->id, GetResourceInfo()->filePath.c_str());

	return ENGINE_OK;
}

int NFont::_BuildAtlas()
{
	FT_Face face;
	FT_GlyphSlot glyph;
	VFSFile *file = nullptr;
	uint64_t size = 0;
	uint8_t *mem = nullptr;
	uint32_t x = 0, vboSize = 0, iboSize = 0, maxChars = 0;

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

	if (FT_New_Memory_Face(Engine::GetFTLibrary(), mem, (FT_Long)size, 0, &face))
	{
		file->Close();
		free(mem);
		Logger::Log(NFONT_MODULE, LOG_CRITICAL, "Failed to load font face for id %s.", GetResourceInfo()->name.c_str());
		return ENGINE_FAIL;
	}

	FT_Set_Pixel_Sizes(face, 0, _pixelSize);

	glyph = face->glyph;

	for (int i = 0; i < NFONT_NUM_CHARS; ++i)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
		{
			Logger::Log(NFONT_MODULE, LOG_WARNING, "Failed to load character %c for font %s", (char)i, GetResourceInfo()->name.c_str());
			continue;
		}

		_texWidth += glyph->bitmap.width;
		_texHeight = std::max(_texHeight, glyph->bitmap.rows);
	}

	Logger::Log(NFONT_MODULE, LOG_DEBUG, "Creating font texture atlas with size %dx%d", _texWidth, _texHeight);

	if ((_texture = Engine::GetRenderer()->CreateTexture(TextureType::Tex2D)) == nullptr)
	{ DIE("Out of resources"); }
	_texture->SetStorage2D(1, TextureSizedFormat::R_8U, _texWidth, _texHeight);

	_texture->SetWrapS(TextureWrap::ClampToEdge);
	_texture->SetWrapT(TextureWrap::ClampToEdge);

	_texture->SetMinFilter(TextureFilter::Linear);
	_texture->SetMagFilter(TextureFilter::Linear);

	Engine::GetRenderer()->SetPixelStore(PixelStoreParameter::UnpackAlignment, 1);

	for (int i = 0; i < NFONT_NUM_CHARS; ++i)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER)) continue;

		_texture->SetSubImage2D(0, x, 0, glyph->bitmap.width, glyph->bitmap.rows, TextureFormat::RED, TextureInternalType::UnsignedByte, glyph->bitmap.buffer);

		_characterInfo[i].size = ivec2(glyph->bitmap.width, glyph->bitmap.rows);
		_characterInfo[i].bearing = ivec2(glyph->bitmap_left, glyph->bitmap_top);
		_characterInfo[i].advance = (unsigned int)glyph->advance.x >> 6;
		_characterInfo[i].offset = (float)x / _texWidth;

		x += glyph->bitmap.width;
	}

	Engine::GetRenderer()->SetPixelStore(PixelStoreParameter::UnpackAlignment, 4);

	maxChars = (int)((1.f / (((float)_texWidth / (float)NFONT_NUM_CHARS) * (1.f / Engine::GetScreenWidth()))) * (1.f / (_texHeight * (1.f / Engine::GetScreenHeight()))));

	vboSize = maxChars * 4;
	iboSize = maxChars * 6;

	if ((_vertexBuffer = Engine::GetRenderer()->CreateBuffer(BufferType::Vertex, false, true)) == nullptr)
	{ DIE("Out of resources"); }
	_vertexBuffer->SetNumBuffers(1);
	_vertexBuffer->SetStorage(sizeof(Vertex) * vboSize, nullptr);

	if ((_indexBuffer = Engine::GetRenderer()->CreateBuffer(BufferType::Index, false, true)) == nullptr)
	{ DIE("Out of resources"); }
	_indexBuffer->SetNumBuffers(1);
	_indexBuffer->SetStorage(sizeof(uint32_t) * iboSize, nullptr);

	BufferAttribute attrib;
	attrib.index = SHADER_POSITION_ATTRIBUTE;
	attrib.size = 3;
	attrib.type = BufferDataType::Float;
	attrib.normalize = false;
	attrib.stride = sizeof(Vertex);
	attrib.ptr = (void *)VERTEX_POSITION_OFFSET;
	_vertexBuffer->AddAttribute(attrib);

	attrib.index = SHADER_COLOR_ATTRIBUTE;
	attrib.ptr = (void *)VERTEX_COLOR_OFFSET;
	_vertexBuffer->AddAttribute(attrib);

	attrib.index = SHADER_UV_ATTRIBUTE;
	attrib.size = 2;
	attrib.ptr = (void *)VERTEX_UV_OFFSET;
	_vertexBuffer->AddAttribute(attrib);

	FT_Done_Face(face);
	free(mem);

	return ENGINE_OK;
}

void NFont::ScreenResized(int width, int height)
{
	_projection = ortho(0.f, (float)width, 0.f, (float)height);
	
	_uniformBuffer->BeginUpdate();
	_uniformBuffer->UpdateData(0, sizeof(mat4), value_ptr(_projection));
	_uniformBuffer->EndUpdate();
}

int NFont::SetPixelSize(int pixelSize)
{
	RTexture *oldTexture = _texture;
	RBuffer *oldVertexBuffer = _vertexBuffer, *oldIndexBuffer = _indexBuffer;

	_texture = nullptr;
	_vertexBuffer = _indexBuffer = nullptr;

	_pixelSize = pixelSize;

	int ret = _BuildAtlas();

	if (ret != ENGINE_OK)
	{
		delete _texture; _texture = oldTexture;
		delete _vertexBuffer; _vertexBuffer = oldVertexBuffer;
		delete _indexBuffer; _indexBuffer = oldIndexBuffer;
		return ret;
	}

	_arrayBuffer->SetVertexBuffer(_vertexBuffer);
	_arrayBuffer->SetIndexBuffer(_indexBuffer);
	_arrayBuffer->CommitBuffers();

	delete oldTexture;
	delete oldVertexBuffer;
	delete oldIndexBuffer;

	return ENGINE_OK;
}

void NFont::Draw(string text, vec2 &pos, vec3 &color) noexcept
{
	unsigned int vertexCount = (unsigned int)_vertices.size();
	int offset = Engine::GetScreenHeight() - (int)_texHeight + 4;

	for (unsigned int i = 0; i < text.length(); i++)
	{
		NFontCharacterInfo &info = _characterInfo[(int)text[i]];

		float x = pos.x + info.bearing.x;
		float y = (offset - pos.y) - (info.size.y - info.bearing.y);
		float w = (float)info.size.x;
		float h = (float)info.size.y;

		Vertex v;
		v.color = color;

		v.pos = vec3(x, y, 0.f);
		v.uv = vec2(info.offset, (float)info.size.y / (float)_texHeight);
		_vertices.push_back(v);

		v.pos = vec3(x, y + h, 0.f);
		v.uv = vec2(info.offset, 0.f);
		_vertices.push_back(v);

		v.pos = vec3(x + w, y + h, 0.f);
		v.uv = vec2(info.offset + ((float)info.size.x / (float)_texWidth), 0.f);
		_vertices.push_back(v);

		v.pos = vec3(x + w, y, 0);
		v.uv = vec2(info.offset + ((float)info.size.x / (float)_texWidth), (float)info.size.y / (float)_texHeight);
		_vertices.push_back(v);

		unsigned int indexOffset = (4 * i) + vertexCount;
		_indices.push_back(indexOffset);
		_indices.push_back(1 + indexOffset);
		_indices.push_back(2 + indexOffset);
		_indices.push_back(indexOffset);
		_indices.push_back(2 + indexOffset);
		_indices.push_back(3 + indexOffset);

		pos.x += info.advance;
	}
}

void NFont::Render()
{
	Renderer *r = Engine::GetRenderer();

	r->EnableDepthTest(false);
	r->EnableBlend(true);
	r->SetBlendFunc(BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha);

	_shader->Enable();
	_shader->GetRShader()->BindUniformBuffers();
	_shader->GetRShader()->SetTexture(U_TEXTURE0, _texture);

	_vertexBuffer->BeginUpdate();
	_indexBuffer->BeginUpdate();

	_vertexBuffer->UpdateData(0, (sizeof(Vertex) * _vertices.size()), _vertices.data());
	_indexBuffer->UpdateData(0, (sizeof(uint32_t) * _indices.size()), _indices.data());

	_vertexBuffer->EndUpdate();
	_indexBuffer->EndUpdate();

	_arrayBuffer->Bind();

	r->DrawElements(PolygonMode::Triangles, (int32_t)_indices.size(), ElementType::UnsignedInt, 0);

	_shader->Disable();

	r->EnableDepthTest(true);
	r->EnableBlend(false);

	_arrayBuffer->Unbind();

	_vertices.clear();
	_indices.clear();

	_vertexBuffer->NextBuffer();
	_indexBuffer->NextBuffer();
}

NFont::~NFont()
{
	_vertices.clear();
	_indices.clear();

	ResourceManager::UnloadResourceByName("sh_font", ResourceType::RES_SHADER);

	delete _texture;
	delete _vertexBuffer;
	delete _indexBuffer;
	delete _arrayBuffer;
	delete _uniformBuffer;
}
