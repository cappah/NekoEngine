/* NekoEngine
 *
 * CPUParticleSystemComponent.cpp
 * Author: Alexandru Naiman
 *
 * CPU Particle System Component
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

#include <random>

#include <Scene/Object.h>
#include <Renderer/VKUtil.h>
#include <Renderer/DebugMarker.h>
#include <Renderer/RenderPassManager.h>
#include <Engine/CameraManager.h>
#include <Engine/ResourceManager.h>
#include <Scene/Components/CPUParticleSystemComponent.h>
#include <System/AssetLoader/AssetLoader.h>

#define CPU_PSYSCOMP_MODULE	"CPU_ParticleSystemComponent"

using namespace glm;
using namespace std;

ENGINE_REGISTER_COMPONENT_CLASS(CPUParticleSystemComponent);

struct BillboardData
{
	mat4 viewProjection;
	vec4 cameraPosition;
	vec4 p0;
};

constexpr VkDeviceSize _billboardDataOffset = 0;
constexpr VkDeviceSize _drawIndirectCommandOffset = sizeof(BillboardData);
// UBOs need to be aligned at 256 bytes
constexpr VkDeviceSize _emitterDataOffset = _drawIndirectCommandOffset + sizeof(VkDrawIndirectCommand) + (256 - (_drawIndirectCommandOffset + sizeof(VkDrawIndirectCommand)));
// SSBOs need to be aligned at 32 bytes
constexpr VkDeviceSize _vertexBufferOffset = _emitterDataOffset; /*+ sizeof(EmitterData);*/

CPUParticleSystemComponent::CPUParticleSystemComponent(ComponentInitializer *initializer) :
	ObjectComponent(initializer),
	_particleBuffer(nullptr),
	_drawCommandBuffer(VK_NULL_HANDLE), _computeCommandBuffer(VK_NULL_HANDLE),
	_descriptorPool(VK_NULL_HANDLE),
	_drawDescriptorSet(VK_NULL_HANDLE), _computeDescriptorSet(VK_NULL_HANDLE),
	_vertexBufferSize(0),
	_texture(nullptr),
	_noiseTexture(nullptr)
{
	/*ArgumentMapType::iterator it{};
	const char *ptr{ nullptr };

	memset(&_emitterData, 0x0, sizeof(EmitterData));

	if (((it = initializer->arguments.find("initialvelocity")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		AssetLoader::ReadFloatArray(ptr, 3, _initialVelocity.x);

	if (((it = initializer->arguments.find("initialcolor")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		AssetLoader::ReadFloatArray(ptr, 4, _initialColor.x);

	if (((it = initializer->arguments.find("finalcolor")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		AssetLoader::ReadFloatArray(ptr, 4, _finalColor.x);

	if (((it = initializer->arguments.find("initialsize")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_initialSize = (float)atof(ptr);

	if (((it = initializer->arguments.find("finalsize")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_finalSize = (float)atof(ptr);

	if (((it = initializer->arguments.find("lifespan")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_lifespan = (float)atof(ptr);

	if (((it = initializer->arguments.find("rate")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_emitRate = (float)atof(ptr);

	if (((it = initializer->arguments.find("maxemit")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_maxEmit = atoi(ptr);

	if (((it = initializer->arguments.find("maxparticles")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_maxParticles = atoi(ptr);

	if (((it = initializer->arguments.find("velocitycurve")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_velocityCurve = atoi(ptr);

	if (((it = initializer->arguments.find("sizecurve")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_sizeCurve = atoi(ptr);

	if (((it = initializer->arguments.find("colorcurve")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_colorCurve = atoi(ptr);

	if (((it = initializer->arguments.find("emittertype")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_emitterType = atoi(ptr);

	if (((it = initializer->arguments.find("particletype")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_particleType = atoi(ptr);

	uint32_t nextType{ 0 };
	ArgumentMapRangeType range = initializer->arguments.equal_range("texture");

	for (ArgumentMapType::iterator it = range.first; it != range.second; ++it)
	{
		NString str = it->second.c_str();
		NArray<NString> split = str.Split(',');
		ParticleTexture texture{};

		if (split.Count() != 2)
			continue;

		_textureIds.Add(ResourceManager::GetResourceID(*split[1], ResourceType::RES_TEXTURE));
		
		texture.age = (float)atof(*split[0]);
		texture.index = nextType++;

		_particleTextures.Add(texture);
	}*/
}

