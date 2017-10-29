/* NekoEngine
 *
 * Material.cpp
 * Author: Alexandru Naiman
 *
 * Material class implementation
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

#include <Renderer/SSAO.h>
#include <Renderer/VKUtil.h>
#include <Renderer/Material.h>
#include <Renderer/DebugMarker.h>
#include <Renderer/PipelineManager.h>
#include <Engine/Engine.h>
#include <Engine/ResourceManager.h>
#include <System/VFS/VFS.h>
#include <System/AssetLoader/AssetLoader.h>

#define LINE_BUFF	1024
#define MAT_MODULE	"Material"

using namespace std;

Material::Material(MaterialResource* res) noexcept
{
	_resourceInfo = res;

	_transparent = false;
	_noCulling = false;
	_animated = false;

	_diffuseTexture = nullptr;
	_normalTexture = nullptr;
	_specularTexture = nullptr;
	_emissionTexture = nullptr;

	_pipelineId = PIPE_Unlit;
	_pipelineLayoutId = PIPE_LYT_OneSampler;
	_descriptorSet = VK_NULL_HANDLE;
	_normalDescriptorSet = Renderer::GetInstance()->GetBlankTextureDescriptorSet();
	_descriptorPool = VK_NULL_HANDLE;
	_descriptorSetLayout = VK_NULL_HANDLE;

	memset(&_data, 0x0, sizeof(MaterialData));

	_data.Bloom = 1.f;
}

Material::Material(MaterialData &data) noexcept
{
	_noCulling = false;
	memcpy(&_data, &data, sizeof(MaterialData));

	_LoadInfo();
}

int Material::Load()
{
	NString lineBuff(LINE_BUFF);

	VFSFile *f = VFS::Open(GetResourceInfo()->filePath);
	if (!f)
	{
		Logger::Log(MAT_MODULE, LOG_CRITICAL, "Failed to open material file for %s", _resourceInfo->name.c_str());
		return ENGINE_IO_FAIL;
	}

	char header[7];
	if (f->Read(header, sizeof(char), 6) != 6)
	{
		Logger::Log(MAT_MODULE, LOG_CRITICAL, "Failed to read material file for %s", _resourceInfo->name.c_str());
		f->Close();
		return ENGINE_IO_FAIL;
	}
	header[6] = 0x0;

	if (strncmp(header, "NMTL2 ", 6))
	{
		Logger::Log(MAT_MODULE, LOG_CRITICAL, "Invalid header for material file for %s", _resourceInfo->name.c_str());
		f->Close();
		return ENGINE_INVALID_RES;
	}

	while (!f->EoF())
	{
		lineBuff.Clear();
		f->Gets(*lineBuff, LINE_BUFF);

		if (lineBuff.IsEmpty())
			continue;

		lineBuff.RemoveComment();
		lineBuff.RemoveNewLine();

		if (lineBuff.IsEmpty())
			continue;

		NArray<NString> split = lineBuff.Split('=');

		if (split.Count() != 2)
		{
			lineBuff.Count();

			if (lineBuff == "transparent")
				_transparent = true;
			if (lineBuff == "nocull")
				_noCulling = true;

			continue;
		}

		if (split[0] == "type")
		{
			if (split[1] == "phong")
				_data.Type = MT_Phong;
			else if (split[1] == "phong_spec")
				_data.Type = MT_PhongSpecular;
			else if (split[1] == "phong_spec_em")
				_data.Type = MT_PhongSpecularEmission;
			else if (split[1] == "phong_nm")
				_data.Type = MT_NormalPhong;
			else if (split[1] == "phong_spec_nm")
				_data.Type = MT_NormalPhongSpecular;
			else if (split[1] == "phong_spec_em_nm")
				_data.Type = MT_NormalPhongSpecularEmission;
			else if (split[1] == "unlit")
				_data.Type = MT_Unlit;
			else if (split[1] == "skysphere")
				_data.Type = MT_Skysphere;
			else if (split[1] == "skysphere_reflection")
				_data.Type = MT_SkysphereReflection;
			else if (split[1] == "terrain")
				_data.Type = MT_Terrain;
		}
		else if (split[0] == "kdiffuse")
			AssetLoader::ReadFloatArray(*split[1], 3, &_data.Diffuse.x);
		else if (split[0] == "kspecular")
			AssetLoader::ReadFloatArray(*split[1], 3, &_data.Specular.x);
		else if (split[0] == "kemission")
			AssetLoader::ReadFloatArray(*split[1], 3, &_data.Emission.x);
		else if (split[0] == "shininess")
			_data.Shininess = (float)split[1];
		else if (split[0] == "ior")
			_data.IndexOfRefraction = (float)split[1];
		else if (split[0] == "diffuse")
			_diffuseTextureId = split[1];
		else if (split[0] == "normal")
			_normalTextureId = split[1];
		else if (split[0] == "specular")
			_specularTextureId = split[1];
		else if (split[0] == "emission")
			_emissionTextureId = split[1];
	}

	if (_diffuseTextureId.Length())
	{
		if (_diffuseTextureId == "tex_blank")
			_diffuseTexture = Renderer::GetInstance()->GetBlankTexture();
		else if ((_diffuseTexture = (Texture *)ResourceManager::GetResourceByName(*_diffuseTextureId, ResourceType::RES_TEXTURE)) == nullptr)
		{
			Unload();
			Logger::Log(MAT_MODULE, LOG_CRITICAL, "Failed to load texture %s for material %s", *_diffuseTextureId, _resourceInfo->name.c_str());
			return ENGINE_INVALID_RES;
		}
	}

	if (_normalTextureId.Length())
	{
		if (_normalTextureId == "tex_blank")
			_normalTexture = Renderer::GetInstance()->GetBlankTexture();
		else if ((_normalTexture = (Texture *)ResourceManager::GetResourceByName(*_normalTextureId, ResourceType::RES_TEXTURE)) == nullptr)
		{
			Unload();
			Logger::Log(MAT_MODULE, LOG_CRITICAL, "Failed to load texture %s for material %s", *_normalTextureId, _resourceInfo->name.c_str());
			return ENGINE_INVALID_RES;
		}
	}

	if (_specularTextureId.Length())
	{
		if (_specularTextureId == "tex_blank")
			_specularTexture = Renderer::GetInstance()->GetBlankTexture();
		else if ((_specularTexture = (Texture *)ResourceManager::GetResourceByName(*_specularTextureId, ResourceType::RES_TEXTURE)) == nullptr)
		{
			Unload();
			Logger::Log(MAT_MODULE, LOG_CRITICAL, "Failed to load texture %s for material %s", *_specularTextureId, _resourceInfo->name.c_str());
			return ENGINE_INVALID_RES;
		}
	}

	if (_emissionTextureId.Length())
	{
		if (_emissionTextureId == "tex_blank")
			_emissionTexture = Renderer::GetInstance()->GetBlankTexture();
		else if ((_emissionTexture = (Texture *)ResourceManager::GetResourceByName(*_emissionTextureId, ResourceType::RES_TEXTURE)) == nullptr)
		{
			Unload();
			Logger::Log(MAT_MODULE, LOG_CRITICAL, "Failed to load texture %s for material %s", *_emissionTextureId, _resourceInfo->name.c_str());
			return ENGINE_INVALID_RES;
		}
	}

	_LoadInfo();

	Logger::Log(MAT_MODULE, LOG_DEBUG, "Loaded material %s", _resourceInfo->name.c_str());

	return ENGINE_OK;
}

void Material::SetAnimated(bool animated)
{
	if (_animated == animated)
		return;

	if (_animated)
	{
		_pipelineId = (PipelineId)((uint8_t)_pipelineId - 10);
		_pipelineLayoutId = (PipelineLayoutId)((uint8_t)_pipelineLayoutId - 10);;
	}
	else
	{
		_pipelineId = (PipelineId)((uint8_t)_pipelineId + 10);
		_pipelineLayoutId = (PipelineLayoutId)((uint8_t)_pipelineLayoutId + 10);;
	}

	_animated = animated;
}

void Material::Enable(VkCommandBuffer buffer)
{
	vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipeline(_pipelineId));
	vkCmdPushConstants(buffer, PipelineManager::GetPipelineLayout(_pipelineLayoutId), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(MaterialData), &_data);
	vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(_pipelineLayoutId), 2, 1, &_descriptorSet, 0, nullptr);
}

void Material::BindNormal(VkCommandBuffer buffer)
{
	vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineManager::GetPipelineLayout(_animated ? PIPE_LYT_Anim_Depth : PIPE_LYT_Depth), 2, 1, &_normalDescriptorSet, 0, nullptr);
}

bool Material::CreateDescriptorSet()
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = 1;

	switch (_data.Type)
	{
		case MT_NormalPhong:
		case MT_PhongSpecular:
			poolSize.descriptorCount = 2;
		break;
		case MT_PhongSpecularEmission:
		case MT_NormalPhongSpecular:
			poolSize.descriptorCount = 3;
		break;
		case MT_NormalPhongSpecularEmission:
			poolSize.descriptorCount = 4;
		break;
	}

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 2;

	if (vkCreateDescriptorPool(VKUtil::GetDevice(), &poolInfo, VKUtil::GetAllocator(), &_descriptorPool) != VK_SUCCESS)
	{
		Logger::Log(MAT_MODULE, LOG_CRITICAL, "Failed to create descriptor pool");
		return false;
	}

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &_descriptorSetLayout;

	if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, &_descriptorSet) != VK_SUCCESS)
	{
		Logger::Log(MAT_MODULE, LOG_CRITICAL, "Failed to allocate descriptor set");
		return false;
	}

	VkWriteDescriptorSet writeSampler{};
	VKUtil::WriteDS(&writeSampler, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nullptr, _descriptorSet, 0);

	vector<VkDescriptorImageInfo> imageInfo;

	VkDescriptorImageInfo diffuseImageInfo{};
	diffuseImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	diffuseImageInfo.imageView = _diffuseTexture->GetImageView();
	diffuseImageInfo.sampler = _diffuseTexture->GetSampler();
	imageInfo.push_back(diffuseImageInfo);

	if (_data.Type == MT_PhongSpecular)
	{
		VkDescriptorImageInfo specularImageInfo{};
		specularImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		specularImageInfo.imageView = _specularTexture->GetImageView();
		specularImageInfo.sampler = _specularTexture->GetSampler();
		imageInfo.push_back(specularImageInfo);
	}
	else if (_data.Type == MT_PhongSpecularEmission)
	{
		VkDescriptorImageInfo specularImageInfo{};
		specularImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		specularImageInfo.imageView = _specularTexture->GetImageView();
		specularImageInfo.sampler = _specularTexture->GetSampler();
		imageInfo.push_back(specularImageInfo);

		VkDescriptorImageInfo emissionImageInfo{};
		emissionImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		emissionImageInfo.imageView = _emissionTexture->GetImageView();
		emissionImageInfo.sampler = _emissionTexture->GetSampler();
		imageInfo.push_back(emissionImageInfo);
	}
	else if (_data.Type == MT_NormalPhongSpecular)
	{
		VkDescriptorImageInfo specularImageInfo{};
		specularImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		specularImageInfo.imageView = _specularTexture->GetImageView();
		specularImageInfo.sampler = _specularTexture->GetSampler();
		imageInfo.push_back(specularImageInfo);
	}
	else if (_data.Type == MT_NormalPhongSpecularEmission)
	{
		VkDescriptorImageInfo specularImageInfo{};
		specularImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		specularImageInfo.imageView = _specularTexture->GetImageView();
		specularImageInfo.sampler = _specularTexture->GetSampler();
		imageInfo.push_back(specularImageInfo);

		VkDescriptorImageInfo emissionImageInfo{};
		emissionImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		emissionImageInfo.imageView = _emissionTexture->GetImageView();
		emissionImageInfo.sampler = _emissionTexture->GetSampler();
		imageInfo.push_back(emissionImageInfo);
	}

	writeSampler.descriptorCount = (uint32_t)imageInfo.size();
	writeSampler.pImageInfo = imageInfo.data();

	vkUpdateDescriptorSets(VKUtil::GetDevice(), 1, &writeSampler, 0, nullptr);

	if (!_normalTexture)
		return true;

	VkDescriptorSetLayout normalSetLayout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_OneSampler);
	allocInfo.pSetLayouts = &normalSetLayout;

	if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, &_normalDescriptorSet) != VK_SUCCESS)
	{
		Logger::Log(MAT_MODULE, LOG_CRITICAL, "Failed to allocate descriptor set");
		return false;
	}

	VkDescriptorImageInfo normalImageInfo{};
	normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	normalImageInfo.imageView = _normalTexture->GetImageView();
	normalImageInfo.sampler = _normalTexture->GetSampler();

	writeSampler.dstSet = _normalDescriptorSet;
	writeSampler.pBufferInfo = nullptr;
	writeSampler.pTexelBufferView = nullptr;
	writeSampler.descriptorCount = 1;
	writeSampler.pImageInfo = &normalImageInfo;

	vkUpdateDescriptorSets(VKUtil::GetDevice(), 1, &writeSampler, 0, nullptr);

	return true;
}

void Material::Unload()
{
	_descriptorSetLayout = VK_NULL_HANDLE;

	if (_descriptorPool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(VKUtil::GetDevice(), _descriptorPool, VKUtil::GetAllocator());

	/*if (_diffuseTextureId.Length() && _diffuseTextureId != "tex_blank") ResourceManager::UnloadResourceByName(*_diffuseTextureId, ResourceType::RES_TEXTURE);
	if (_normalTextureId.Length() && _normalTextureId != "tex_blank") ResourceManager::UnloadResourceByName(*_normalTextureId, ResourceType::RES_TEXTURE);
	if (_specularTextureId.Length() && _specularTextureId != "tex_blank") ResourceManager::UnloadResourceByName(*_specularTextureId, ResourceType::RES_TEXTURE);
	if (_emissionTextureId.Length() && _emissionTextureId != "tex_blank") ResourceManager::UnloadResourceByName(*_emissionTextureId, ResourceType::RES_TEXTURE);*/
}

