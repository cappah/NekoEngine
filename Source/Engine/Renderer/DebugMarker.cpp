/* NekoEngine
 *
 * DebugMarkerk.h
 * Author: Alexandru Naiman
 *
 * Vulkan DebugMarker extension wrapper
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

#include <vulkan/vulkan.h>
#include <Renderer/VKUtil.h>
#include <Renderer/DebugMarker.h>

#define DBGMKR_MODULE	"DebugMarker"

static PFN_vkDebugMarkerSetObjectNameEXT _DebugMakerSetObjectNameEXT = nullptr;
static PFN_vkDebugMarkerSetObjectTagEXT _DebugMarkerSetObjectTagEXT = nullptr;
static PFN_vkCmdDebugMarkerBeginEXT _DebugMarkerBeginEXT = nullptr;
static PFN_vkCmdDebugMarkerInsertEXT _DebugMarkerInsertEXT = nullptr;
static PFN_vkCmdDebugMarkerEndEXT _DebugMarkerEndEXT = nullptr;

void DebugMarker::Initialize(VkDevice device)
{
	Logger::Log(DBGMKR_MODULE, LOG_INFORMATION, "Initializing...");

	_DebugMakerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT");
	_DebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT");
	_DebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT");
	_DebugMarkerInsertEXT = (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT");
	_DebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT");

	if(_DebugMakerSetObjectNameEXT)
		Logger::Log(DBGMKR_MODULE, LOG_INFORMATION, "Initialized");
	else
		Logger::Log(DBGMKR_MODULE, LOG_INFORMATION, "VK_EXT_debug_marker not present");
}

void DebugMarker::SetObjectName(uint64_t object, VkDebugReportObjectTypeEXT type, const char *name)
{
	if (!_DebugMakerSetObjectNameEXT)
		return;

	VkDebugMarkerObjectNameInfoEXT nameInfo{};
	nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
	nameInfo.objectType = type;
	nameInfo.object = object;
	nameInfo.pObjectName = name;

	_DebugMakerSetObjectNameEXT(VKUtil::GetDevice(), &nameInfo);
}

void DebugMarker::SetObjectTag(uint64_t object, VkDebugReportObjectTypeEXT type, uint64_t name, uint64_t size, const void *tag)
{
	if (!_DebugMarkerSetObjectTagEXT)
		return;

	VkDebugMarkerObjectTagInfoEXT tagInfo{};
	tagInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
	tagInfo.objectType = type;
	tagInfo.object = object;
	tagInfo.tagName = name;
	tagInfo.tagSize = size;
	tagInfo.pTag = tag;

	_DebugMarkerSetObjectTagEXT(VKUtil::GetDevice(), &tagInfo);
}

void DebugMarker::BeginRegion(VkCommandBuffer cmdBuffer, const char *name, glm::vec4 color)
{
	if (!_DebugMarkerBeginEXT)
		return;

	VkDebugMarkerMarkerInfoEXT markerInfo{};
	markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
	markerInfo.pMarkerName = name;
	memcpy(markerInfo.color, &color[0], sizeof(float) * 4);

	_DebugMarkerBeginEXT(cmdBuffer, &markerInfo);
}

void DebugMarker::InsertMarker(VkCommandBuffer cmdBuffer, const char *name, glm::vec4 color)
{
	if (!_DebugMarkerInsertEXT)
		return;

	VkDebugMarkerMarkerInfoEXT markerInfo{};
	markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
	markerInfo.pMarkerName = name;
	memcpy(markerInfo.color, &color[0], sizeof(float) * 4);

	_DebugMarkerInsertEXT(cmdBuffer, &markerInfo);
}

void DebugMarker::EndRegion(VkCommandBuffer cmdBuffer)
{
	if (!_DebugMarkerEndEXT)
		return;

	_DebugMarkerEndEXT(cmdBuffer);
}
