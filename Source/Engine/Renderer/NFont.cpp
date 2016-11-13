/* NekoEngine
 *
 * NFont.h
 * Author: Alexandru Naiman
 *
 * TrueType font renderer
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

#include <algorithm>

#include <System/VFS/VFS.h>
#include <Renderer/NFont.h>
#include <Renderer/Debug.h>
#include <Renderer/VKUtil.h>
#include <Renderer/Renderer.h>
#include <Renderer/PipelineManager.h>
#include <Renderer/RenderPassManager.h>

#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#define FONT_MODULE		"FontRenderer"

using namespace std;
using namespace glm;

static FT_Library _ft = nullptr;

NFont::NFont(FontResource *res)
{
	_resourceInfo = res;

	_cmdBuffer = VK_NULL_HANDLE;
	_descriptorPool = VK_NULL_HANDLE;
	_image = VK_NULL_HANDLE;
	_imageMemory = VK_NULL_HANDLE;
	_view = VK_NULL_HANDLE;

	_texWidth = _texHeight = 0;
	_bufferSize = 0;
	_pixelSize = 20;

	_buffer = _stagingBuffer = nullptr;
}

int NFont::Load()
{
	FT_Init_FreeType(&_ft);

	int ret = _BuildAtlas();

	FT_Done_FreeType(_ft);

	if (ret != ENGINE_OK)
		return ret;

	if ((ret = _CreateDescriptorSet()) != ENGINE_OK)
		return ret;

	if ((ret = _BuildCommandBuffer()) != ENGINE_OK)
		return ret;

	return ENGINE_OK;
}

void NFont::UpdateData(VkCommandBuffer cmdBuffer)
{
	uint8_t *data = _stagingBuffer->Map(0, _bufferSize);
	if(!data)
	{ DIE("Failed to map staging buffer"); }

	_drawCommand.indexCount = (uint32_t)_indices.Count();
	_drawCommand.instanceCount = 1;
	_drawCommand.firstIndex = 0;
	_drawCommand.firstInstance = 0;
	_drawCommand.vertexOffset = 0;

	memcpy(data, &_drawCommand, sizeof(VkDrawIndexedIndirectCommand));
	memcpy(data + _vboOffset, *_vertices, _vertices.Count() * sizeof(GUIVertex));
	memcpy(data + _iboOffset, *_indices, _indices.Count() * sizeof(uint32_t));

	_stagingBuffer->Unmap();

	VKUtil::CopyBuffer(_stagingBuffer->GetHandle(), _buffer->GetHandle(), _bufferSize, 0, _buffer->GetParentOffset(), cmdBuffer);

	_vertices.Clear();
	_indices.Clear();
}

void NFont::Draw(NString text, glm::vec2 &pos, glm::vec3 &color) noexcept
{
	unsigned int vertexCount = (unsigned int)_vertices.Count();
	uint32_t offset = Engine::GetConfiguration().Engine.ScreenHeight - _texHeight + 4;

	for (unsigned int i = 0; i < text.Length(); i++)
	{
		CharacterInfo &info = _characterInfo[(int)text[i]];

		float x = pos.x + info.bearing.x;
		float y = (offset - pos.y) - (info.size.y - info.bearing.y);
		float w = (float)info.size.x;
		float h = (float)info.size.y;

		GUIVertex v;
		v.color = vec4(color, 1.0);

		v.posAndUV = vec4(x, y, info.offset, (float)info.size.y / (float)_texHeight);
		_vertices.Add(v);

		v.posAndUV = vec4(x, y + h, info.offset, 0.f);
		_vertices.Add(v);

		v.posAndUV = vec4(x + w, y + h, info.offset + ((float)info.size.x / (float)_texWidth), 0.f);
		_vertices.Add(v);

		v.posAndUV = vec4(x + w, y, info.offset + ((float)info.size.x / (float)_texWidth), (float)info.size.y / (float)_texHeight);
		_vertices.Add(v);

		unsigned int indexOffset = (4 * i) + vertexCount;
		_indices.Add(indexOffset);
		_indices.Add(1 + indexOffset);
		_indices.Add(2 + indexOffset);
		_indices.Add(indexOffset);
		_indices.Add(2 + indexOffset);
		_indices.Add(3 + indexOffset);

		pos.x += info.advance;
	}
}

void NFont::AddCommandBuffer()
{
	Renderer::GetInstance()->AddGUICommandBuffer(_cmdBuffer);
}

int NFont::_BuildAtlas()
{
	FT_Face face;
	FT_GlyphSlot glyph;
	VFSFile *file = nullptr;
	size_t size = 0;
	uint8_t *mem = nullptr;
	uint32_t x = 0, vboSize = 0, iboSize = 0, maxChars = 0;

	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

	if ((file = VFS::Open(GetResourceInfo()->filePath)) == nullptr)
	{
		Logger::Log(FONT_MODULE, LOG_CRITICAL, "Failed to open file for font id %d, file name [%s].", GetResourceInfo()->id, *GetResourceInfo()->filePath);
		return ENGINE_FAIL;
	}

	if (file->Seek(0, SEEK_END) == ENGINE_FAIL)
	{
		file->Close();
		Logger::Log(FONT_MODULE, LOG_CRITICAL, "Seek failed for file [%].", *GetResourceInfo()->filePath);
		return ENGINE_FAIL;
	}

	size = file->Tell();

	if (file->Seek(0, SEEK_SET) == ENGINE_FAIL)
	{
		file->Close();
		Logger::Log(FONT_MODULE, LOG_CRITICAL, "Seek failed for file [%].", *GetResourceInfo()->filePath);
		return ENGINE_FAIL;
	}

	mem = (uint8_t*)calloc((size_t)size, sizeof(uint8_t));
	if (file->Read(mem, sizeof(uint8_t), size) == 0)
	{
		file->Close();
		free(mem);
		Logger::Log(FONT_MODULE, LOG_CRITICAL, "Failed to read file [%].", *GetResourceInfo()->filePath);
		return ENGINE_FAIL;
	}

	if (FT_New_Memory_Face(_ft, mem, (FT_Long)size, 0, &face))
	{
		file->Close();
		free(mem);
		Logger::Log(FONT_MODULE, LOG_CRITICAL, "Failed to load font face for id %s.", GetResourceInfo()->name.c_str());
		return ENGINE_FAIL;
	}

	FT_Set_Pixel_Sizes(face, 0, _pixelSize);

	glyph = face->glyph;

	for (int i = 0; i < FONT_NUM_CHARS; ++i)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
			continue;

		_texWidth += glyph->bitmap.width;
		_texHeight = std::max(_texHeight, (uint32_t)glyph->bitmap.rows);
	}

	VkDeviceSize buffSize = _texWidth * _texHeight;

	VKUtil::CreateBuffer(stagingBuffer, stagingBufferMemory, buffSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	uint8_t *data;
	vkMapMemory(VKUtil::GetDevice(), stagingBufferMemory, 0, buffSize, 0, (void **)&data);

	for (int i = 0; i < FONT_NUM_CHARS; ++i)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER)) continue;

		for (uint32_t j = 0; j < glyph->bitmap.rows; ++j)
		{
			uint32_t offset = x + _texWidth * j;
			uint32_t size = glyph->bitmap.width;
			uint32_t dataOffset = size * j;

			memcpy(data + offset, glyph->bitmap.buffer + dataOffset, size);
		}

		_characterInfo[i].size = ivec2(glyph->bitmap.width, glyph->bitmap.rows);
		_characterInfo[i].bearing = ivec2(glyph->bitmap_left, glyph->bitmap_top);
		_characterInfo[i].advance = (unsigned int)glyph->advance.x >> 6;
		_characterInfo[i].offset = (float)x / _texWidth;

		x += glyph->bitmap.width;
	}

	vkUnmapMemory(VKUtil::GetDevice(), stagingBufferMemory);

	VKUtil::CreateImage(_image, _imageMemory, _texWidth, _texHeight, 1,
		VK_MEMORY_HEAP_DEVICE_LOCAL_BIT, VK_FORMAT_R8_UNORM, VK_IMAGE_TYPE_2D,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_TILING_OPTIMAL);

	VKUtil::TransitionImageLayout(_image, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	VKUtil::CopyBufferToImage(stagingBuffer, _image, _texWidth, _texHeight);
	VKUtil::TransitionImageLayout(_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VKUtil::CreateImageView(_view, _image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8_UNORM);

	vkDestroyBuffer(VKUtil::GetDevice(), stagingBuffer, VKUtil::GetAllocator());
	vkFreeMemory(VKUtil::GetDevice(), stagingBufferMemory, VKUtil::GetAllocator());

	maxChars = (int)((1.f / (((float)_texWidth / (float)FONT_NUM_CHARS) * (1.f / Engine::GetScreenWidth()))) * (1.f / (_texHeight * (1.f / Engine::GetScreenHeight()))));

	_vertices.Resize(maxChars * 4);
	_indices.Resize(maxChars * 6);

	vboSize = maxChars * 4 * sizeof(GUIVertex);
	iboSize = maxChars * 6 * sizeof(uint16_t);

	_vboOffset = sizeof(VkDrawIndexedIndirectCommand);
	_iboOffset = _vboOffset + vboSize;
	_bufferSize = _iboOffset + iboSize;

	_buffer = new Buffer(_bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
		VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	_stagingBuffer = new Buffer(_bufferSize, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, nullptr, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	data = (uint8_t *)_stagingBuffer->Map(0, buffSize);

	_drawCommand.indexCount = (uint32_t)_indices.Count();
	_drawCommand.instanceCount = 1;
	_drawCommand.firstIndex = 0;
	_drawCommand.firstInstance = 0;
	_drawCommand.vertexOffset = 0;

	memcpy(data, &_drawCommand, sizeof(VkDrawIndexedIndirectCommand));
	memcpy(data + _vboOffset, *_vertices, vboSize);
	memcpy(data + _iboOffset, *_indices, iboSize);

	_stagingBuffer->Unmap();

	VKUtil::CopyBuffer(_stagingBuffer->GetHandle(), _buffer->GetHandle(), _bufferSize);

	FT_Done_Face(face);

	DBG_SET_OBJECT_NAME((uint64_t)_buffer->GetHandle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, *NString::StringWithFormat(40, "Font %s buffer", _resourceInfo->name.c_str()));
	DBG_SET_OBJECT_NAME((uint64_t)_stagingBuffer->GetHandle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, *NString::StringWithFormat(40, "Font %s staging buffer", _resourceInfo->name.c_str()));
	DBG_SET_OBJECT_NAME((uint64_t)_image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, *NString::StringWithFormat(40, "Font %s image", _resourceInfo->name.c_str()));

	free(mem);

	return ENGINE_OK;
}

int NFont::_CreateDescriptorSet()
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;

	if (vkCreateDescriptorPool(VKUtil::GetDevice(), &poolInfo, VKUtil::GetAllocator(), &_descriptorPool) != VK_SUCCESS)
	{
		Logger::Log(FONT_MODULE, LOG_CRITICAL, "Failed to create descriptor pool");
		return ENGINE_DESCRIPTOR_POOL_CREATE_FAIL;
	}

	VkDescriptorSetLayout layout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_OneSampler);

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, &_descriptorSet) != VK_SUCCESS)
	{
		Logger::Log(FONT_MODULE, LOG_CRITICAL, "Failed to allocate descriptor set");
		return ENGINE_DESCRIPTOR_SET_CREATE_FAIL;
	}

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = _view;
	imageInfo.sampler = GUI::GetSampler();

	VkWriteDescriptorSet textureDescriptorWrite = {};
	textureDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	textureDescriptorWrite.dstSet = _descriptorSet;
	textureDescriptorWrite.dstBinding = 0;
	textureDescriptorWrite.dstArrayElement = 0;
	textureDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureDescriptorWrite.descriptorCount = 1;
	textureDescriptorWrite.pBufferInfo = nullptr;
	textureDescriptorWrite.pImageInfo = &imageInfo;
	textureDescriptorWrite.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(VKUtil::GetDevice(), 1, &textureDescriptorWrite, 0, nullptr);

	return ENGINE_OK;
}

int NFont::_BuildCommandBuffer()
{
	if (_cmdBuffer != VK_NULL_HANDLE)
		VKUtil::FreeCommandBuffer(_cmdBuffer);

	if ((_cmdBuffer = VKUtil::CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY)) == VK_NULL_HANDLE)
		return ENGINE_CMDBUFFER_CREATE_FAIL;

	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;
	inheritanceInfo.renderPass = RenderPassManager::GetRenderPass(RP_GUI);
	inheritanceInfo.subpass = 0;
	inheritanceInfo.framebuffer = Renderer::GetInstance()->GetGUIFramebuffer();

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = &inheritanceInfo;

	if (vkBeginCommandBuffer(_cmdBuffer, &beginInfo) != VK_SUCCESS)
	{
		Logger::Log(FONT_MODULE, LOG_CRITICAL, "vkBeginCommandBuffer call failed");
		return ENGINE_CMDBUFFER_BEGIN_FAIL;
	}

	DBG_MARKER_INSERT(_cmdBuffer, "Font", vec4(1.0, 0.5, 1.0, 1.0));

	VkDeviceSize offset = _vboOffset;
	vkCmdBindVertexBuffers(_cmdBuffer, 0, 1, &_buffer->GetHandle(), &offset);
	vkCmdBindIndexBuffer(_cmdBuffer, _buffer->GetHandle(), _iboOffset, VK_INDEX_TYPE_UINT32);

	vkCmdBindPipeline(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipeline(PIPE_Font));
	GUI::BindDescriptorSet(_cmdBuffer);
	vkCmdBindDescriptorSets(_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_GUI), 1, 1, &_descriptorSet, 0, nullptr);

	vkCmdDrawIndexedIndirect(_cmdBuffer, _buffer->GetHandle(), 0, 1, 0);

	if (vkEndCommandBuffer(_cmdBuffer) != VK_SUCCESS)
	{
		Logger::Log(FONT_MODULE, LOG_CRITICAL, "vkEndCommandBuffer call failed");
		return ENGINE_CMDBUFFER_RECORD_FAIL;
	}

	return ENGINE_OK;
}

NFont::~NFont()
{
	VKUtil::FreeCommandBuffer(_cmdBuffer);

	if (_view != VK_NULL_HANDLE)
		vkDestroyImageView(VKUtil::GetDevice(), _view, VKUtil::GetAllocator());

	if (_imageMemory != VK_NULL_HANDLE)
		vkFreeMemory(VKUtil::GetDevice(), _imageMemory, VKUtil::GetAllocator());

	if (_image != VK_NULL_HANDLE)
		vkDestroyImage(VKUtil::GetDevice(), _image, VKUtil::GetAllocator());

	if (_descriptorPool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(VKUtil::GetDevice(), _descriptorPool, VKUtil::GetAllocator());

	delete _buffer;
	delete _stagingBuffer;
}
