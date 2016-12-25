/* NekoEngine
 *
 * RArrayBuffer.h
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

#include <Renderer/RBuffer.h>

#include <vector>

class RArrayBuffer
{
public:

	/**
	* Create a buffer object.
	* You must call SetStorage to initialize the buffer's data store before use.
	*/
	RArrayBuffer() :
		_vertexBuffer(nullptr),
		_indexBuffer(nullptr)
	{ };

	virtual void Bind() = 0;
	virtual void Unbind() = 0;

	virtual void SetVertexBuffer(RBuffer* buffer) { _vertexBuffer = buffer; }
	virtual void SetIndexBuffer(RBuffer* buffer) { _indexBuffer = buffer; }
	virtual void CommitBuffers() = 0;

	/**
	* Release resources
	*/
	virtual ~RArrayBuffer() { };

protected:
	RBuffer *_vertexBuffer, *_indexBuffer;
};