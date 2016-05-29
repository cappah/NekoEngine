/* Neko Engine
 *
 * TextureFont.cpp
 * Author: Alexandru Naiman
 *
 * TextureFont class implementation 
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

#include <Engine/Vertex.h>
#include <Engine/Engine.h>
#include <Engine/TextureFont.h>
#include <Engine/ResourceManager.h>

using namespace std;
using namespace glm;

#define UV_STEP_Y		.125f
#define UV_STEP_X		.028f

map<char, Glyph> TextureFont::_charPosition
{
	{ ' ',  { 0.f,    .875f } },
	{ '!',  { .0625f, .875f } },
	{ '"',  { .125f,  .875f } },
	{ '#',  { .1875f, .875f } },
	{ '$',  { .25f,   .875f } },
	{ '%',  { .3125f, .875f } },
	{ '&',  { .375f,  .875f } },
	{ '\'', { .4375f, .875f } },
	{ '(',  { .5f,    .875f } },
	{ ')',  { .5625f, .875f } },
	{ '*',  { .625f,  .875f } },
	{ '+',  { .6875f, .875f } },
	{ ',',  { .75f,   .875f } },
	{ '-',  { .8125f, .875f } },
	{ '.',  { .875f,  .875f } },
	{ '/',  { .9375f, .875f } },

	{ '0',  { 0.f,    .750f } },
	{ '1',  { .0625f, .750f } },
	{ '2',  { .125f,  .750f } },
	{ '3',  { .1875f, .750f } },
	{ '4',  { .25f,   .750f } },
	{ '5',  { .3125f, .750f } },
	{ '6',  { .375f,  .750f } },
	{ '7',  { .4375f, .750f } },
	{ '8',  { .5f,    .750f } },
	{ '9',  { .5625f, .750f } },
	{ ':',  { .625f,  .750f } },
	{ ';',  { .6875f, .750f } },
	{ '<',  { .75f,   .750f } },
	{ '=',  { .8125f, .750f } },
	{ '>',  { .875f,  .750f } },
	{ '?',  { .9375f, .750f } },

	{ '@',  { 0.f,    .625f } },
	{ 'A',  { .0625f, .625f } },
	{ 'B',  { .125f,  .625f } },
	{ 'C',  { .1875f, .625f } },
	{ 'D',  { .25f,   .625f } },
	{ 'E',  { .3125f, .625f } },
	{ 'F',  { .375f,  .625f } },
	{ 'G',  { .4375f, .625f } },
	{ 'H',  { .5f,    .625f } },
	{ 'I',  { .5625f, .625f } },
	{ 'J',  { .625f,  .625f } },
	{ 'K',  { .6875f, .625f } },
	{ 'L',  { .75f,   .625f } },
	{ 'M',  { .8125f, .625f } },
	{ 'N',  { .875f,  .625f } },
	{ 'O',  { .9375f, .625f } },

	{ 'P',  { 0.f,    .5f } },
	{ 'Q',  { .0625f, .5f } },
	{ 'R',  { .125f,  .5f } },
	{ 'S',  { .1875f, .5f } },
	{ 'T',  { .25f,   .5f } },
	{ 'U',  { .3125f, .5f } },
	{ 'V',  { .375f,  .5f } },
	{ 'W',  { .4375f, .5f } },
	{ 'X',  { .5f,    .5f } },
	{ 'Y',  { .5625f, .5f } },
	{ 'Z',  { .625f,  .5f } },
	{ '[',  { .6875f, .5f } },
	{ '\\', { .75f,   .5f } },
	{ ']',  { .8125f, .5f } },
	{ '^',  { .875f,  .5f } },
	{ '_',  { .9375f, .5f } },

	{ '`',  { 0.f,    .375f } },
	{ 'a',  { .0625f, .375f } },
	{ 'b',  { .125f,  .375f } },
	{ 'c',  { .1875f, .375f } },
	{ 'd',  { .25f,   .375f } },
	{ 'e',  { .3125f, .375f } },
	{ 'f',  { .375f,  .375f } },
	{ 'g',  { .4375f, .375f } },
	{ 'h',  { .5f,    .375f } },
	{ 'i',  { .5625f, .375f } },
	{ 'j',  { .625f,  .375f } },
	{ 'k',  { .6875f, .375f } },
	{ 'l',  { .75f,   .375f } },
	{ 'm',  { .8125f, .375f } },
	{ 'n',  { .875f,  .375f } },
	{ 'o',  { .9375f, .375f } },

	{ 'p',  { 0.f,    .25f } },
	{ 'q',  { .0625f, .25f } },
	{ 'r',  { .125f,  .25f } },
	{ 's',  { .1875f, .25f } },
	{ 't',  { .25f,   .25f } },
	{ 'u',  { .3125f, .25f } },
	{ 'v',  { .375f,  .25f } },
	{ 'w',  { .4375f, .25f } },
	{ 'x',  { .5f,    .25f } },
	{ 'y',  { .5625f, .25f } },
	{ 'z',  { .625f,  .25f } },
	{ '{',  { .6875f, .25f } },
	{ '|',  { .75f,   .25f } },
	{ '}',  { .8125f, .25f } },
	{ '~',  { .875f,  .25f } }
};

TextureFont::TextureFont(TextureFontResource *res) noexcept
{
	_resourceInfo = res;
	_shader = nullptr;
	_texture = nullptr;

	_charWidth = .0225f;
	_charHeight = .075f;

	_loaded = false;

	_vboSize = 0;
	_iboSize = 0;

	int maxChars = (int)((1.f / _charWidth) * (1.f / _charHeight));

	int vboSize = maxChars * 4;
	int iboSize = maxChars * 6;

	if((_vertexBuffer = Engine::GetRenderer()->CreateBuffer(BufferType::Vertex, false, true)) == nullptr)
	{ DIE("Out of resources"); }
	_vertexBuffer->SetNumBuffers(1);
	_vertexBuffer->SetStorage(sizeof(Vertex) * vboSize, nullptr);

	if((_indexBuffer = Engine::GetRenderer()->CreateBuffer(BufferType::Index, false, true)) == nullptr)
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

	if((_arrayBuffer = Engine::GetRenderer()->CreateArrayBuffer()) == nullptr)
	{ DIE("Out of resources"); }
	_arrayBuffer->SetVertexBuffer(_vertexBuffer);
	_arrayBuffer->SetIndexBuffer(_indexBuffer);
	_arrayBuffer->CommitBuffers();
}

int TextureFont::Load()
{
	if (_loaded)
		return ENGINE_OK;

	_shader = (Shader *)ResourceManager::GetResource(GetResourceInfo()->shaderId, ResourceType::RES_SHADER);

	// Must force high quality texture for the font
	int texQuality = Engine::GetConfiguration().Renderer.TextureQuality;
	Engine::GetConfiguration().Renderer.TextureQuality = RENDER_TEX_Q_HIGH;

	_texture = (Texture *)ResourceManager::GetResource(GetResourceInfo()->textureId, ResourceType::RES_TEXTURE);

	Engine::GetConfiguration().Renderer.TextureQuality = texQuality;

	if (_shader == nullptr || _texture == nullptr)
		return ENGINE_FAIL;

	TextureParams params { TextureFilter::Linear, TextureFilter::Linear, TextureWrap::ClampToEdge, TextureWrap::ClampToEdge };
	_texture->SetParameters(params);

	_loaded = true;

	_shader->GetRShader()->SetTexture(U_TEXTURE0, _texture->GetRTexture());

	return ENGINE_OK;
}

void TextureFont::Draw(string text, vec2 &pos) noexcept
{
	vec3 color(1.f, 1.f, 1.f);
	Draw(text, pos, color);
}

void TextureFont::Draw(string text, vec2 &pos, vec3 &color) noexcept
{
	_GenerateVertices(text, pos, color);
}

void TextureFont::_GenerateVertices(string &text, vec2 &pos, vec3 &color) noexcept
{
	float nextX = -pos.x - .99f, nextY = -pos.y + (1.f - _charHeight);
	unsigned int vertexCount = (unsigned int)_vertices.size();

	for (unsigned int i = 0; i < text.length(); i++)
	{
		Vertex v;
		v.color = color;

		Glyph &glyph = _charPosition[text[i]];

		v.pos = vec3(nextX, nextY, 0);
		v.uv = vec2(glyph.x, glyph.y);
		_vertices.push_back(v);

		v.pos = vec3(nextX, nextY + _charHeight, 0);
		v.uv = vec2(glyph.x, glyph.y + UV_STEP_Y);
		_vertices.push_back(v);

		v.pos = vec3(nextX + _charWidth, nextY + _charHeight, 0);
		v.uv = vec2(glyph.x + UV_STEP_X, glyph.y + UV_STEP_Y);
		_vertices.push_back(v);

		v.pos = vec3(nextX + _charWidth, nextY, 0);
		v.uv = vec2(glyph.x + UV_STEP_X, glyph.y);
		_vertices.push_back(v);
		
		unsigned int indexOffset = (4 * i) + vertexCount;
		_indices.push_back(indexOffset);
		_indices.push_back(1 + indexOffset);
		_indices.push_back(2 + indexOffset);
		_indices.push_back(indexOffset);
		_indices.push_back(2 + indexOffset);
		_indices.push_back(3 + indexOffset);

		nextX += _charWidth;
		if (nextX > 1.f)
		{
			nextX = 0.f;
			nextY += _charHeight;
		}
	}
}

void TextureFont::Render()
{
	Renderer *r = Engine::GetRenderer();

	r->EnableDepthTest(false);
	r->EnableBlend(true);
	r->SetBlendFunc(BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha);

	_shader->Enable();

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

TextureFont::~TextureFont() noexcept
{
	delete _vertexBuffer;
	delete _indexBuffer;
	delete _arrayBuffer;

	ResourceManager::UnloadResource(GetResourceInfo()->textureId, ResourceType::RES_TEXTURE);
}
