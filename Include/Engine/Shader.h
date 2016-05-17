/* Neko Engine
 *
 * Shader.h
 * Author: Alexandru Naiman
 *
 * Shader loader
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

#pragma once

#include <Renderer/Renderer.h>
#include <Resource/Resource.h>
#include <Resource/ShaderResource.h>

#include <vector>
#include <string>

#define SHADER_POSITION_ATTRIBUTE	0
#define SHADER_NORMAL_ATTRIBUTE		1
#define SHADER_COLOR_ATTRIBUTE		2
#define SHADER_TANGENT_ATTRIBUTE	3
#define SHADER_UV_ATTRIBUTE			4
#define SHADER_TERRAINUV_ATTRIBUTE	5
#define SHADER_INDEX_ATTRIBUTE		6
#define SHADER_WEIGHT_ATTRIBUTE		7
#define SHADER_NUMBONES_ATTRIBUTE	8

// Textures
#define U_TEXTURE0				10
#define U_TEXTURE1				11
#define U_TEXTURE2				12
#define U_TEXTURE3				13
#define U_TEXTURE4				14
#define U_TEXTURE5				15
#define U_TEXTURE6				16
#define U_TEXTURE7				17
#define U_TEXTURE8				18
#define U_TEXTURE9				19

#define U_TEXTURE_CUBE			29

// Lighting shader
#define LT_AMBIENTAL			0
#define LT_DIRECTIONAL			1
#define LT_POINT				2
#define LT_SPOT					3

// Geometry shader
#define U_SHADER_TYPE			34
#define U_SHADER_SUB			35

// SSAO shader
#define SSAO_MAX_SAMPLES		128

// Shader types
#define SH_NM_SPEC		0
#define SH_NM			1
#define SH_SPEC			2
#define SH_TERRAIN		3
#define SH_UNLIT		4
#define SH_LIT			5
#define SH_SKYBOX		6
#define SH_SKYREFLECT	7

// Shader subroutines
#define SH_SUB_C_SPEC_MAP	0
#define SH_SUB_C_SPEC_ARG	1
#define SH_SUB_C_TERRAIN	2
#define SH_SUB_C_SKYBOX		3
#define SH_SUB_C_SKYREFLECT	4

#define SH_SUB_N_MAP		5
#define SH_SUB_N_ARG		6

#define SH_MAX_BONES		100

using namespace std;

class Shader : public Resource
{
public:
	ENGINE_API Shader(ShaderResource* res) noexcept
		: _shader(nullptr)
	{ 
		_resourceInfo = res; 
		_includePath = "/Shaders/include";
	};

	RShader* GetRShader() noexcept { return _shader; }

	ENGINE_API void SetNumTextures(int numTextures) noexcept { GetResourceInfo()->numTextures = numTextures; }
	ENGINE_API int GetNumTextures() noexcept { return GetResourceInfo()->numTextures; }

	ENGINE_API ShaderResource* GetResourceInfo() noexcept { return (ShaderResource *)_resourceInfo; }

	ENGINE_API virtual int Load() override;

	ENGINE_API void Enable() noexcept;
	ENGINE_API void Disable() noexcept;

	ENGINE_API virtual ~Shader() noexcept;
	
	static void SetDefines(Renderer *r);

private:
	RShader* _shader;
	std::string _includePath;

	int _CompileShader(ShaderType type, std::string& file);

	bool _IncludeFile(char* file, char** source, size_t* sourceSize);

	inline bool _AppendSourceString(char** dst, char* src, size_t* size);
};

