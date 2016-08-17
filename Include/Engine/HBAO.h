/* NekoEngine
 *
 * HBAO.h
 * Author: Alexandru Naiman
 *
 * HBAO+
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

#include <Engine/Engine.h>
#include <Engine/Shader.h>

class HBAO
{
public:
	HBAO(int width, int height);

	RTexture* GetTexture() noexcept { return _texture; }

	void SetViewport(int top, int left, int width, int height, float zNear, float zFar)
	{
		_args.viewport.top = top;
		_args.viewport.left = left;
		_args.viewport.width = width;
		_args.viewport.height = height;
		_args.viewport.zNear = zNear;
		_args.viewport.zFar = zFar;
	}
	void SetProjection(float *projection) { memcpy(_args.projection, projection, sizeof(float) * 16); }
	void SetWorldToView(float *worldToView) { memcpy(_args.worldToView, worldToView, sizeof(float) * 16); }

	bool Initialize() noexcept;
	void Render() noexcept;
	void Resize(int width, int height) noexcept;

	virtual ~HBAO();

private:
	RFramebuffer* _fbo;
	RTexture* _texture;
	RHBAOArgs _args;
	int _fboWidth, _fboHeight;
};