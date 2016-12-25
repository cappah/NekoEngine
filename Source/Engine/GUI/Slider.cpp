/* NekoEngine
 *
 * Slider.cpp
 * Author: Alexandru Naiman
 *
 * Slider control
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

#include <GUI/Slider.h>
#include <Engine/Keycodes.h>

#define SLIDER_MODULE	"GUI_Slider"

using namespace glm;
using namespace std;
using namespace std::placeholders;

int Slider::_InitializeControl(Renderer *renderer)
{
	_sliderMin = (_vertical ? _controlRect.y : _controlRect.x) + 5;
	_sliderMax = (_vertical ? _controlRect.y + _controlRect.h - _sliderBox->GetSize().y : _controlRect.x + _controlRect.w - _sliderBox->GetSize().x) - 5;

	_onMouseUp = bind(&Slider::_MouseUp, this, _1, _2);
	_onMouseDown = bind(&Slider::_MouseDown, this, _1, _2);
	_onMouseMoved = bind(&Slider::_MouseMoved, this, _1, _2);
	_onMouseLeave = bind(&Slider::_MouseLeave, this);

	return ENGINE_OK;
}

void Slider::_Update(double deltaTime)
{
	(void)deltaTime;
}

void Slider::_MouseUp(uint8_t button, const Point &pos)
{
	if (button == NE_MOUSE_LMB)
		_dragging = false;
}

void Slider::_MouseDown(uint8_t button, const Point &pos)
{
	if (button == NE_MOUSE_LMB && _sliderBox->GetControlRect().PtInRect(pos))
		_dragging = true;
}

void Slider::_MouseLeave()
{
	_dragging = false;
}

void Slider::_MouseMoved(const Point &mousePos, const Point &lastMousePos)
{
	if (!_dragging)
		return;

	Point offset{ lastMousePos - mousePos };
	Point boxPosition{ _sliderBox->GetPosition() };
	int sliderPosition{ 0 };

	offset.x = -offset.x;
	offset.y = -offset.y;

	if (_vertical)
	{
		sliderPosition = offset.y;
		boxPosition.y += offset.y;

		if (boxPosition.y > _sliderMax)
			boxPosition.y = _sliderMax;
		else if (boxPosition.y < _sliderMin)
			boxPosition.y = _sliderMin;
	}
	else
	{
		sliderPosition = offset.x;
		boxPosition.x += offset.x;

		if (boxPosition.x > _sliderMax)
			boxPosition.x = _sliderMax;
		else if (boxPosition.x < _sliderMin)
			boxPosition.x = _sliderMin;
	}

	_value = (float)(sliderPosition - _sliderMin) / (float)(_sliderMax - _sliderMin);
	_sliderBox->SetPosition(boxPosition);

	if (_valueChanged)
		_valueChanged(_value);
}

Slider::~Slider()
{
	delete _barBox;
	delete _sliderBox;
}