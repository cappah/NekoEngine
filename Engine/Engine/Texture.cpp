/* Neko Engine
 *
 * Texture.cpp
 * Author: Alexandru Naiman
 *
 * Texture class implementation 
 *
 * ----------------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman <alexandru dot naiman at icloud dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ALEXANDRU NAIMAN
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define ENGINE_INTERNAL

#include <string>
#include <string.h>

#include <Engine/Engine.h>
#include <Engine/Texture.h>
#include <System/AssetLoader/AssetLoader.h>
#include <System/VFS/VFS.h>

#define TEX_MODULE	"Texture"

using namespace std;

Texture::Texture(TextureResource *res) noexcept
{
	_resourceInfo = res;
	_parametersSet = false;
	_texture = nullptr;
	_id = res->id;
}

int Texture::Load()
{
	TextureType type = GetResourceInfo()->textureType == TextureResourceType::TEXTURE_2D ? TextureType::Tex2D : TextureType::TexCubemap;
	TextureFileFormat format = TextureFileFormat::DDS;
	string path("/");
	path.append(GetResourceInfo()->filePath);

	path.append(".dds");
	VFSFile *file = VFS::Open(path);

	if (!file)
	{
		path[path.length() - 3] = 't';
		path[path.length() - 2] = 'g';
		path[path.length() - 1] = 'a';

		file = VFS::Open(path);
		format = TextureFileFormat::TGA;
	}

	if (!file)
	{
		path[path.length() - 3] = 'k';
		path[path.length() - 2] = 't';
		path[path.length() - 1] = 'x';

		file = VFS::Open(path);
		format = TextureFileFormat::KTX;
	}

	if (!file)
	{
		path[path.length() - 3] = 't';
		path[path.length() - 2] = 'g';
		path[path.length() - 1] = 'a';

		file = VFS::Open(path);
		format = TextureFileFormat::TGA;

		if(!file)
		{
			Logger::Log(TEX_MODULE, LOG_CRITICAL, "Failed to load texture id %d, file name [%s]. Reason: unsupported texture format.", GetResourceInfo()->id, GetResourceInfo()->filePath.c_str());
			return ENGINE_FAIL;
		}
	}

	if(file->Seek(0, SEEK_END) == ENGINE_FAIL)
	{
		file->Close();
		return ENGINE_FAIL;
	}
	
	uint64_t size = file->Tell();
	
	if(file->Seek(0, SEEK_SET) == ENGINE_FAIL)
	{
		file->Close();
		return ENGINE_FAIL;
	}
	
	uint8_t *mem = (uint8_t*)calloc(size, sizeof(uint8_t));
	if(file->Read(mem, sizeof(uint8_t), size) == ENGINE_FAIL)
	{
		file->Close();
		free(mem);
		return ENGINE_FAIL;
	}

	_texture = Engine::GetRenderer()->CreateTexture(type);
	if (!_texture)
	{
		file->Close();
		free(mem);
		return ENGINE_FAIL;
	}

	if (!_texture->LoadFromMemory(format, mem, size))
	{
		file->Close();
		free(mem);
		return ENGINE_FAIL;
	}
	
	free(mem);

	if (Engine::GetConfiguration().Renderer.Mipmaps)
		_texture->GenerateMipmaps();

	Logger::Log(TEX_MODULE, LOG_DEBUG, "Loaded texture id %d from %s, size %dx%d", _resourceInfo->id, path.c_str(), _texture->GetWidth(), _texture->GetHeight());

	return ENGINE_OK;
}

void Texture::SetParameters(TextureParams &params) noexcept
{
	if (_parametersSet)
		return;

	_texture->SetMinFilter(params.minFilter);
	_texture->SetMagFilter(params.magFilter);
	_texture->SetWrapS(params.wrapS);
	_texture->SetWrapT(params.wrapT);
	
		/*int32_t aniso = Engine::GetConfiguration().Renderer.Aniso, maxAniso;
		GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso));

		if (maxAniso < aniso)
		{
			Logger::Log(MODULE, LOG_WARNING, "Requested %dx anisotropic filtering, but the maximum supported is %dx", aniso, maxAniso);
			aniso = maxAniso;
		}

		GL_CHECK(glTexParameteri(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso));*/

	if (Engine::GetConfiguration().Renderer.Anisotropic)
		_texture->SetAnisotropic(16);

	_parametersSet = true;
}

Texture::~Texture() noexcept
{
	delete _texture;
}
