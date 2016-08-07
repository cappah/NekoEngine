/* NekoEngine
 *
 * Material.h
 * Author: Alexandru Naiman
 *
 * Material class definition 
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

#include <vector>

#include <stdint.h>

#include <Resource/Resource.h>
#include <Engine/Texture.h>
#include <Engine/Shader.h>
#include <Resource/MaterialResource.h>

#define SUB_COLOR	0
#define SUB_NORMAL	1

typedef struct MATERIAL_BLOCK
{
	float DiffuseConstant;
	float SpecularConstant;
	float Shininess;
	float Bloom;
	float MaterialType;
	float NoDiscard;
	float AnimatedMesh;
	float padding;
} MaterialBlock;

class Material :
	public Resource
{
public:
	ENGINE_API Material(MaterialResource* res) noexcept :
		_blend(false),
		_noCulling(false),
		_materialUbo(nullptr),
		_subroutines{0, 0}
	{
		_resourceInfo = res;
		memset(&_materialInfo, 0x0, sizeof(MaterialBlock));
	};
	
	ENGINE_API MaterialResource* GetResourceInfo() noexcept { return (MaterialResource*)_resourceInfo; }
	
	ENGINE_API void SetAnimatedMesh(bool animated) noexcept;
	
	ENGINE_API bool EnableBlend() { return _blend; }
	ENGINE_API bool DisableCulling() { return _noCulling; }

	ENGINE_API virtual int Load() override;

	ENGINE_API void Enable(RShader* shader);

	ENGINE_API void AddTextureId(int id, TextureParams params) noexcept { _textureIds.push_back(id); _textureParams.push_back(params); }

	ENGINE_API void Unload();

	ENGINE_API virtual ~Material();

private:
	bool _blend;
	bool _noCulling;
	std::vector<int> _textureIds;
	std::vector<Texture*> _textures;
	std::vector<TextureParams> _textureParams;
	RBuffer *_materialUbo;
	MaterialBlock _materialInfo;

	uint32_t _subroutines[2];

	void _LoadTexture(const char* name, int* id, TextureFilter* minFilter, TextureFilter* magFilter, TextureWrap* wrapS, TextureWrap* wrapT);
};
