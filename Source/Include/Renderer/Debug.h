/* NekoEngine
 *
 * Debug.h
 * Author: Alexandru Naiman
 *
 * Vulkan DebugMarker extension wrapper
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

#pragma once

#include <vulkan/vulkan.h>
#include <Engine/Engine.h>

class DebugMarker
{
public:
	static void Initialize(VkDevice device);
	static void SetObjectName(uint64_t object, VkDebugReportObjectTypeEXT type, const char *name);
	static void SetObjectTag(uint64_t object, VkDebugReportObjectTypeEXT type, uint64_t name, uint64_t tagSize, const void *tag);
	static void BeginRegion(VkCommandBuffer cmdBuffer, const char *name, glm::vec4 color);
	static void InsertMarker(VkCommandBuffer cmdBuffer, const char *name, glm::vec4 color);
	static void EndRegion(VkCommandBuffer cmdBuffer);
};

#if defined(NE_CONFIG_DEBUG) || defined(NE_CONFIG_DEVELOPMENT)
	#define DBG_SET_OBJECT_NAME(object, type, name) DebugMarker::SetObjectName(object, type, name)
	#define DBG_SET_OBJECT_TAG(object, type, name, tagSize, tag) DebugMarker::SetObjectTag(object, type, name, tagSize, tag)
	#define DBG_MARKER_BEGIN(cmdBuffer, name, color) DebugMarker::BeginRegion(cmdBuffer, name, color)
	#define DBG_MARKER_INSERT(cmdBuffer, name, color) DebugMarker::InsertMarker(cmdBuffer, name, color)
	#define DBG_MARKER_END(cmdBuffer) DebugMarker::EndRegion(cmdBuffer)
#else
	#define DBG_SET_OBJECT_NAME(object, type, name)
	#define DBG_SET_OBJECT_TAG(object, type, name, tagSize, tag)
	#define DBG_MARKER_BEGIN(cmdBuffer, name, color)
	#define DBG_MARKER_INSERT(cmdBuffer, name, color)
	#define DBG_MARKER_END(cmdBuffer)
#endif