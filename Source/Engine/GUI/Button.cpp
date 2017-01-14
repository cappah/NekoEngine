/* NekoEngine
 *
 * Button.cpp
 * Author: Alexandru Naiman
 *
 * Button control
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

#include <GUI/Button.h>
#include <Renderer/VKUtil.h>
#include <Renderer/DebugMarker.h>
#include <Renderer/RenderPassManager.h>

#define BUTTON_MODULE	"GUI_Button"

using namespace glm;

int Button::_InitializeControl()
{
	int ret{ Box::_InitializeControl() };
	if (ret != ENGINE_OK)
		return ret;

	_vertices[0].color = _vertices[1].color = _vertices[2].color = _vertices[3].color = vec4(.9f, .9f, .9f, 1.f);

	return ENGINE_OK;
}

void Button::_Update(double deltaTime)
{
	Box::_Update(deltaTime);

	if (!_text.Length())
		return;
	
	float y = ceilf(((float)_controlRect.h - (float)_font->GetCharacterHeight()) / 2.f);
	float x = ((float)_controlRect.w - (float)_font->GetTextLength(_text)) / 2.f;
	vec2 pos = vec2(_controlRect.x + x, _controlRect.y + y);
	
	_font->Draw(_text, pos, _hovered ? _hoveredTextColor : _textColor);
}