int CPUParticleSystemComponent::Load()
{
	/*int ret = ObjectComponent::Load();
	if (ret != ENGINE_OK)
		return ret;
	
	// Textures
	{
		VkCommandBuffer textureCommandBuffer{ VKUtil::CreateOneShotCmdBuffer() };

		VkImageSubresourceRange range{};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseArrayLayer = 0;
		range.layerCount = (uint32_t)_textureIds.Count();
		range.baseMipLevel = 0;
		range.levelCount = 1;

		VkImageSubresourceRange srcRange{};
		srcRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		srcRange.baseArrayLayer = 0;
		srcRange.layerCount = 1;
		srcRange.baseMipLevel = 0;
		srcRange.levelCount = 1;

		if (_texture)
			VKUtil::TransitionImageLayout(_texture->GetImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, range);

		for (uint32_t i = 0; i < _textureIds.Count(); ++i)
		{
			Texture *tex{ (Texture *)ResourceManager::GetResource(_textureIds[i], ResourceType::RES_TEXTURE) };
			if (!tex)
				return ENGINE_FAIL;

			if (!_texture)
			{
				if ((_texture = new Texture(tex->GetFormat(), VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL,
											tex->GetWidth(), tex->GetHeight(), 1, false, VK_NULL_HANDLE, VK_SAMPLE_COUNT_1_BIT, 1, (uint32_t)_textureIds.Count())) == nullptr)
					return ENGINE_OUT_OF_RESOURCES;

				if (!_texture->CreateView(VK_IMAGE_ASPECT_COLOR_BIT, true))
					return ENGINE_FAIL;

				VKUtil::TransitionImageLayout(_texture->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, range);
			}
			
			VKUtil::TransitionImageLayout(tex->GetImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcRange);

			VkImageCopy region{};

			region.srcOffset.x = region.srcOffset.y = region.srcOffset.z = 0;
			region.dstOffset.x = region.dstOffset.y = region.dstOffset.z = 0;
			region.extent.width = _texture->GetWidth();
			region.extent.height = _texture->GetHeight();
			region.extent.depth = 1;

			region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.srcSubresource.baseArrayLayer = 0;
			region.srcSubresource.layerCount = 1;
			region.srcSubresource.mipLevel = 0;

			region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.dstSubresource.baseArrayLayer = i;
			region.dstSubresource.layerCount = 1;
			region.dstSubresource.mipLevel = 0;

			vkCmdCopyImage(textureCommandBuffer, tex->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _texture->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		}

		VKUtil::TransitionImageLayout(_texture->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, range);

		VKUtil::ExecuteOneShotCmdBuffer(textureCommandBuffer);

		for (uint32_t id : _textureIds)
			ResourceManager::UnloadResource(id, ResourceType::RES_TEXTURE);
	}

	// Descriptor pools
	{
		VkDescriptorPoolSize poolSizes[3]{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[0].descriptorCount = 2;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[1].descriptorCount = 1;
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[2].descriptorCount = 3;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 3;
		poolInfo.pPoolSizes = poolSizes;
		poolInfo.maxSets = 2;
		
		if (vkCreateDescriptorPool(VKUtil::GetDevice(), &poolInfo, VKUtil::GetAllocator(), &_descriptorPool) != VK_SUCCESS)
		{
			Logger::Log(GPU_PSYSCOMP_MODULE, LOG_CRITICAL, "Failed to create descriptor pool");
			return ENGINE_DESCRIPTOR_POOL_CREATE_FAIL;
		}
	}

	// Buffer
	{
		_vertexBufferSize = sizeof(ParticleVertex) * _emitterData.maxParticles * 2;
		_particleBuffer = new Buffer(_vertexBufferOffset + _vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, nullptr, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		_particleBuffer->Fill(0);
	}

	// Noise texture
	{
		VkDeviceSize noiseSize = 32 * 32 * 4;
		uint8_t *noise = (uint8_t *)calloc(noiseSize, sizeof(uint8_t));
		
		uniform_int_distribution<uint16_t> rd(0, 255);
		default_random_engine generator;

		for (VkDeviceSize i = 0; i < noiseSize; ++i)
			noise[i] = (uint8_t)rd(generator);

		_noiseTexture = new Texture(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, noiseSize, noise);
	}

	// Draw
	{
		VkDescriptorSetLayout layout{ PipelineManager::GetDescriptorSetLayout(DESC_LYT_ParticleDraw) };

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = _descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, &_drawDescriptorSet) != VK_SUCCESS)
		{
			Logger::Log(GPU_PSYSCOMP_MODULE, LOG_CRITICAL, "Failed to allocate draw descriptor set");
			return ENGINE_DESCRIPTOR_SET_CREATE_FAIL;
		}

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = _texture->GetImageView();
		imageInfo.sampler = Renderer::GetInstance()->GetNearestSampler();
		
		VkDescriptorBufferInfo uboInfo{};
		uboInfo.buffer = _particleBuffer->GetHandle();
		uboInfo.offset = _billboardDataOffset;
		uboInfo.range = sizeof(BillboardData);
		
		VkWriteDescriptorSet descriptorWrites[2]{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = _drawDescriptorSet;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &uboInfo;
		descriptorWrites[0].pImageInfo = nullptr;
		descriptorWrites[0].pTexelBufferView = nullptr;
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = _drawDescriptorSet;
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = nullptr;
		descriptorWrites[1].pImageInfo = &imageInfo;
		descriptorWrites[1].pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(VKUtil::GetDevice(), 2, descriptorWrites, 0, nullptr);

		_drawCommandBuffer = VKUtil::CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);
		VK_DBG_SET_OBJECT_NAME((uint64_t)ret, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, "Particle system draw command buffer");

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		VkCommandBufferInheritanceInfo inheritanceInfo{};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.occlusionQueryEnable = VK_FALSE;
		inheritanceInfo.renderPass = RenderPassManager::GetRenderPass(RP_Graphics);
		inheritanceInfo.subpass = 0;
		inheritanceInfo.framebuffer = Renderer::GetInstance()->GetDrawFramebuffer();

		beginInfo.pInheritanceInfo = &inheritanceInfo;

		vkBeginCommandBuffer(_drawCommandBuffer, &beginInfo);

		VkBuffer vb[]{ _particleBuffer->GetHandle() };
		VkDeviceSize offsets[]{ _vertexBufferOffset + _vertexBufferSize / 2 };
		vkCmdBindVertexBuffers(_drawCommandBuffer, 0, 1, vb, offsets);

		int32_t numTextures = 1;
		vkCmdBindPipeline(_drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipeline(PIPE_ParticleDraw));
		vkCmdPushConstants(_drawCommandBuffer, PipelineManager::GetPipelineLayout(PIPE_LYT_ParticleDraw), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int32_t), &numTextures);
		vkCmdBindDescriptorSets(_drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(PIPE_LYT_ParticleDraw), 0, 1, &_drawDescriptorSet, 0, nullptr);

		vkCmdDrawIndirect(_drawCommandBuffer, _particleBuffer->GetHandle(), _drawIndirectCommandOffset, 1, 0);

		vkEndCommandBuffer(_drawCommandBuffer);
	}

	Renderer::GetInstance()->AddParticleDrawCommandBuffer(_drawCommandBuffer);

	_emitterData.enable = 1;*/

	return ENGINE_OK;
}

