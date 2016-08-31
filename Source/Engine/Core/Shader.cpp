/* NekoEngine
 *
 * Shader.cpp
 * Author: Alexandru Naiman
 *
 * Shader class implementation
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

#include <string>
#include <sstream>
#include <cstring>

#include <Engine/Engine.h>
#include <Engine/Shader.h>
#include <Scene/Scene.h>
#include <System/VFS/VFS.h>
#include <Platform/Compat.h>

#define SHADER_MODULE		"Shader"

#define SHADER_BUFF		65535
#define SHADER_LINE_BUFF	512

using namespace std;

int Shader::Load()
{
	if((_shader = Engine::GetRenderer()->CreateShader()) == nullptr)
		return ENGINE_OUT_OF_RESOURCES;
	
	NString vsPath = NString::StringWithFormat(VFS_MAX_FILE_NAME, "/Shaders/%s/%s.%s", Engine::GetRenderer()->GetShadingLanguage(), *GetResourceInfo()->vsFilePath, Engine::GetRenderer()->GetShadingLanguage());
	if (!_CompileShader(ShaderType::Vertex, vsPath))
		return ENGINE_LOAD_VS_FAIL;

	NString fsPath = NString::StringWithFormat(VFS_MAX_FILE_NAME, "/Shaders/%s/%s.%s", Engine::GetRenderer()->GetShadingLanguage(), *GetResourceInfo()->fsFilePath, Engine::GetRenderer()->GetShadingLanguage());
	if (!_CompileShader(ShaderType::Fragment, fsPath))
		return ENGINE_LOAD_FS_FAIL;

	if (GetResourceInfo()->gsFilePath.Length() > 0)
	{
		NString gsPath = NString::StringWithFormat(VFS_MAX_FILE_NAME, "/Shaders/%s/%s.%s", Engine::GetRenderer()->GetShadingLanguage(), *GetResourceInfo()->gsFilePath, Engine::GetRenderer()->GetShadingLanguage());
		if (!_CompileShader(ShaderType::Geometry, gsPath))
			return ENGINE_LOAD_GS_FAIL;
	}

	if (!_shader->Link())
		return ENGINE_LOAD_SHADER_FAIL;

	return ENGINE_OK;
}

void Shader::Enable() noexcept
{
	_shader->Enable();
}

void Shader::Disable() noexcept
{
	_shader->Disable();
}

int Shader::_CompileShader(ShaderType type, NString &file)
{
	size_t sourceSize = SHADER_BUFF;
	char *source = nullptr, lineBuff[SHADER_LINE_BUFF];	
	memset(lineBuff, 0x0, SHADER_LINE_BUFF);
	
	VFSFile *f = VFS::Open(file);
	if (!f)
	{
		Logger::Log(SHADER_MODULE, LOG_CRITICAL, "Failed to open source file: %s", *file);
		return ENGINE_IO_FAIL;
	}

	if ((source = (char *)calloc(sourceSize, sizeof(char))) == nullptr)
	{ DIE("Memory allocation failed"); }
	
	while (f->Gets(lineBuff, SHADER_LINE_BUFF) != 0)
	{
		char *str = nullptr;

		if ((str = strstr(lineBuff, "#include")) == nullptr)
		{
			if (!_AppendSourceString(&source, lineBuff, &sourceSize))
			{
				f->Close();
				free(source);
				Logger::Log(SHADER_MODULE, LOG_CRITICAL, "This should NEVER happen. Shader is too large\n");
				return false;
			}
		}
		else
		{
			str = str + 10;
			char *pEnd = strchr(str, '"');
			*pEnd = 0x0;

			if (!_IncludeFile(str, &source, &sourceSize))
			{
				f->Close();
				free(source);
				Logger::Log(SHADER_MODULE, LOG_CRITICAL, "Error compiling shader <%s>:\nIncluded file %s not found\n", *file, str);
				return false;
			}
		}

		memset(lineBuff, 0x0, SHADER_LINE_BUFF);
	}

	f->Close();

	if (!_shader->LoadFromSource(type, 1, source, (int)strlen(source)))
	{
		Logger::Log(SHADER_MODULE, LOG_CRITICAL, "Error compiling shader <%s>:\ns\n", *file/*, infoLog*/);
		free(source);
		return false;
	}

	free(source);
	return true;
}

