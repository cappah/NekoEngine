/* NekoEngine
 *
 * GUIManager.h
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

#pragma once

#include <vector>

#include <GUI/GUIDefs.h>
#include <Engine/NFont.h>
#include <Engine/Vertex.h>
#include <Engine/Engine.h>
#include <Runtime/Runtime.h>
#include <Renderer/Renderer.h>

struct GUIDrawInfo
{
	Renderer *renderer;
	RShader *shader;
	NFont *guiFont;
	int offset;
};

class GUIManager
{
public:
	static int Initialize();

	ENGINE_API static NFont *GetGUIFont() noexcept;
	ENGINE_API static int GetCharacterHeight() noexcept;
	ENGINE_API static RBuffer *GetIndexBuffer() noexcept { return _indexBuffer; }

	ENGINE_API static void DrawString(glm::vec2 pos, glm::vec3 color, NString text) noexcept;
	ENGINE_API static void DrawString(glm::vec2 pos, glm::vec3 color, const char *fmt, ...) noexcept;

	ENGINE_API static void SetFocus(class Control *ctl);

	static void Draw();
	static void Update(double deltaTime);

	ENGINE_API static void RegisterControl(class Control *ctl);
	ENGINE_API static void RegisterFont(class NFont *font);

	static void ScreenResized();

	static void Release();

#ifdef ENGINE_INTERNAL

	//static void BindDescriptorSet(VkCommandBuffer buffer) { vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_GUI), 0, 1, &_descriptorSet, 0, nullptr); }

private:
	static RBuffer *_uniformBuffer, *_indexBuffer;
	static GUIDrawInfo _drawInfo;
	static glm::mat4 _projection;
	static bool _needUpdate;
	static std::vector<class NFont *> _fonts;
	static std::vector<class Control*> _controls;
#endif
};

#if defined(_MSC_VER)
template class ENGINE_API NArray<GUIVertex>;
#endif
