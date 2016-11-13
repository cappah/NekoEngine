/* NekoEngine
 *
 * GUI.cpp
 * Author: Alexandru Naiman
 *
 * GUI system
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

#include <Renderer/GUI.h>
#include <Renderer/NFont.h>
#include <Renderer/VKUtil.h>
#include <Renderer/PipelineManager.h>
#include <Engine/ResourceManager.h>

#include <glm/gtc/matrix_transform.hpp>

#define GUI_MODULE	"GUI"

using namespace glm;

typedef struct GUI_DATA
{
	mat4 Projection;
	ivec2 ScreenSize;
} GUIData;

VkDescriptorPool GUI::_descriptorPool = VK_NULL_HANDLE;
VkDescriptorSet GUI::_descriptorSet = VK_NULL_HANDLE;
VkCommandBuffer GUI::_commandBuffer = VK_NULL_HANDLE;
VkSampler GUI::_sampler = VK_NULL_HANDLE;
Buffer *GUI::_buffer = nullptr;
NFont *GUI::_systemFont = nullptr;
bool GUI::_needUpdate = true;

static GUIData _guiData;

int GUI::Initialize()
{
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

	Renderer::GetInstance()->AddGUICommandBuffer(_systemFont->GetCommandBuffer());

	Logger::Log(GUI_MODULE, LOG_INFORMATION, "Initialized");

	return ENGINE_OK;
}

int GUI::GetCharacterHeight() noexcept { return _systemFont->GetCharacterHeight(); }

void GUI::DrawString(glm::vec2 pos, glm::vec3 color, NString text) noexcept
{
	_systemFont->Draw(text, pos, color);
}

void GUI::DrawString(glm::vec2 pos, glm::vec3 color, const char *fmt, ...) noexcept
{
	va_list args;
	char buff[8192];
	memset(buff, 0x0, 8192);

	va_start(args, fmt);
	vsnprintf(buff, 8192, fmt, args);
	va_end(args);

	_systemFont->Draw(buff, pos, color);
}

void GUI::Update(double deltaTime)
{
		
}

void GUI::UpdateData(VkCommandBuffer cmdBuffer)
{
	if (_needUpdate)
	{
		_buffer->UpdateData((uint8_t *)&_guiData, 0, sizeof(GUIData), cmdBuffer);
		_needUpdate = false;
	}

	_systemFont->UpdateData(cmdBuffer);
}

void GUI::ScreenResized(int width, int height)
{
	_guiData.ScreenSize = ivec2(width, height);
	_guiData.Projection = ortho(0.f, (float)width, (float)height, 0.f);
}

bool GUI::_CreateDescriptorSet()
{
	VkDescriptorPoolSize size = {};
	size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	size.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo = {};
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

void GUI::Release()
{
	delete _buffer;

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