/* NekoEngine
 *
 * NFont.h
 * Author: Alexandru Naiman
 *
 * FreeType font renderer
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

#pragma once

#include <glm/glm.hpp>

#include <Engine/Engine.h>
#include <Resource/Resource.h>
#include <Resource/FontResource.h>

#define NFONT_START_CHAR	32
#define NFONT_NUM_CHARS		128

typedef struct NFONT_CHARACTER_INFO
{
	glm::ivec2 size;
	glm::ivec2 bearing;
	unsigned int advance;
	float offset;
} NFontCharacterInfo;

class NFont : public Resource
{
public:
	ENGINE_API NFont(FontResource *res);

	ENGINE_API FontResource* GetResourceInfo() noexcept { return (FontResource*)_resourceInfo; }
	ENGINE_API int GetCharacterHeight() noexcept { return _texHeight; }

	ENGINE_API int SetPixelSize(int size);

	ENGINE_API virtual int Load() override;
	
	ENGINE_API void ScreenResized(int width, int height);

	ENGINE_API void Draw(std::string text, glm::vec2& pos) noexcept { glm::vec3 white(1.f, 1.f, 1.f); Draw(text, pos, white); }
	ENGINE_API void Draw(std::string text, glm::vec2& pos, glm::vec3& color) noexcept;
	ENGINE_API void Render();

	ENGINE_API virtual ~NFont();

private:
	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;
	glm::mat4 _projection;
	NFontCharacterInfo _characterInfo[NFONT_NUM_CHARS];
	size_t _vboSize;
	size_t _iboSize;
	uint32_t _texWidth;
	uint32_t _texHeight;
	RTexture *_texture;
	RBuffer *_vertexBuffer;
	RBuffer *_indexBuffer;
	RBuffer *_uniformBuffer;
	RArrayBuffer *_arrayBuffer;
	Shader *_shader;
	int _pixelSize;

	int _BuildAtlas();
};
