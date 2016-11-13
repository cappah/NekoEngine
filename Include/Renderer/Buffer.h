/* NekoEngine
 *
 * Buffer.h
 * Author: Alexandru Naiman
 *
 * Vulkan buffer wrapper
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

#include <vulkan/vulkan.h>

class Buffer
{
public:
	Buffer();

	virtual ~Buffer();

	Buffer(size_t size, VkBufferUsageFlags usage, uint8_t *data = nullptr, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	Buffer(size_t size, VkBufferUsageFlags usage, uint8_t *data = nullptr, VkDeviceMemory memory = VK_NULL_HANDLE, VkDeviceSize offset = 0, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	Buffer(Buffer *parent, VkDeviceSize offset, VkDeviceSize size);

	VkBuffer &GetHandle() { return _buffer; }
	VkDeviceMemory &GetMemoryHandle() { return _memory; }
	VkDeviceSize GetParentOffset() { return _offset; }
	VkDeviceSize GetSize() { return _size; }

	uint8_t *Map(VkDeviceSize offset = 0, VkDeviceSize size = 0);
	void Unmap();

	/*
	 * If the buffer is under 65536 bytes (limit as of Vulkan 1.0.26), it will be updated with vkCmdUpdateBuffer.
	 * Otherwise, the data will be copied to a staging buffer.
	 * The useStagingBuffer parameter can be used to force the use of a staging buffer.
	 */
	void UpdateData(uint8_t *data, VkDeviceSize offset, VkDeviceSize size, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE, bool useStagingBuffer = false);

	void Copy(Buffer *dst, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);

	//void InitUpdateBuffer(size_t size);

private:
	VkBuffer _buffer;
	VkDeviceMemory _memory;
	VkDeviceSize _offset, _size;
	bool _persistent, _ownMemory, _child;

	void _CreateBuffer(size_t size, VkBufferUsageFlags usage, uint8_t *data, VkDeviceMemory memory, VkDeviceSize offset, VkMemoryPropertyFlags properties);
};