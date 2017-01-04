/* NekoEngine
 *
 * GUIManager.cpp
 * Author: Alexandru Naiman
 *
 * GUI system
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

#include <GUI/GUIManager.h>
#include <GUI/Control.h>
#include <Engine/Engine.h>
#include <Engine/NFont.h>
#include <Engine/ResourceManager.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define FONT_BUFF		8192

using namespace std;
using namespace glm;

RBuffer *GUIManager::_uniformBuffer{ nullptr }, *GUIManager::_indexBuffer{ nullptr };
GUIDrawInfo GUIManager::_drawInfo;
mat4 GUIManager::_projection;
vector<class NFont *> GUIManager::_fonts;
vector<class Control*> GUIManager::_controls;

static Control *_focusedControl{ nullptr };

int GUIManager::Initialize()
{
	uint32_t indices[6]{ 0, 1, 2, 0, 2, 3 };
	_drawInfo.renderer = Engine::GetRenderer();

	_projection = ortho(0.f, 512.f, 0.f, 512.f);
	_uniformBuffer = _drawInfo.renderer->CreateBuffer(BufferType::Uniform, true, false);
	_uniformBuffer->SetNumBuffers(1);
	_uniformBuffer->SetStorage(sizeof(mat4), value_ptr(_projection));

	_indexBuffer = _drawInfo.renderer->CreateBuffer(BufferType::Index, false, false);
	_indexBuffer->SetStorage(sizeof(indices), indices);

	//_drawInfo.shader = _drawInfo.renderer->CreateShader();
	//_drawInfo.shader->LoadFromSource(ShaderType::Vertex, 1, vs, (int)strlen(vs));
	//_drawInfo.shader->LoadFromSource(ShaderType::Fragment, 1, fs, (int)strlen(fs));
	//_drawInfo.shader->Link();

//	_drawInfo.shader->VSUniformBlockBinding(0, "shader_data");
//	_drawInfo.shader->VSSetUniformBuffer(0, 0, sizeof(mat4), _uniformBuffer);

	_drawInfo.guiFont = (NFont*)ResourceManager::GetResourceByName("fnt_system", ResourceType::RES_FONT);

	return true;
}

void GUIManager::SetFocus(Control *ctl)
{
	_focusedControl->_focused = false;
	ctl->_focused = true;
	_focusedControl = ctl;
}

NFont *GUIManager::GetGUIFont() noexcept
{
	return _drawInfo.guiFont;
}

int GUIManager::GetCharacterHeight() noexcept
{
	return _drawInfo.guiFont->GetCharacterHeight();
}

void GUIManager::DrawString(vec2 pos, vec3 color, NString text) noexcept
{
	_drawInfo.guiFont->Draw(text, pos, color);
}

void GUIManager::DrawString(vec2 pos, vec3 color, const char *fmt, ...) noexcept
{
	va_list args;
	char buff[FONT_BUFF];
	memset(buff, 0x0, FONT_BUFF);

	va_start(args, fmt);
	vsnprintf(buff, FONT_BUFF, fmt, args);
	va_end(args);

	_drawInfo.guiFont->Draw(buff, pos, color);
}

void GUIManager::Draw()
{
	/*_drawInfo.shader->Enable();
	_drawInfo.offset = 512;

	for (Control *ctl : _controls)
		ctl->_Draw(&_drawInfo);

	_drawInfo.shader->Disable();*/

	_drawInfo.guiFont->Render();
}

void GUIManager::Update(double deltaTime)
{
	NPoint _mousePos;
	int x = 0, y = 0;
	bool _lmbDown = false;// Platform_GetMouseDown(MOUSE_1);

	//Platform_GetPointerPosition(x, y);
	_mousePos.x = x;
	_mousePos.y = y;

	for (Control *ctl : _controls)
	{
		if (ctl->_controlRect.PtInRect(_mousePos))
		{
			if (_lmbDown)
			{
				if (ctl->_onClick) ctl->_onClick();
			}
			else if (!ctl->_hovered)
			{
				if (ctl->_onMouseEnter) ctl->_onMouseEnter();
				ctl->_hovered = true;
			}
		}
		else
		{
			if (ctl->_hovered)
			{
				if (ctl->_onMouseLeave) ctl->_onMouseLeave();
				ctl->_hovered = false;
			}
		}

		ctl->_Update(deltaTime);
	}
}

void GUIManager::RegisterControl(Control *ctl)
{
	ctl->_vertexBuffer = _drawInfo.renderer->CreateBuffer(BufferType::Vertex, true, true);
	ctl->_indexBuffer = _indexBuffer;
	ctl->_arrayBuffer = _drawInfo.renderer->CreateArrayBuffer();
	ctl->_InitializeControl(_drawInfo.renderer);

	_controls.push_back(ctl);
}

void GUIManager::RegisterFont(NFont *font)
{
	_fonts.push_back(font);
}

void GUIManager::ScreenResized()
{
	_drawInfo.guiFont->ScreenResized(Engine::GetScreenWidth(), Engine::GetScreenHeight());
}

void GUIManager::Release()
{

	ResourceManager::UnloadResourceByName("fnt_system", ResourceType::RES_FONT);
}
