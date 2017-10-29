/* NekoEngine
 *
 * NFont.h
 * Author: Alexandru Naiman
 *
 * TrueType font loader
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

#pragma once

#include <Engine/Engine.h>
#include <Renderer/Buffer.h>
#include <Runtime/Runtime.h>
#include <Resource/Resource.h>
#include <Resource/FontResource.h>

#define FONT_START_CHAR		32
#define FONT_NUM_CHARS		128

typedef struct CHARACTER_INFO
{
	glm::ivec2 size;
	glm::ivec2 bearing;
	unsigned int advance;
	float offset;
} CharacterInfo;

class NFont : public Resource
{
public:
	ENGINE_API NFont(FontResource *res);

	ENGINE_API FontResource* GetResourceInfo() noexcept { return (FontResource*)_resourceInfo; }
	ENGINE_API uint32_t GetCharacterHeight() { return _texHeight; }
	ENGINE_API uint32_t GetTextLength(const char *text);
	ENGINE_API uint32_t GetTextLength(const NString &text);

	ENGINE_API int SetPixelSize(int pixelSize);

	ENGINE_API virtual int Load() override;

	ENGINE_API void UpdateData(VkCommandBuffer cmdBuffer);
	ENGINE_API void Draw(NString text, glm::vec2& pos) noexcept { glm::vec3 white(1.f, 1.f, 1.f); Draw(text, pos, white); }
	ENGINE_API void Draw(NString text, glm::vec2& pos, glm::vec3& color) noexcept;

	ENGINE_API virtual ~NFont();

#ifdef ENGINE_INTERNAL
	VkCommandBuffer GetCommandBuffer() { return _cmdBuffer; }
	void AddCommandBuffer();
#endif

private:
	NArray<GUIVertex> _vertices;
	NArray<uint32_t> _indices;
	CharacterInfo _characterInfo[FONT_NUM_CHARS];
	Buffer *_buffer, *_stagingBuffer;
	uint32_t _texWidth, _texHeight;
	int _pixelSize;

#ifdef ENGINE_INTERNAL
	VkImage _image;
	VkImageView _view;
	VkDeviceMemory _imageMemory;
	VkCommandBuffer _cmdBuffer, _oldCmdBuffer;
	VkDescriptorSet _descriptorSet;
	VkDescriptorPool _descriptorPool;
	VkDeviceSize _vboOffset, _iboOffset, _bufferSize;
	VkDrawIndexedIndirectCommand _drawCommand;

	int _BuildAtlas();
	int _CreateDescriptorSet();
	void _UpdateDescriptorSet();
	int _BuildCommandBuffer();
#endif
};

#if defined(_MSC_VER)
template class ENGINE_API NArray<NFont *>;
template class ENGINE_API NArray<CharacterInfo>;
#endif