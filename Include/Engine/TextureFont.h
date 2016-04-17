/* Neko Engine
 *
 * TextureFont.h
 * Author: Alexandru Naiman
 *
 * Font renderer using a texture
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

#pragma once

#include <glm/glm.hpp>

#include <Engine/Vertex.h>
#include <Engine/Shader.h>
#include <Engine/Texture.h>
#include <Resource/Resource.h>
#include <Resource/TextureFontResource.h>

#include <map>
#include <string>
#include <vector>

struct Glyph
{
	float x;
	float y;
};

class TextureFont : public Resource
{
public:
	ENGINE_API TextureFont(TextureFontResource* res) noexcept;

	ENGINE_API TextureFontResource* GetResourceInfo() noexcept { return (TextureFontResource*)_resourceInfo; }

	ENGINE_API float GetCharacterHeight() noexcept { return _charHeight; }
	ENGINE_API float GetCharacterWidth() noexcept { return _charWidth; }

	ENGINE_API virtual int Load() override;
	ENGINE_API void Draw(std::string text, glm::vec2& pos) noexcept;
	ENGINE_API void Draw(std::string text, glm::vec2& pos, glm::vec3& color) noexcept;

	ENGINE_API void Render();

	ENGINE_API virtual ~TextureFont() noexcept;

private:
	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;
	static std::map<char, Glyph> _charPosition;
	float _charWidth, _charHeight;
	Shader* _shader;
	Texture* _texture;
	bool _loaded;
	size_t _vboSize;
	size_t _iboSize;
	RBuffer* _vertexBuffer;
	RBuffer* _indexBuffer;
	RArrayBuffer* _arrayBuffer;

	void _GenerateVertices(std::string& text, glm::vec2& pos, glm::vec3& color) noexcept;
};

