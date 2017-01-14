/* NekoEngine
 *
 * Control.h
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

#pragma once

#include <vulkan/vulkan.h>

#include <GUI/GUIDefs.h>
#include <GUI/GUIManager.h>
#include <Renderer/NFont.h>
#include <Engine/Engine.h>

class Control
{
	friend class GUIManager;

public:
	ENGINE_API Control(int x = 0, int y = 0, int width = 10, int height = 10);

	ENGINE_API virtual const Point &GetPosition() const { return _controlRect.pos; }
	ENGINE_API virtual const Point &GetSize() const { return _controlRect.size; }
	ENGINE_API virtual const Rect &GetControlRect() const { return _controlRect; }
	ENGINE_API virtual NString &GetText() { return _text; }
	ENGINE_API virtual bool IsEnabled() { return _enabled; }
	ENGINE_API virtual bool IsVisible() { return _visible; }
	ENGINE_API virtual bool IsFocused() { return _focused; }

	ENGINE_API virtual void SetPosition(int x, int y) { _controlRect.x = x; _controlRect.y = y; }
	ENGINE_API virtual void SetPosition(Point pt) { _controlRect.x = pt.x; _controlRect.y = pt.y; }
	ENGINE_API virtual void SetSize(int width, int height) { _controlRect.w = width; _controlRect.h = height; }
	ENGINE_API virtual void SetSize(Point pt) { _controlRect.w = pt.x; _controlRect.h = pt.y; }
	ENGINE_API virtual void SetText(std::string text) { _text = text; }
	ENGINE_API virtual void SetTextColor(glm::vec3 &color) { _textColor = color; }
	ENGINE_API virtual void SetEnabled(bool enable) { _enabled = enable; }
	ENGINE_API virtual void SetVisible(bool visible) { _visible = visible; }
	ENGINE_API virtual void SetFont(NFont *font) { _font = font; }

	ENGINE_API virtual void SetFocus() { GUIManager::SetFocus(this); }

	ENGINE_API virtual void SetClickHandler(std::function<void(void)> onClick) { _onClick = onClick; }
	ENGINE_API virtual void SetRightClickHandler(std::function<void(void)> onRightClick) { _onRightClick = onRightClick; }
	ENGINE_API virtual void SetMiddleClickHandler(std::function<void(void)> onMiddleClick) { _onMiddleClick = onMiddleClick; }

	ENGINE_API virtual void SetMouseDownUp(std::function<void(uint8_t, const Point &)> onMouseUp) { _onMouseUp = onMouseUp; }
	ENGINE_API virtual void SetMouseDownHandler(std::function<void(uint8_t, const Point &)> onMouseDown) { _onMouseDown = onMouseDown; }
	ENGINE_API virtual void SetMouseEnterHandler(std::function<void(void)> onMouseEnter) { _onMouseEnter = onMouseEnter; }
	ENGINE_API virtual void SetMouseLeaveHandler(std::function<void(void)> onMouseLeave) { _onMouseLeave = onMouseLeave; }
	ENGINE_API virtual void SetMouseMovedHandler(std::function<void(const Point &, const Point &)> onMouseMoved) { _onMouseMoved = onMouseMoved; }

	ENGINE_API virtual void SetKeyUpHandler(std::function<void(uint8_t)> onKeyUp) { _onKeyUp = onKeyUp; }
	ENGINE_API virtual void SetKeyDownHandler(std::function<void(uint8_t)> onKeyDown) { _onKeyDown = onKeyDown; }

	ENGINE_API virtual ~Control();

protected:
	Rect _controlRect;
	bool _enabled, _hovered, _visible;
	NString _text;
	glm::vec3 _textColor, _hoveredTextColor;
	NFont *_font;
	Buffer *_buffer;
	bool _focused;

	std::function <void(void)> _onClick, _onRightClick, _onMiddleClick, _onMouseEnter, _onMouseLeave;
	std::function <void(uint8_t, const Point &)> _onMouseUp, _onMouseDown;
	std::function <void(const Point &, const Point &)> _onMouseMoved;
	std::function<void(uint8_t)> _onKeyUp, _onKeyDown;

	ENGINE_API virtual int _InitializeControl() { return ENGINE_OK;  }
	ENGINE_API virtual int _CreateDescriptorSet() { return ENGINE_OK; }
	ENGINE_API virtual int _BuildCommandBuffer() { return ENGINE_OK; }
	ENGINE_API virtual int _RebuildCommandBuffer() { return ENGINE_OK; }
	ENGINE_API virtual void _Update(double deltaTime) = 0;
	ENGINE_API virtual void _UpdateData(void *commandBuffer) { (void)commandBuffer; }

	VkCommandBuffer _commandBuffer;
	VkDescriptorSet _descriptorSet;
	VkDescriptorPool _descriptorPool;
	VkDeviceSize _vertexOffset;
};