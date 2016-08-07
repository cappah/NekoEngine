/* NekoEngine
 *
 * SMAA.cpp
 * Author: Alexandru Naiman
 *
 * Subpixel Morphological Anti-Aliasing
 * http://www.iryoku.com/smaa/
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

#define ENGINE_INTERNAL

#include <Engine/PostProcessor.h>
#include <Engine/ResourceManager.h>

#include <PostEffects/SMAA.h>
#include <PostEffects/SMAA/AreaTex.h>
#include <PostEffects/SMAA/SearchTex.h>

SMAA::SMAA() noexcept :
	Effect("SMAA"),
	_textures{ 0, 0 }
{
}

int SMAA::Load(RBuffer *sharedUbo)
{
	_shaderIds.push_back(ResourceManager::GetResourceID("sh_smaa", ResourceType::RES_SHADER));
	
	int ret = Effect::Load(sharedUbo);

	if (ret != ENGINE_OK)
		return ret;

	if((_textures[SMAA_TEX_AREA] = Engine::GetRenderer()->CreateTexture(TextureType::Tex2D)) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;
	_textures[SMAA_TEX_AREA]->SetStorage2D(1, TextureSizedFormat::RG_8UI, AREATEX_WIDTH, AREATEX_HEIGHT);
	_textures[SMAA_TEX_AREA]->SetImage2D(0, AREATEX_WIDTH, AREATEX_HEIGHT, TextureFormat::RG_INT, TextureInternalType::UnsignedByte, areaTexBytes);
	_textures[SMAA_TEX_AREA]->SetMinFilter(TextureFilter::Linear);
	_textures[SMAA_TEX_AREA]->SetMagFilter(TextureFilter::Linear);

	if((_textures[SMAA_TEX_SEARCH] = Engine::GetRenderer()->CreateTexture(TextureType::Tex2D)) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;
	_textures[SMAA_TEX_SEARCH]->SetStorage2D(1, TextureSizedFormat::R_8UI, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT);
	_textures[SMAA_TEX_SEARCH]->SetImage2D(0, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, TextureFormat::RED_INT, TextureInternalType::UnsignedByte, searchTexBytes);
	_textures[SMAA_TEX_SEARCH]->SetMinFilter(TextureFilter::Linear);
	_textures[SMAA_TEX_SEARCH]->SetMagFilter(TextureFilter::Linear);

	_shaders[0]->GetRShader()->SetTexture(U_TEXTURE4, _textures[SMAA_TEX_AREA]);
	_shaders[0]->GetRShader()->SetTexture(U_TEXTURE5, _textures[SMAA_TEX_SEARCH]);

	return ENGINE_OK;
}

void SMAA::Apply()
{
	float step = 0.f;

	_effectUbo->UpdateData(0, sizeof(float), &step);
	PostProcessor::DrawEffect(_shaders[0]->GetRShader(), false);

	step = 1.f;
	_effectUbo->UpdateData(0, sizeof(float), &step);
	PostProcessor::DrawEffect(_shaders[0]->GetRShader(), false);

	step = 2.f;
	_effectUbo->UpdateData(0, sizeof(float), &step);
	PostProcessor::DrawEffect(_shaders[0]->GetRShader(), false);
}

SMAA::~SMAA()
{
	delete _textures[SMAA_TEX_AREA];
	delete _textures[SMAA_TEX_SEARCH];
}
