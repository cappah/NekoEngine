/* NekoEngine
 *
 * RBuffer.h
 * Author: Alexandru Naiman
 *
 * Rendering API abstraction
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

#include <stdint.h>
#include <stddef.h>
#include <vector>
#include <string>

#define R_VERTEX_BUFFER	0
#define R_INDEX_BUFFER	1

enum class BufferDataType
{
	Byte = 0,
	UnsignedByte = 1,
	Short,
	UnsignedShort,
	Int,
	UnsignedInt,
	HalfFloat,
	Float,
	Double,
	Fixed
};

typedef struct BUFFER_ATTRIB
{
	uint32_t index;
	std::string name;
	int32_t size;
	BufferDataType type;
	bool normalize;
	size_t stride;
	void *ptr;
} BufferAttribute;

enum class BufferType : uint8_t
{
	Vertex = 0,
	Index = 1,
	Uniform = 2
};

class RBuffer
{
public:
	
	/**
	 * Create a buffer object.
	 * You must call SetStorage to initialize the buffer's data store before use.
	 */
	RBuffer(BufferType type) :
		_type(type),
		_data(nullptr),
		_size(0),
		_persistent(false),
		_dynamic(false)
	{ };

	virtual void Bind(int location) = 0;
	virtual void Unbind() = 0;

	/**
	 * Get a pointer to the buffer's data store
	 * If the buffer is not dynamic & persistent, this function will return nullptr.
	 */
	virtual uint8_t* GetData() { return _data; }

	virtual int GetCurrentBuffer() = 0;

	virtual uint64_t GetOffset() = 0;
	
	/**
	 * True if the buffer's storage is dynamic.
	 */
	virtual bool IsDynamic() { return _dynamic; }

	/**
	 * True if the buffer's storage is persistent.
	 */
	virtual bool IsPersistent() { return _persistent; }
	
	/**
	 * Create and initialize the buffer's data store
	 */
	virtual void SetStorage(size_t size, void* data) = 0;
	
	/**
	 * Update a subset of the buffer's data store
	 * If the buffer is not dynamic, this function will have no effect.
	 */
	virtual void UpdateData(size_t offset, size_t size, void* data) = 0;

	/**
	 * Add a vertex attribute data
	 */
	virtual void AddAttribute(BufferAttribute &attrib) { _attributes.push_back(attrib); }

	virtual void SetNumBuffers(int n) = 0;

	/**
	 * Wait until the GPU has finished with the buffer
	 */
	virtual void BeginUpdate() = 0;

	/**
	 * Lock the buffer
	 */
	virtual void EndUpdate() = 0;

	virtual void NextBuffer() = 0;

	/**
	 * Release resources
	 */
	virtual ~RBuffer() { };

protected:
	BufferType _type;
	uint8_t *_data;
	size_t _size;
	bool _persistent, _dynamic;
	std::vector<BufferAttribute> _attributes;
};