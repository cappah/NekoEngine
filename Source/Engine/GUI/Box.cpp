/* NekoEngine
 *
 * Box.cpp
 * Author: Alexandru Naiman
 *
 * Box control
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

#include <GUI/Box.h>
#include <Engine/ResourceManager.h>

#define BOX_MODULE	"GUI_Box"

using namespace glm;

void Box::_UpdateVertices()
{
	float y = (float)Engine::GetScreenHeight() - _controlRect.y - _controlRect.h;

	_vertices[0].posAndUV = vec4((float)_controlRect.x, (float)(y + _controlRect.h), 0, 0);
	_vertices[1].posAndUV = vec4((float)_controlRect.x, y, 0, 1);
	_vertices[2].posAndUV = vec4((float)(_controlRect.x + _controlRect.w), y, 1, 1);
	_vertices[3].posAndUV = vec4((float)(_controlRect.x + _controlRect.w), (float)(y + _controlRect.h), 1, 0);
}

int Box::_InitializeControl(Renderer *renderer)
{
	_UpdateVertices();
	_vertices[0].color = _vertices[1].color = _vertices[2].color = _vertices[3].color = vec4(.3f, .3f, .3f, 1.f);
	_vertexBuffer->SetStorage(sizeof(_vertices), _vertices);

	BufferAttribute attrib;
	attrib.name = "POSITION";
	attrib.index = 0;
	attrib.size = 4;
	attrib.type = BufferDataType::Float;
	attrib.normalize = false;
	attrib.stride = sizeof(GUIVertex);
	attrib.ptr = (void *)0;
	_vertexBuffer->AddAttribute(attrib);

	_arrayBuffer->SetVertexBuffer(_vertexBuffer);
	_arrayBuffer->SetIndexBuffer(_indexBuffer);
	_arrayBuffer->CommitBuffers();

	_texture = (Texture *)ResourceManager::GetResourceByName("tex_blank", ResourceType::RES_TEXTURE);
	_texture->GetRTexture()->LoadFromFile("Resources/button.tga");
	_texture->GetRTexture()->GenerateMipmaps();
	_texture->GetRTexture()->SetMinFilter(TextureFilter::Trilinear);
	_texture->GetRTexture()->SetMagFilter(TextureFilter::Linear);
	_texture->GetRTexture()->SetWrapS(TextureWrap::Repeat);
	_texture->GetRTexture()->SetWrapT(TextureWrap::Repeat);
	_texture->GetRTexture()->SetAnisotropic(16);

	return ENGINE_OK;
}

/*int Box::_BuildCommandBuffer()
{
	if (_commandBuffer != VK_NULL_HANDLE)
		VKUtil::FreeCommandBuffer(_commandBuffer);

	_commandBuffer = VKUtil::CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

	VkCommandBufferInheritanceInfo inheritanceInfo{};
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;
	inheritanceInfo.renderPass = RenderPassManager::GetRenderPass(RP_GUI);
	inheritanceInfo.subpass = 0;
	inheritanceInfo.framebuffer = Renderer::GetInstance()->GetGUIFramebuffer();

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = &inheritanceInfo;

	if (vkBeginCommandBuffer(_commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		Logger::Log(BOX_MODULE, LOG_CRITICAL, "vkBeginCommandBuffer call failed");
		return ENGINE_FAIL;
	}

	VK_DBG_MARKER_INSERT(_commandBuffer, "Box", vec4(1.0, 0.5, 0.0, 1.0));

	vkCmdBindVertexBuffers(_commandBuffer, 0, 1, &_buffer->GetHandle(), &_vertexOffset);
	vkCmdBindIndexBuffer(_commandBuffer, GUIManager::GetGUIIndexBuffer()->GetHandle(), 0, VK_INDEX_TYPE_UINT16);

	vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipeline(PIPE_GUI));
	GUIManager::BindDescriptorSet(_commandBuffer);
	vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PipelineLayoutId::PIPE_LYT_GUI), 1, 1, &_descriptorSet, 0, nullptr);

	vkCmdDrawIndexed(_commandBuffer, 6, 1, 0, 0, 0);

	if (vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS)
	{
		Logger::Log(BOX_MODULE, LOG_CRITICAL, "vkEndCommandBuffer call failed");
		return ENGINE_FAIL;
	}

	return ENGINE_OK;
}*/

void Box::_Draw(GUIDrawInfo *drawInfo)
{
	drawInfo->renderer->SetBlendFunc(BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha);
	drawInfo->renderer->EnableBlend(true);

	_arrayBuffer->Bind();

	drawInfo->shader->SetTexture(0, _texture->GetRTexture());
	drawInfo->renderer->DrawElements(PolygonMode::Triangles, 6, ElementType::UnsignedInt, 0);

	//int textY = (_controlRect.h - drawInfo->guiFont->GetCharacterHeight()) / 2;

	_arrayBuffer->Unbind();

	drawInfo->renderer->EnableBlend(false);
}

void Box::_Update(double deltaTime)
{
	(void)deltaTime;
}

Box::~Box()
{
	//GUIManager::FreeVertexBuffer(_buffer);
}
