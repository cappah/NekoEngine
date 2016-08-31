/* NekoEngine
 *
 * D3D11Buffer.h
 * Author: Alexandru Naiman
 *
 * DirectX 11 Renderer Implementation
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

#include "D3D11Buffer.h"
#include <vector>

using namespace std;

UINT D3D11_BufferTargets[3]
{
	D3D11_BIND_VERTEX_BUFFER,
	D3D11_BIND_INDEX_BUFFER,
	D3D11_BIND_CONSTANT_BUFFER
};

D3D11Buffer::D3D11Buffer(D3D11Context *context, BufferType type, bool dynamic, bool persistent)
	: RBuffer(type)
{
	_ctx = context;
	_dynamic = dynamic || persistent;
	_persistent = persistent;
}

void D3D11Buffer::Bind(int location)
{
	if (_type == BufferType::Vertex)
	{
		UINT stride = (UINT)_attributes[0].stride;
		UINT offset = 0;
		_ctx->deviceContext->IASetVertexBuffers(0, 1, &_buffer, (UINT *)&stride, &offset);
	}
	else if(_type == BufferType::Index)
		_ctx->deviceContext->IASetIndexBuffer(_buffer, DXGI_FORMAT_R32_UINT, 0);
}

void D3D11Buffer::Unbind()
{
	if (_type == BufferType::Vertex)
		_ctx->deviceContext->IASetVertexBuffers(0, 0, nullptr, 0, 0);
	else if (_type == BufferType::Index)
		_ctx->deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
}

void D3D11Buffer::SetStorage(size_t size, void* data)
{
	D3D11_BUFFER_DESC bd;
	D3D11_SUBRESOURCE_DATA srd;

	ZeroMemory(&bd, sizeof(bd));
	ZeroMemory(&srd, sizeof(srd));

	bd.Usage = _dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	bd.ByteWidth = (UINT)size;
	bd.BindFlags = D3D11_BufferTargets[(int)_type];
	bd.CPUAccessFlags = _dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	srd.pSysMem = data;

	if (bd.ByteWidth < 16)
		bd.ByteWidth = 16;

	HRESULT hr = _ctx->device->CreateBuffer(&bd, data ? &srd : nullptr, &_buffer);
	if (FAILED(hr))
	{
		OutputDebugStringA("fk");
	}
}

void D3D11Buffer::UpdateData(size_t offset, size_t size, void* data)
{
	D3D11_MAPPED_SUBRESOURCE ms;
	HRESULT hr = _ctx->deviceContext->Map(_buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	if (FAILED(hr))
	{
		int fkme = GetLastError();
		return;
	}
	memcpy((uint8_t *)ms.pData + offset, data, size);
	_ctx->deviceContext->Unmap(_buffer, NULL);
}

// D3D11 does not support persistent buffers
int D3D11Buffer::GetCurrentBuffer() { return 0; }
uint64_t D3D11Buffer::GetOffset() { return 0; }
void D3D11Buffer::SetNumBuffers(int n) { }
void D3D11Buffer::BeginUpdate() { }
void D3D11Buffer::EndUpdate() { }
void D3D11Buffer::NextBuffer() { }

D3D11Buffer::~D3D11Buffer()
{
	if(_buffer)	_buffer->Release();
}
