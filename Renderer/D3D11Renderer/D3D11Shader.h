/* NekoEngine
 *
 * D3D11Shader.h
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

#pragma once

#include <Renderer/RShader.h>
#include "D3D11Context.h"
#include "D3D11Texture.h"
#include "D3D11Buffer.h"

#include <vector>

#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>

typedef struct SHADER_BLOB
{
	char *data;
	size_t size;
} ShaderBlob;

typedef struct D3D11_TEXTURE_INFO
{
	unsigned int location;
	D3D11Texture *texture;
} D3D11TextureInfo;

typedef struct D3D11_UNIF_BUF
{
	D3D11Buffer *ubo;
	size_t offset;
	size_t size;
	uint32_t binding;
	int32_t index;
} D3D11UniformBuffer;

class D3D11Shader :
	public RShader
{
public:
	D3D11Shader(D3D11Context *ctx);

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void SetTexture(unsigned int location, RTexture *tex) override;

	virtual void BindUniformBuffers() override;

	virtual void VSUniformBlockBinding(int location, const char *name) override;
	virtual void FSUniformBlockBinding(int location, const char *name) override;
	virtual void VSSetUniformBuffer(int location, uint64_t offset, uint64_t size, RBuffer *buf) override;
	virtual void FSSetUniformBuffer(int location, uint64_t offset, uint64_t size, RBuffer *buf) override;

	virtual void SetSubroutines(ShaderType type, int count, const uint32_t* indices) override;

	virtual bool LoadFromSource(ShaderType type, int count, const char *source, int length) override;
	virtual bool LoadFromStageBinary(ShaderType type, const char *file) override;
	virtual bool LoadFromBinary(int count, const void *binary, size_t length) override;
	
	virtual bool Link() override;

	virtual ~D3D11Shader();

	void EnableTextures();
	void SetInputLayout();

private:
	D3D11Context *_ctx;

	ShaderBlob _vsBlob;
	ShaderBlob _psBlob;
	ShaderBlob _gsBlob;
	ShaderBlob _csBlob;
	ShaderBlob _hsBlob;
	ShaderBlob _dsBlob;

	ID3DBlob *_d3dVsBlob;
	ID3DBlob *_d3dPsBlob;
	ID3DBlob *_d3dGsBlob;
	ID3DBlob *_d3dCsBlob;
	ID3DBlob *_d3dHsBlob;
	ID3DBlob *_d3dDsBlob;

	ID3D11VertexShader *_vs;
	ID3D11PixelShader *_ps;
	ID3D11GeometryShader *_gs;
	ID3D11HullShader *_hs;
	ID3D11DomainShader *_ds;
	ID3D11ComputeShader *_cs;

	ID3D11InputLayout *_layout;

	D3D11UniformBuffer _vsBuffers[10];
	D3D11UniformBuffer _fsBuffers[10];
	uint8_t _vsNumBuffers;
	uint8_t _fsNumBuffers;
	uint8_t _nextBinding;

	std::vector<D3D11TextureInfo> _textures;
};