void Shader::SetDefines(Renderer *r)
{
	char buff[SHADER_LINE_BUFF];
	
	// Vertex attributes
	r->AddShaderDefine("SHADER_POSITION_ATTRIBUTE", to_string(SHADER_POSITION_ATTRIBUTE));
	r->AddShaderDefine("SHADER_NORMAL_ATTRIBUTE", to_string(SHADER_NORMAL_ATTRIBUTE));
	r->AddShaderDefine("SHADER_COLOR_ATTRIBUTE", to_string(SHADER_COLOR_ATTRIBUTE));
	r->AddShaderDefine("SHADER_TANGENT_ATTRIBUTE", to_string(SHADER_TANGENT_ATTRIBUTE));
	r->AddShaderDefine("SHADER_UV_ATTRIBUTE", to_string(SHADER_UV_ATTRIBUTE));
	r->AddShaderDefine("SHADER_TERRAINUV_ATTRIBUTE", to_string(SHADER_TERRAINUV_ATTRIBUTE));
	r->AddShaderDefine("SHADER_INDEX_ATTRIBUTE", to_string(SHADER_INDEX_ATTRIBUTE));
	r->AddShaderDefine("SHADER_WEIGHT_ATTRIBUTE", to_string(SHADER_WEIGHT_ATTRIBUTE));
	r->AddShaderDefine("SHADER_NUMBONES_ATTRIBUTE", to_string(SHADER_NUMBONES_ATTRIBUTE));

	// Textures
	for (int i = 0; i < 10; i++)
	{
		memset(buff, 0x0, SHADER_LINE_BUFF);
		if(snprintf(buff, SHADER_LINE_BUFF, "U_TEXTURE%d", i) >= SHADER_LINE_BUFF)
		{ DIE("Failed to create shader defines"); }
		r->AddShaderDefine(buff, to_string(U_TEXTURE0 + i));
	}
	r->AddShaderDefine("U_TEXTURE_CUBE", to_string(U_TEXTURE_CUBE));

	// Lighting
	r->AddShaderDefine("LT_AMBIENTAL", to_string(LT_AMBIENTAL));
	r->AddShaderDefine("LT_DIRECTIONAL", to_string(LT_DIRECTIONAL));
	r->AddShaderDefine("LT_POINT", to_string(LT_POINT));
	r->AddShaderDefine("LT_SPOT", to_string(LT_SPOT));

	// Geometry shader
	r->AddShaderDefine("U_SHADER_TYPE", to_string(U_SHADER_TYPE));
	r->AddShaderDefine("U_SHADER_SUB", to_string(U_SHADER_SUB));

	// SSAO shader
	r->AddShaderDefine("SSAO_MAX_SAMPLES", to_string(SSAO_MAX_SAMPLES));

	// Shader types
	memset(buff, 0x0, SHADER_LINE_BUFF);
	if(snprintf(buff, SHADER_LINE_BUFF, "%d.0", SH_NM_SPEC) >= SHADER_LINE_BUFF)
	{ DIE("Failed to create shader defines"); }
	r->AddShaderDefine("SH_NM_SPEC", buff);
	
	memset(buff, 0x0, SHADER_LINE_BUFF);
	if(snprintf(buff, SHADER_LINE_BUFF, "%d.0", SH_NM) >= SHADER_LINE_BUFF)
	{ DIE("Failed to create shader defines"); }
	r->AddShaderDefine("SH_NM", buff);
	
	memset(buff, 0x0, SHADER_LINE_BUFF);
	if(snprintf(buff, SHADER_LINE_BUFF, "%d.0", SH_SPEC) >= SHADER_LINE_BUFF)
	{ DIE("Failed to create shader defines"); }
	r->AddShaderDefine("SH_SPEC", buff);
	
	memset(buff, 0x0, SHADER_LINE_BUFF);
	if(snprintf(buff, SHADER_LINE_BUFF, "%d.0", SH_TERRAIN) >= SHADER_LINE_BUFF)
	{ DIE("Failed to create shader defines"); }
	r->AddShaderDefine("SH_TERRAIN", buff);
	
	memset(buff, 0x0, SHADER_LINE_BUFF);
	if(snprintf(buff, SHADER_LINE_BUFF, "%d.0", SH_UNLIT) >= SHADER_LINE_BUFF)
	{ DIE("Failed to create shader defines"); }
	r->AddShaderDefine("SH_UNLIT", buff);
	
	memset(buff, 0x0, SHADER_LINE_BUFF);
	if(snprintf(buff, SHADER_LINE_BUFF, "%d.0", SH_LIT) >= SHADER_LINE_BUFF)
	{ DIE("Failed to create shader defines"); }
	r->AddShaderDefine("SH_LIT", buff);
	
	memset(buff, 0x0, SHADER_LINE_BUFF);
	if(snprintf(buff, SHADER_LINE_BUFF, "%d.0", SH_SKYBOX) >= SHADER_LINE_BUFF)
	{ DIE("Failed to create shader defines"); }
	r->AddShaderDefine("SH_SKYBOX", buff);
	
	memset(buff, 0x0, SHADER_LINE_BUFF);
	if(snprintf(buff, SHADER_LINE_BUFF, "%d.0", SH_SKYREFLECT) >= SHADER_LINE_BUFF)
	{ DIE("Failed to create shader defines"); }
	r->AddShaderDefine("SH_SKYREFLECT", buff);
	
	// Subroutines
	r->AddShaderDefine("SH_SUB_C_SPEC_MAP", to_string(SH_SUB_C_SPEC_MAP));
	r->AddShaderDefine("SH_SUB_C_SPEC_ARG", to_string(SH_SUB_C_SPEC_ARG));
	r->AddShaderDefine("SH_SUB_C_TERRAIN", to_string(SH_SUB_C_TERRAIN));
	r->AddShaderDefine("SH_SUB_C_SKYBOX", to_string(SH_SUB_C_SKYBOX));
	r->AddShaderDefine("SH_SUB_C_SKYREFLECT", to_string(SH_SUB_C_SKYREFLECT));
	r->AddShaderDefine("SH_SUB_N_MAP", to_string(SH_SUB_N_MAP));
	r->AddShaderDefine("SH_SUB_N_ARG", to_string(SH_SUB_N_ARG));

	// Output
	r->AddShaderDefine("O_FRAGCOLOR", "0");
	r->AddShaderDefine("O_COLORTEXTURE", "1");
	r->AddShaderDefine("O_BRIGHTCOLOR", "1");
	r->AddShaderDefine("O_POSITION", "0");
	r->AddShaderDefine("O_NORMAL", "1");
	r->AddShaderDefine("O_COLORSPECULAR", "2");
	r->AddShaderDefine("O_MATERIALINFO", "3");
	r->AddShaderDefine("O_VIEWNORMAL", "4");
	
	r->AddShaderDefine("SH_MAX_BONES", to_string(SH_MAX_BONES));
	
#ifdef _DEBUG
	r->AddShaderDefine("_DEBUG", "1");
#endif
}

