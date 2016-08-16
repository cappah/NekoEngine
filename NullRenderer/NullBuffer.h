/* NekoEngine
 *
 * NullBuffer.h
 * Author: Alexandru Naiman
 *
 * Null Renderer Implementation
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

class NullBuffer :
	public RBuffer
{
public:
	NullBuffer(BufferType type) : RBuffer(type) { }
	NullBuffer(BufferType type, bool dynamic, bool persistent) : RBuffer(type) { }

	virtual void Bind(int location) override { }
	virtual void Unbind() override { }

	virtual uint8_t* GetData() override { return nullptr; }
	virtual int GetCurrentBuffer() override { return 0; }
	virtual uint64_t GetOffset() override { return 0; }

	virtual void SetStorage(size_t size, void* data) override { }
	virtual void UpdateData(size_t offset, size_t size, void* data) override { }

	virtual void SetNumBuffers(int n) override { }

	virtual void BeginUpdate() override { }
	virtual void EndUpdate() override { }
	virtual void NextBuffer() override { }

	void BindUniform(int index, uint64_t offset, uint64_t size) { }

	virtual ~NullBuffer() { }
};