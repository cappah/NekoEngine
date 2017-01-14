/* NekoEngine
 *
 * Buffer.cpp
 * Author: Alexandru Naiman
 *
 * Vulkan buffer object wrapper
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

#include <Renderer/Buffer.h>
#include <Renderer/VKUtil.h>
#include <Renderer/Renderer.h>

using namespace std;

Buffer::Buffer()
{
	_buffer = VK_NULL_HANDLE;
	_memory = VK_NULL_HANDLE;
	_offset = _size = 0;
	_child = _persistent = _ownMemory = false;
}

Buffer::Buffer(size_t size, VkBufferUsageFlags usage, uint8_t *data, VkMemoryPropertyFlags properties)
{
	_buffer = VK_NULL_HANDLE;
	_memory = VK_NULL_HANDLE;
	_offset = _size = 0;
	_child = _persistent = _ownMemory = false;

	_CreateBuffer(size, usage, data, VK_NULL_HANDLE, 0, properties);
}

Buffer::Buffer(size_t size, VkBufferUsageFlags usage, uint8_t *data, VkDeviceMemory memory, VkDeviceSize offset, VkMemoryPropertyFlags properties)
{
	_buffer = VK_NULL_HANDLE;
	_memory = VK_NULL_HANDLE;
	_offset = _size = 0;
	_child = _persistent = _ownMemory = false;

	_CreateBuffer(size, usage, data, memory, offset, properties);
}

Buffer::Buffer(Buffer *parent, VkDeviceSize offset, VkDeviceSize size)
{
	_buffer = parent->GetHandle();
	_memory = parent->GetMemoryHandle();
	_offset = offset;
	_size = size;
	_child = true;
	_persistent = false;
	_ownMemory = false;
}

void Buffer::UpdateData(uint8_t *data, VkDeviceSize offset, VkDeviceSize size, VkCommandBuffer cmdBuffer, bool useStagingBuffer)
{
	bool submit{ false };

	if (cmdBuffer == VK_NULL_HANDLE)
	{
		submit = true;
		cmdBuffer = VKUtil::CreateOneShotCmdBuffer();
	}

	if (size < 65535 && !useStagingBuffer)
		vkCmdUpdateBuffer(cmdBuffer, _buffer, _offset + offset, size, data);
	else
	{
		Buffer *stagingBuffer{ Renderer::GetInstance()->GetStagingBuffer(size) };

		uint8_t *ptr{ stagingBuffer->Map(0, size) };
		if (!ptr)
		{ DIE("Failed to map buffer"); }

		memcpy(ptr, data, size);
		stagingBuffer->Unmap();

		VKUtil::CopyBuffer(stagingBuffer->GetHandle(), _buffer, size, 0, _offset + offset);

		Renderer::GetInstance()->FreeStagingBuffer(stagingBuffer);
	}

	if (submit) VKUtil::ExecuteOneShotCmdBuffer(cmdBuffer);
}

void Buffer::Copy(Buffer *dst, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkCommandBuffer cmdBuffer)
{
	VKUtil::CopyBuffer(_buffer, dst->GetHandle(), size, _offset + srcOffset, dst->GetParentOffset() + dstOffset, cmdBuffer);
}

void Buffer::Fill(uint32_t value, VkCommandBuffer buffer)
{
	VKUtil::FillBuffer(_buffer, _offset, _size, value);
}

uint8_t *Buffer::Map(VkDeviceSize offset, VkDeviceSize size)
{
	if (!size) size = _size;

	uint8_t *ptr{ nullptr };
	if (vkMapMemory(VKUtil::GetDevice(), _memory, _offset + offset, size, 0, (void **)&ptr) != VK_SUCCESS)
		return nullptr;

	return ptr;
}

void Buffer::Unmap()
{
	vkUnmapMemory(VKUtil::GetDevice(), _memory);
}

void Buffer::_CreateBuffer(size_t size, VkBufferUsageFlags usage, uint8_t *data, VkDeviceMemory memory, VkDeviceSize offset, VkMemoryPropertyFlags properties)
{
	_buffer = VK_NULL_HANDLE;
	_memory = memory;
	_offset = offset;
	_size = size;

	if (_memory == VK_NULL_HANDLE)
		_ownMemory = true;

	if (!VKUtil::CreateBuffer(_buffer, _memory, _size, usage, properties, offset))
	{ DIE("Failed to create buffer"); }

	if (data)
	{
		Buffer *stagingBuffer{ Renderer::GetInstance()->GetStagingBuffer(size) };
	
		uint8_t *ptr{ stagingBuffer->Map(0, size) };
		if (!ptr)
		{ DIE("Failed to map buffer"); }

		memcpy(ptr, data, size);
		stagingBuffer->Unmap();

		VKUtil::CopyBuffer(stagingBuffer->GetHandle(), _buffer, size, 0, _offset + offset);

		Renderer::GetInstance()->FreeStagingBuffer(stagingBuffer);
	}
}

Buffer::~Buffer()
{
	if (_child)
		return;

	if(_ownMemory)
		vkFreeMemory(VKUtil::GetDevice(), _memory, VKUtil::GetAllocator());

	vkDestroyBuffer(VKUtil::GetDevice(), _buffer, VKUtil::GetAllocator());
}