bool Shader::_IncludeFile(char* file, char** source, size_t* sourceSize)
{
	VFSFile *f;
	char lineBuff[SHADER_LINE_BUFF];
	memset(lineBuff, 0x0, SHADER_LINE_BUFF);
	NString path = _includePath;
	path.Append("/");
	path.Append(file);

	memset(lineBuff, 0x0, SHADER_LINE_BUFF);

	f = VFS::Open(path);
	if (!f)
		return false;

	while (f->Gets(lineBuff, SHADER_LINE_BUFF) != 0)
	{
		char *str = nullptr;

		if ((str = strstr(lineBuff, "#include")) == nullptr)
		{
			if (!_AppendSourceString(source, lineBuff, sourceSize))
				return false;
		}
	/*	else
		{
			str = str + 10;
			str[strlen(str) - 2] = 0x0;

			if (!_IncludeFile(str, source, sourceSize))
				return false;
		}*/

		memset(lineBuff, 0x0, SHADER_LINE_BUFF);
	}

	f->Close();

	return true;
}

bool Shader::_AppendSourceString(char **dst, char *src, size_t *size)
{
	size_t len = strlen(src);

	while (len > *size - strlen(*dst))
	{
		char *newptr = (char *)reallocarray(*dst, *size + SHADER_BUFF, sizeof(char));

		if (newptr == nullptr)
		{
			free(*dst);
			*dst = nullptr;
			return false;
		}

		*dst = newptr;
		*size += SHADER_BUFF;
	}

	strncat(*dst, src, len);
	return true;
}

Shader::~Shader() noexcept
{
	delete _shader;
}
