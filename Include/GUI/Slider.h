/* NekoEngine
 *
 * Slider.h
 * Author: Alexandru Naiman
 *
 * Slider control
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

#include <GUI/Box.h>
#include <GUI/Control.h>
#include <Renderer/Texture.h>

class Slider : public Control
{
public:
	ENGINE_API Slider(int x = 0, int y = 0, int width = 75, int height = 24, bool vertical = false) :
		Control(x, y, width, height),
		_vertical(vertical), _dragging(false),
		_value(0.f),
		_barBox(new Box(x, y - 5 + height / 2, width, 5)),
		_sliderBox(new Box(x + 5, y, height / 2, height))
	{
		GUIManager::RegisterControl(_barBox);
		GUIManager::RegisterControl(_sliderBox);
	}

	ENGINE_API float GetValue() const { return _value; }
	ENGINE_API void SetValue(float value) { _value = value; }

	ENGINE_API void SetValueChangedHandler(std::function<void(float)> valueChanged) { _valueChanged = valueChanged; }

	ENGINE_API virtual ~Slider();

protected:
	GUIVertex _vertices[4];
	Texture *_tex;
	Box *_barBox, *_sliderBox;
	float _value;
	bool _vertical, _dragging;
	int _sliderMin, _sliderMax;
	std::function<void(float)> _valueChanged;

	virtual int _InitializeControl() override;
	virtual void _Update(double deltaTime) override;
	virtual void _MouseUp(uint8_t button, const Point &pos);
	virtual void _MouseDown(uint8_t button, const Point &pos);
	virtual void _MouseLeave();
	virtual void _MouseMoved(const Point &mousePos, const Point &lastMousePos);
};