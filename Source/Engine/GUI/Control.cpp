/* NekoEngine
 *
 * Control.cpp
 * Author: Alexandru Naiman
 *
 * Control base
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

#include <GUI/Control.h>

Control::Control(int x, int y, int width, int height) :
	_controlRect{ NPoint(x, y), NPoint(width, height) },
	_enabled(true), _hovered(false), _visible(true),
	_text(""),
	_textColor(0.f), _hoveredTextColor(1.f, 0.f, 0.f),
	_font(GUIManager::GetGUIFont()),
	_vertexBuffer(nullptr), _indexBuffer(nullptr),
	_arrayBuffer(nullptr),
	_focused(false),
	_onClick(nullptr), _onRightClick(nullptr), _onMiddleClick(nullptr),
	_onMouseEnter(nullptr), _onMouseLeave(nullptr),
	_onMouseUp(nullptr), _onMouseDown(nullptr),
	_onMouseMoved(nullptr),
	_onKeyUp(nullptr), _onKeyDown(nullptr)
{
	GUIManager::RegisterControl(this);
}

Control::~Control()
{
	delete _vertexBuffer;
	delete _indexBuffer;
	delete _arrayBuffer;
}