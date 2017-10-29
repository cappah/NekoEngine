/* NekoEngine
 *
 * GUI.cpp
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

#include <GUI/GUI.h>
#include <Renderer/NFont.h>
#include <Renderer/VKUtil.h>
#include <Renderer/PipelineManager.h>
#include <Engine/ResourceManager.h>
#include <Engine/EventManager.h>
#include <Input/Keycodes.h>
#include <Input/Input.h>

#define GUI_MODULE	"GUIManager"

using namespace std;
using namespace glm;

typedef struct GUI_DATA
{
	mat4 Projection;
	ivec2 ScreenSize;
} GUIData;

VkDescriptorPool GUIManager::_descriptorPool = VK_NULL_HANDLE;
VkDescriptorSet GUIManager::_descriptorSet = VK_NULL_HANDLE;
VkCommandBuffer GUIManager::_commandBuffer = VK_NULL_HANDLE;
VkSampler GUIManager::_sampler = VK_NULL_HANDLE;
Buffer *GUIManager::_buffer = nullptr;
Buffer *GUIManager::_indexBuffer = nullptr;
NFont *GUIManager::_systemFont = nullptr;
bool GUIManager::_needUpdate = true;
vector<class NFont *> GUIManager::_fonts;
vector<class Control *> GUIManager::_controls;

static Control *_focusedControl{ nullptr };
static GUIData _guiData{};
static uint32_t _sceneUnloadedEventHandler;

int GUIManager::Initialize()
{
	Logger::Log(GUI_MODULE, LOG_INFORMATION, "Initializing...");

	if ((_buffer = new Buffer(sizeof(GUIData), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, nullptr, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) == nullptr)
	{ DIE("Out of resources"); }

	_guiData.ScreenSize = ivec2(Engine::GetConfiguration().Engine.ScreenWidth, Engine::GetConfiguration().Engine.ScreenHeight);
	_guiData.Projection = ortho(0.f, (float)Engine::GetConfiguration().Engine.ScreenWidth, (float)Engine::GetConfiguration().Engine.ScreenHeight, 0.f);

	_buffer->UpdateData((uint8_t *)&_guiData, 0, sizeof(GUIData));

	if (!VKUtil::CreateSampler(_sampler))
		return ENGINE_FAIL;
	
	if (!_CreateDescriptorSet())
		return ENGINE_DESCRIPTOR_SET_CREATE_FAIL;

	if ((_systemFont = (NFont *)ResourceManager::GetResourceByName("fnt_system", ResourceType::RES_FONT)) == nullptr)
		return ENGINE_FAIL;

	uint16_t indices[6]{ 0, 1, 2, 0, 2, 3 };
	if ((_indexBuffer = new Buffer(sizeof(uint16_t) * 6, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, (uint8_t *)indices, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) == nullptr)
	{ DIE("Out of resources"); }

	_sceneUnloadedEventHandler = EventManager::RegisterHandler(NE_EVT_SCN_UNLOADED, [&](int32_t eventId, void *eventData) {
		GUIManager::_controls.clear();
		_focusedControl = nullptr;
	});

	Logger::Log(GUI_MODULE, LOG_INFORMATION, "Initialized");

	return ENGINE_OK;
}

NFont *GUIManager::GetGUIFont() noexcept { return _systemFont; }

int GUIManager::GetCharacterHeight() noexcept { return _systemFont->GetCharacterHeight(); }

void GUIManager::DrawString(glm::vec2 pos, glm::vec3 color, NString text) noexcept
{
	_systemFont->Draw(text, pos, color);
}

void GUIManager::DrawString(glm::vec2 pos, glm::vec3 color, const char *fmt, ...) noexcept
{
	va_list args;
	char buff[8192];
	memset(buff, 0x0, 8192);

	va_start(args, fmt);
	vsnprintf(buff, 8192, fmt, args);
	va_end(args);

	_systemFont->Draw(buff, pos, color);
}

void GUIManager::SetFocus(Control *ctl)
{
	_focusedControl->_focused = false;
	ctl->_focused = true;
	_focusedControl = ctl;
}

void GUIManager::Update(double deltaTime)
{
	static Point lastMousePos{ Point() };
	Point mousePos{ Point() };
	long x{ 0 }, y{ 0 };

	bool lmbDown = Input::GetButtonDown(NE_MOUSE_LMB);
	bool rmbDown = Input::GetButtonDown(NE_MOUSE_RMB);
	bool mmbDown = Input::GetButtonDown(NE_MOUSE_MMB);
	bool lmbUp = Input::GetButtonUp(NE_MOUSE_LMB);
	bool rmbUp = Input::GetButtonUp(NE_MOUSE_RMB);
	bool mmbUp = Input::GetButtonUp(NE_MOUSE_MMB);

	if (!Input::PointerCaptured()) Input::GetPointerPosition(x, y);
	mousePos.x = x;
	mousePos.y = y;

	if (_focusedControl)
	{
		if (_focusedControl->_onKeyUp)
			for (uint8_t key : Input::GetKeyUpList())
				_focusedControl->_onKeyUp(key);

		if (_focusedControl->_onKeyDown)
			for (uint8_t key : Input::GetKeyDownList())
				_focusedControl->_onKeyDown(key);
	}

	for (Control *ctl : _controls)
	{
		if (!ctl->_visible)
			continue;

		if (ctl->_controlRect.PtInRect(mousePos))
		{
			if (lmbDown)
			{
				if (ctl->_onMouseDown)
					ctl->_onMouseDown(NE_MOUSE_LMB, mousePos);
			}
			else if (lmbUp)
			{
				if (ctl->_onClick) ctl->_onClick();
				if (ctl->_onMouseUp) ctl->_onMouseUp(NE_MOUSE_LMB, mousePos);
				_focusedControl = ctl;
			}

			if (rmbDown)
			{
				if (ctl->_onMouseDown)
					ctl->_onMouseDown(NE_MOUSE_RMB, mousePos);
			}
			else if (rmbUp)
			{
				if (ctl->_onRightClick) ctl->_onRightClick();
				if (ctl->_onMouseUp) ctl->_onMouseUp(NE_MOUSE_RMB, mousePos);
			}

			if (mmbDown)
			{
				if (ctl->_onMouseDown)
					ctl->_onMouseDown(NE_MOUSE_MMB, mousePos);
			}
			else if (mmbUp)
			{
				if (ctl->_onMiddleClick) ctl->_onMiddleClick();
				if (ctl->_onMouseUp) ctl->_onMouseUp(NE_MOUSE_MMB, mousePos);
			}

			if (mousePos != lastMousePos && ctl->_onMouseMoved)
				ctl->_onMouseMoved(mousePos, lastMousePos);

			if (!ctl->_hovered)
			{
				if (ctl->_onMouseEnter) ctl->_onMouseEnter();
				ctl->_hovered = true;
			}
		}
		else if (ctl->_hovered)
		{
			if (ctl->_onMouseLeave) ctl->_onMouseLeave();
			ctl->_hovered = false;
		}

		ctl->_Update(deltaTime);
	}

	lastMousePos = mousePos;
}

void GUIManager::UpdateData(VkCommandBuffer cmdBuffer)
{
	if (_needUpdate)
	{
		_buffer->UpdateData((uint8_t *)&_guiData, 0, sizeof(GUIData), cmdBuffer);
		_needUpdate = false;
	}

	for (Control *ctl : _controls)
		if (ctl->_visible)
			ctl->_UpdateData((void *)&cmdBuffer);

	for (NFont *font : _fonts)
		font->UpdateData(cmdBuffer);
}

void GUIManager::PrepareCommandBuffers()
{
	for (Control *ctl : _controls)
		if (ctl->_visible && ctl->_commandBuffer != VK_NULL_HANDLE)
			Renderer::GetInstance()->AddGUICommandBuffer(ctl->_commandBuffer);

	for (NFont *font : _fonts)
		Renderer::GetInstance()->AddGUICommandBuffer(font->GetCommandBuffer());
}

int GUIManager::RegisterControl(class Control *ctl)
{
	int ret = ENGINE_FAIL;

	if ((ret = ctl->_InitializeControl()) != ENGINE_OK)
		return ret;

	if ((ret = ctl->_CreateDescriptorSet()) != ENGINE_OK)
		return ret;

	if ((ret = ctl->_BuildCommandBuffer()) != ENGINE_OK)
		return ret;

	_controls.push_back(ctl);

	return ENGINE_OK;
}

void GUIManager::RegisterFont(class NFont *font)
{
	_fonts.push_back(font);
}

void GUIManager::UnregisterControl(class Control *ctl)
{
	_controls.erase(remove(_controls.begin(), _controls.end(), ctl), _controls.end());
}

void GUIManager::ScreenResized()
{
	_guiData.ScreenSize = ivec2(Engine::GetScreenWidth(), Engine::GetScreenHeight());
	_guiData.Projection = ortho(0.f, (float)Engine::GetScreenWidth(), (float)Engine::GetScreenHeight(), 0.f);
	_needUpdate = true;

	vkDeviceWaitIdle(VKUtil::GetDevice());

	for (Control *ctl : _controls)
	{
		if (ctl->_RebuildCommandBuffer() != ENGINE_OK)
		{ DIE("Failed to recreate command buffers"); }
	}

	vkDeviceWaitIdle(VKUtil::GetDevice());
}

bool GUIManager::_CreateDescriptorSet()
{
	VkDescriptorPoolSize size{};
	size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	size.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &size;
	poolInfo.maxSets = 1;

	if (vkCreateDescriptorPool(VKUtil::GetDevice(), &poolInfo, VKUtil::GetAllocator(), &_descriptorPool) != VK_SUCCESS)
	{
		Logger::Log(GUI_MODULE, LOG_CRITICAL, "Failed to create descriptor pool");
		return false;
	}

	VkDescriptorSetLayout guiDescriptorSetLayout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_Object);

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &guiDescriptorSetLayout;

	if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, &_descriptorSet) != VK_SUCCESS)
	{
		Logger::Log(GUI_MODULE, LOG_CRITICAL, "Failed to allocate descriptor set");
		return false;
	}

	VkDescriptorBufferInfo guiDataBlock = {};
	guiDataBlock.buffer = _buffer->GetHandle();
	guiDataBlock.offset = 0;
	guiDataBlock.range = sizeof(GUIData);

	VkWriteDescriptorSet writeBuffer = {};
	writeBuffer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeBuffer.dstSet = _descriptorSet;
	writeBuffer.dstBinding = 0;
	writeBuffer.dstArrayElement = 0;
	writeBuffer.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeBuffer.descriptorCount = 1;
	writeBuffer.pBufferInfo = &guiDataBlock;
	writeBuffer.pImageInfo = nullptr;
	writeBuffer.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(VKUtil::GetDevice(), 1, &writeBuffer, 0, nullptr);

	return true;
}

void GUIManager::Release()
{
	delete _buffer;
	delete _indexBuffer;

	EventManager::UnregisterHandler(NE_EVT_SCN_UNLOADED, _sceneUnloadedEventHandler);

	if (_sampler != VK_NULL_HANDLE)
		vkDestroySampler(VKUtil::GetDevice(), _sampler, VKUtil::GetAllocator());

	if (_descriptorPool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(VKUtil::GetDevice(), _descriptorPool, VKUtil::GetAllocator());

	if (_systemFont)
	{
		ResourceManager::UnloadResourceByName("fnt_system", ResourceType::RES_FONT);
		_systemFont = nullptr;
	}

	Logger::Log(GUI_MODULE, LOG_INFORMATION, "Released");
}

Buffer *GUIManager::CreateVertexBuffer(size_t size, uint8_t *data)
{
	return new Buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, data, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void GUIManager::FreeVertexBuffer(Buffer *buff)
{
	delete buff;
}
