/* NekoEngine
 *
 * Box.h
 * Author: Alexandru Naiman
 *
 * Box control
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

#include <GUI/Control.h>
#include <Renderer/Texture.h>

class Box : public Control
{
public:
	ENGINE_API Box(int x = 0, int y = 0, int width = 75, int height = 24) : Control(x, y, width, height) { memset(_vertices, 0x0, sizeof(GUIVertex) * 4); _tex = nullptr; _color = glm::vec4(.3f, .3f, .3f, 1.f); }
	ENGINE_API virtual void SetPosition(int x, int y) override { Control::SetPosition(x, y); _UpdateVertices(); }
	ENGINE_API virtual void SetPosition(Point pt) override { Control::SetPosition(pt); _UpdateVertices(); }
	ENGINE_API virtual void SetSize(int width, int height) override { Control::SetSize(width, height); _UpdateVertices(); }
	ENGINE_API virtual void SetSize(Point pt) override { Control::SetSize(pt); _UpdateVertices(); }
	ENGINE_API virtual void SetColor(glm::vec4 &color) override { Control::SetColor(color); _UpdateVertices(); }
	ENGINE_API virtual void SetTexture(Texture *texture);
	ENGINE_API virtual ~Box();

protected:
	GUIVertex _vertices[4];
	Texture *_tex;

	void _UpdateVertices();

	virtual int _InitializeControl() override;
	virtual int _CreateDescriptorSet() override;
	virtual int _BuildCommandBuffer() override;
	virtual void _Update(double deltaTime) override;
	virtual void _UpdateData(void *commandBuffer) override;
};
