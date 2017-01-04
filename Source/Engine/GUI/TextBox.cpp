/* NekoEngine
 *
 * TextBox.cpp
 * Author: Alexandru Naiman
 *
 * TextBox control
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

#include <GUI/TextBox.h>
#include <Engine/Input.h>
#include <Engine/Keycodes.h>

#define TEXTBOX_MODULE	"GUI_TextBox"

using namespace glm;
using namespace std;
using namespace std::placeholders;

int TextBox::_InitializeControl(Renderer *renderer)
{
	int ret{ Box::_InitializeControl(renderer) };
	if (ret != ENGINE_OK)
		return ret;

//	_vertices[0].color = _vertices[1].color = _vertices[2].color = _vertices[3].color = vec4(.5f, .5f, .5f, 1.f);
	
	_onKeyUp = bind(&TextBox::_KeyUp, this, _1);
	_onKeyDown = bind(&TextBox::_KeyDown, this, _1);

	return ENGINE_OK;
}

void TextBox::_Update(double deltaTime)
{
	Box::_Update(deltaTime);

	if (!_text.Length())
		return;

	//float y = ceilf(((float)_controlRect.h - (float)_font->GetCharacterHeight()) / 2.f);
	//float x = ((float)_controlRect.w - (float)_font->GetTextLength(_text)) / 2.f;
	//vec2 pos = vec2(_controlRect.x + x, _controlRect.y + y);

	//_font->Draw(_text, pos, _textColor);
}

void TextBox::_KeyUp(uint8_t key)
{
	if (key == NE_KEY_RSHIFT || key == NE_KEY_LSHIFT)
		_shift = false;
}

void TextBox::_KeyDown(uint8_t key)
{
/*	if (key == NE_KEY_RETURN)
		_text.Append('\n');
	else if ((key == NE_KEY_BKSPACE) || (key == NE_KEY_DELETE))
		_text.RemoveLast();
	else if (key == NE_KEY_RSHIFT || key == NE_KEY_LSHIFT)
		_shift = true;
	else
		_text.Append(Input::KeycodeToChar(key, _shift));*/
}
