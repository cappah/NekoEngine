/* Neko Engine
 *
 * Material.cpp
 * Author: Alexandru Naiman
 *
 * Material class implementation
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

#include <zlib.h>

#include <Engine/Engine.h>
#include <Engine/Material.h>
#include <Engine/ResourceManager.h>
#include <System/VFS/VFS.h>

#define LINE_BUFF	1024
#define MAT_MODULE	"Material"

void Material::_LoadTexture(const char* name, int *id, TextureFilter* minFilter, TextureFilter* magFilter, TextureWrap* wrapS, TextureWrap* wrapT)
{
	vector<char*> texInfo = EngineUtils::SplitString(name, ',');

	if (texInfo.size() != 5)
	{
		Logger::Log(MAT_MODULE, LOG_CRITICAL, "Malformed texture specification in material \"%s\"" ,_resourceInfo->name.c_str());
		*id = -1;
		for(char* p : texInfo)
			free(p);
		return;
	}

	*id = ResourceManager::GetResourceID(texInfo[0], ResourceType::RES_TEXTURE);

	if (*id == ENGINE_NOT_FOUND)
	{
		Logger::Log(MAT_MODULE, LOG_CRITICAL, "Texture \"%s\" not found; required by material \"%s\"", texInfo[0], _resourceInfo->name.c_str());
		for(char* p : texInfo)
			free(p);
		return;
	}

	if (Engine::GetConfiguration().Renderer.Mipmaps)
	{
		if (!strncmp(texInfo[1], "linear", 6))
			*minFilter = TextureFilter::Trilinear;
		if (!strncmp(texInfo[1], "nearest", 7))
			*minFilter = TextureFilter::Bilinear;
	}
	else
	{
		if (!strncmp(texInfo[1], "linear", 6))
			*minFilter = TextureFilter::Linear;
		if (!strncmp(texInfo[1], "nearest", 7))
			*minFilter = TextureFilter::Nearest;
	}

	if (!strncmp(texInfo[2], "linear", 6))
		*magFilter = TextureFilter::Linear;
	else if (!strncmp(texInfo[2], "nearest", 7))
		*magFilter = TextureFilter::Nearest;

	if (!strncmp(texInfo[3], "clamp", 5))
		*wrapS = TextureWrap::ClampToEdge;
	else if (!strncmp(texInfo[3], "repeat", 6))
		*wrapS = TextureWrap::Repeat;
	else if (!strncmp(texInfo[3], "mirror", 6))
		*wrapS = TextureWrap::MirroredRepeat;

	if (!strncmp(texInfo[4], "clamp", 5))
		*wrapT = TextureWrap::ClampToEdge;
	else if (!strncmp(texInfo[4], "repeat", 6))
		*wrapT = TextureWrap::Repeat;
	else if (!strncmp(texInfo[4], "mirror", 6))
		*wrapT = TextureWrap::MirroredRepeat;
	
	for(char* p : texInfo)
		free(p);
}

int Material::Load()
{
	char lineBuff[LINE_BUFF];
	memset(lineBuff, 0x0, LINE_BUFF);

	string path("/");
	path.append(GetResourceInfo()->filePath);

	VFSFile *f = VFS::Open(path);
	if(!f)
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

	if (strncmp(header, "NMTL1 ", 6))
	{
		Logger::Log(MAT_MODULE, LOG_CRITICAL, "Invalid header for material file for %s", _resourceInfo->name.c_str());
		f->Close();
		return ENGINE_INVALID_RES;
	}

	while (!f->EoF())
	{
		memset(lineBuff, 0x0, LINE_BUFF);
		f->Gets(lineBuff, LINE_BUFF);

		if (lineBuff[0] == 0x0)
			continue;

		EngineUtils::RemoveComment(lineBuff);
		EngineUtils::RemoveNewline(lineBuff);

		if (lineBuff[0] == 0x0)
			continue;

		vector<char*> split = EngineUtils::SplitString(lineBuff, '=');

		if (split.size() != 2)
		{
			if (strstr(lineBuff, "blend"))
				_blend = true;
			if (strstr(lineBuff, "noculling"))
				_noCulling = true;
			if (strstr(lineBuff, "bloom"))
				_materialInfo.Bloom = 1.f;
			if (strstr(lineBuff, "nodiscard"))
				_materialInfo.NoDiscard = 1.f;
			continue;
		}

		size_t len = strlen(split[0]);

		if (!strncmp(split[0], "type", 4))
		{
			size_t splitLen = strlen(split[1]);
			if (!strncmp(split[1], "normalandspecular", splitLen))
				_materialInfo.MaterialType = SH_NM_SPEC;
			if (!strncmp(split[1], "normal", splitLen))
				_materialInfo.MaterialType = SH_NM;
			if (!strncmp(split[1], "specular", splitLen))
				_materialInfo.MaterialType = SH_SPEC;
			if (!strncmp(split[1], "terrain", splitLen))
				_materialInfo.MaterialType = SH_TERRAIN;
			if (!strncmp(split[1], "unlit", splitLen))
				_materialInfo.MaterialType = SH_UNLIT;
			if (!strncmp(split[1], "lit", splitLen))
				_materialInfo.MaterialType = SH_LIT;
			if (!strncmp(split[1], "skybox", splitLen))
				_materialInfo.MaterialType = SH_SKYBOX;
			if (!strncmp(split[1], "skyreflect", splitLen))
				_materialInfo.MaterialType = SH_SKYREFLECT;
		}
		else if (!strncmp(split[0], "kdiffuse", len))
			_materialInfo.DiffuseConstant = (float)atof(split[1]);
		else if (!strncmp(split[0], "kspecular", len))
			_materialInfo.SpecularConstant = (float)atof(split[1]);
		else if (!strncmp(split[0], "shininess", len))
			_materialInfo.Shininess = (float)atof(split[1]);
		else if (!strncmp(split[0], "texture", len))
		{
			TextureFilter minFilter = TextureFilter::Trilinear;
			TextureFilter magFilter = TextureFilter::Linear;
			TextureWrap wrapS = TextureWrap::ClampToEdge;
			TextureWrap wrapT = TextureWrap::ClampToEdge;
			int id;

			_LoadTexture(split[1], &id, &minFilter, &magFilter, &wrapS, &wrapT);

			if (id == -1)
				continue;

			_textureIds.push_back(id);
			_textureParams.push_back(
			{
				minFilter,
				magFilter,
				wrapS,
				wrapT
			});
		}
		
		for(char* p : split)
			free(p);
	}

	f->Close();

	for (int i = 0; i < _textureIds.size(); i++)
	{
		Texture *tex = (Texture *)ResourceManager::GetResource(_textureIds[i], ResourceType::RES_TEXTURE);

		if (tex == nullptr)
		{
			Unload();
			Logger::Log(MAT_MODULE, LOG_CRITICAL, "Failed to load texture %d for material %s", i, _resourceInfo->name.c_str());
			return ENGINE_INVALID_RES;
		}

		tex->SetParameters(_textureParams[i]);
		_textures.push_back(tex);
	}

	if (_materialInfo.MaterialType == SH_NM || _materialInfo.MaterialType == SH_NM_SPEC)
		_subroutines[SUB_NORMAL] = SH_SUB_N_MAP;
	else
		_subroutines[SUB_NORMAL] = SH_SUB_N_ARG;

	if (_materialInfo.MaterialType == SH_SPEC || _materialInfo.MaterialType == SH_NM_SPEC)
		_subroutines[SUB_COLOR] = SH_SUB_C_SPEC_MAP;
	else if (_materialInfo.MaterialType == SH_LIT || _materialInfo.MaterialType == SH_UNLIT || _materialInfo.MaterialType == SH_NM)
		_subroutines[SUB_COLOR] = SH_SUB_C_SPEC_ARG;
	else if (_materialInfo.MaterialType == SH_TERRAIN)
		_subroutines[SUB_COLOR] = SH_SUB_C_TERRAIN;
	else if (_materialInfo.MaterialType == SH_SKYBOX)
		_subroutines[SUB_COLOR] = SH_SUB_C_SKYBOX;
	else if (_materialInfo.MaterialType == SH_SKYREFLECT)
		_subroutines[SUB_COLOR] = SH_SUB_C_SKYREFLECT;

	_materialUbo = Engine::GetRenderer()->CreateBuffer(BufferType::Uniform, true, false);
	if(!_materialUbo)
	{
		Unload();
		return ENGINE_OUT_OF_RESOURCES;
	}
	_materialUbo->SetStorage(sizeof(MaterialBlock), &_materialInfo);
	
	Logger::Log(MAT_MODULE, LOG_DEBUG, "Loaded material %s", _resourceInfo->name.c_str());

	return ENGINE_OK;
}

void Material::Enable(RShader* shader)
{
	shader->FSSetUniformBuffer(1, 0, sizeof(MaterialBlock), _materialUbo);
	shader->VSSetUniformBuffer(1, 0, sizeof(MaterialBlock), _materialUbo);

	for (unsigned int i = 0; i < _textures.size(); i++)
	{
		if(_textures[i]->GetResourceInfo()->textureType == TextureResourceType::TEXTURE_CUBEMAP)
			shader->SetTexture(U_TEXTURE_CUBE, _textures[i]->GetRTexture());
		else
			shader->SetTexture(U_TEXTURE0 + i, _textures[i]->GetRTexture());
	}

	shader->SetSubroutines(ShaderType::Fragment, 2, _subroutines);
}

void Material::Unload()
{
	for (Texture *t : _textures)
		ResourceManager::UnloadResource(t->GetResourceId(), ResourceType::RES_TEXTURE);

	_textures.clear();
	_textureParams.clear();
	_textureIds.clear();
	
	delete _materialUbo;
}

Material::~Material()
{
	Unload();
}