void CPUParticleSystemComponent::Update(double deltaTime) noexcept
{
	ObjectComponent::Update(deltaTime);
}

void CPUParticleSystemComponent::UpdatePosition() noexcept
{
	/*_emitterData.position = _parent->GetPosition() + _position;
	_emitterData.rotation = _parent->GetRotation() + _rotation;
	_emitterData.scale = _scale;*/
}

void CPUParticleSystemComponent::UpdateData(VkCommandBuffer commandBuffer) noexcept
{
/*	Camera *cam{ CameraManager::GetActiveCamera() };
	BillboardData billboardData
	{
		cam->GetProjectionMatrix() * cam->GetView(),
		vec4(cam->GetPosition(), 1.0)
	};

	_particleBuffer->UpdateData((uint8_t *)&billboardData, _billboardDataOffset, sizeof(BillboardData), commandBuffer);
	_particleBuffer->UpdateData((uint8_t *)&_emitterData, _emitterDataOffset, sizeof(EmitterData) - sizeof(vec4) - sizeof(vec2), commandBuffer);*/
}

bool CPUParticleSystemComponent::Unload()
{
	if (!ObjectComponent::Unload())
		return false;

	/*delete _particleBuffer;
	delete _noiseTexture;
	delete _texture;

	if (_descriptorPool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(VKUtil::GetDevice(), _descriptorPool, VKUtil::GetAllocator());

	if (_drawCommandBuffer)
		VKUtil::FreeCommandBuffer(_drawCommandBuffer);*/

	return true;
}
