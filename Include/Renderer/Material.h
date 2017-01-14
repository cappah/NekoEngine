/* NekoEngine
 *
 * Material.h
 * Author: Alexandru Naiman
 *
 * Renderer material
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

#include <stdint.h>

#include <Resource/Resource.h>
#include <Renderer/Texture.h>
#include <Resource/MaterialResource.h>
#include <Renderer/PipelineManager.h>

enum MaterialType : int32_t
{
	MT_Phong = 0,
	MT_PhongSpecular,
	MT_PhongSpecularEmission,
	MT_NormalPhong,
	MT_NormalPhongSpecular,
	MT_NormalPhongSpecularEmission,
	MT_Unlit,
	MT_Skysphere,
	MT_SkysphereReflection,
	MT_Terrain
};

typedef struct MATERIAL_DATA
{
	glm::vec4 Diffuse;
	glm::vec4 Specular;
	glm::vec4 Emission;
	float Shininess;
	float IndexOfRefraction;
	float Bloom;
	int32_t Type;
} MaterialData;

class Material :
	public Resource
{
public:
	ENGINE_API Material(MaterialResource *res) noexcept;
	ENGINE_API Material(MaterialData &data) noexcept;

	ENGINE_API MaterialResource* GetResourceInfo() noexcept { return (MaterialResource*)_resourceInfo; }
	
	/*ENGINE_API glm::vec3 GetColor() noexcept { return glm::vec3(_data.Color[0], _data.Color[1], _data.Color[2]); }
	ENGINE_API void SetColor(glm::vec3& color) noexcept { memcpy(_data.Color, &color.x, sizeof(float) * 3); }*/

	ENGINE_API MaterialType GetType() const { return (MaterialType)_data.Type; }
	ENGINE_API bool IsTransparent() const { return _transparent; }
	ENGINE_API bool HasNormalMap() const { return _normalTexture != nullptr; }
	ENGINE_API void SetAnimated(bool animated);

	ENGINE_API virtual int Load() override;
	ENGINE_API bool CreateDescriptorSet();

	ENGINE_API void SetType(MaterialType type) { _data.Type = (int32_t)type; }
	ENGINE_API Texture *SetDiffuseTexture(Texture *tex) { Texture *ret = _diffuseTexture; _diffuseTexture = tex; return ret; }
	ENGINE_API Texture *SetNormalTexture(Texture *tex) { Texture *ret = _normalTexture; _normalTexture = tex; return ret; }
	ENGINE_API Texture *SetSpecularTexture(Texture *tex) { Texture *ret = _specularTexture; _specularTexture = tex; return ret; }
	ENGINE_API Texture *SetEmissionTexture(Texture *tex) { Texture *ret = _emissionTexture; _emissionTexture = tex; return ret; }

	ENGINE_API void Unload();

	ENGINE_API virtual ~Material();

	VkPipelineLayout GetPipelineLayout() { return PipelineManager::GetPipelineLayout(_pipelineLayoutId); }

	void Enable(VkCommandBuffer buffer);
	void BindNormal(VkCommandBuffer buffer);
	bool HasDescriptorSet() { return _descriptorSet != VK_NULL_HANDLE; }

private:
	bool _transparent;
	bool _noCulling;
	bool _animated;
	NArray<SamplerParams> _textureParams;
	MaterialData _data;

	NString _diffuseTextureId;
	NString _normalTextureId;
	NString _specularTextureId;
	NString _emissionTextureId;

	Texture *_diffuseTexture;
	Texture *_normalTexture;
	Texture *_specularTexture;
	Texture *_emissionTexture;

	PipelineId _pipelineId;
	PipelineLayoutId _pipelineLayoutId;

	VkDescriptorSet _descriptorSet, _normalDescriptorSet;
	VkDescriptorPool _descriptorPool;
	VkDescriptorSetLayout _descriptorSetLayout;

	void _LoadInfo();
};

#if defined(_MSC_VER)
template class ENGINE_API NArray<Material*>;
#endif