void Material::_LoadInfo()
{
	if (_data.Type == MT_Phong)
	{
		_pipelineId = PIPE_Phong;
		_pipelineLayoutId = PIPE_LYT_OneSampler;
		_descriptorSetLayout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_OneSampler);
	}
	else if (_data.Type == MT_PhongSpecular)
	{
		_pipelineId = PIPE_PhongSpecular;
		_pipelineLayoutId = PIPE_LYT_TwoSamplers;
		_descriptorSetLayout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_TwoSamplers);
	}
	else if (_data.Type == MT_PhongSpecularEmission)
	{
		_pipelineId = PIPE_PhongSpecularEmissive;
		_pipelineLayoutId = PIPE_LYT_ThreeSamplers;
		_descriptorSetLayout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_ThreeSamplers);
	}
	else if (_data.Type == MT_NormalPhong)
	{
		_pipelineId = PIPE_PhongNormal;
		_pipelineLayoutId = PIPE_LYT_OneSampler;
		_descriptorSetLayout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_OneSampler);
	}
	else if (_data.Type == MT_NormalPhongSpecular)
	{
		_pipelineId = PIPE_PhongNormalSpecular;
		_pipelineLayoutId = PIPE_LYT_TwoSamplers;
		_descriptorSetLayout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_TwoSamplers);
	}
	else if (_data.Type == MT_NormalPhongSpecularEmission)
	{
		_pipelineId = PIPE_PhongNormalSpecularEmissive;
		_pipelineLayoutId = PIPE_LYT_ThreeSamplers;
		_descriptorSetLayout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_ThreeSamplers);
	}
	else if (_data.Type == MT_Unlit)
	{
		_pipelineId = PIPE_Unlit;
		_pipelineLayoutId = PIPE_LYT_OneSampler;
		_descriptorSetLayout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_OneSampler);
	}
	else if (_data.Type == MT_Skysphere)
	{
		_pipelineId = PIPE_Skysphere;
		_pipelineLayoutId = PIPE_LYT_OneSampler;
		_descriptorSetLayout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_OneSampler);
	}
	else if (_data.Type == MT_SkysphereReflection)
	{
		/*_pipeline = PipelineManager::GetPipeline("skybox_reflection");
		_pipelineLayout = PipelineManager::GetPipelineLayout(PIPE_LYT_TwoSamplers);
		_descriptorSetLayout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_TwoSamplers);*/
	}
	else if (_data.Type == MT_Terrain)
	{
		_pipelineId = PIPE_Terrain;
		_pipelineLayoutId = PIPE_LYT_OneSampler;
		_descriptorSetLayout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_OneSampler);
	}
	
	if (_transparent)
		_pipelineId = (PipelineId)((uint8_t)_pipelineId + 20);
}

Material::~Material()
{
	Unload();
}