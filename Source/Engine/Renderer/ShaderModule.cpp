/* NekoEngine
 *
 * ShaderModule.cpp
 * Author: Alexandru Naiman
 *
 * Shader module loader
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

#include <Engine/Engine.h>
#include <Renderer/VKUtil.h>
#include <Renderer/DebugMarker.h>
#include <Renderer/ShaderModule.h>
#include <System/VFS/VFS.h>
#include <Platform/Compat.h>

#define SHADER_MODULE		"Shader"

using namespace std;

ShaderModule::ShaderModule(ShaderModuleResource* res) noexcept
{ 
	_resourceInfo = res; 
	_module = VK_NULL_HANDLE;
};

int ShaderModule::Load()
{	
	size_t size{};
	void *data = nullptr;
	VFSFile *f = VFS::Open(GetResourceInfo()->filePath);

	if (!f)
	{
		Logger::Log(SHADER_MODULE, LOG_CRITICAL, "Failed to open file: %s", *GetResourceInfo()->filePath);
		return ENGINE_IO_FAIL;
	}

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	data = f->ReadAll(size);

	createInfo.codeSize = size;
	createInfo.pCode = (uint32_t *)data;

	if (vkCreateShaderModule(VKUtil::GetDevice(), &createInfo, VKUtil::GetAllocator(), &_module) != VK_SUCCESS)
	{
		Logger::Log(SHADER_MODULE, LOG_CRITICAL, "vkCreateShaderModule call failed");
		free(data);
		return ENGINE_FAIL;
	}

	VK_DBG_SET_OBJECT_NAME((uint64_t)_module, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, GetResourceInfo()->name.c_str());

	free(data);

	return ENGINE_OK;
}

ShaderModule::~ShaderModule() noexcept
{
	if (_module != VK_NULL_HANDLE)
		vkDestroyShaderModule(VKUtil::GetDevice(), _module, VKUtil::GetAllocator());
}
