/* Neko Engine
 *
 * GLShader.cpp
 * Author: Alexandru Naiman
 *
 * OpenGL 4.5 Renderer Implementation
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

#include "GLShader.h"
#include "GLRenderer.h"
#include "GLTexture.h"
#include "GLBuffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GLenum GL_ShaderType[6] =
{
	GL_VERTEX_SHADER,
	GL_FRAGMENT_SHADER,
	GL_GEOMETRY_SHADER,
	GL_TESS_CONTROL_SHADER,
	GL_TESS_EVALUATION_SHADER,
	GL_COMPUTE_SHADER
};

GLShader::GLShader()
	: RShader()
{
	_shaders[0] = -1;
	_shaders[1] = -1;
	_shaders[2] = -1;
	_shaders[3] = -1;
	_shaders[4] = -1;
	_shaders[5] = -1;

	for (int i = 0; i < 10; ++i)
		_vsBuffers[i].index = -1;

	for (int i = 0; i < 10; ++i)
		_fsBuffers[i].index = -1;

	_nextBinding = 0;
	_program = 0;
	_vsNumBuffers = 0;
	_fsNumBuffers = 0;
}

void GLShader::BindUniformBuffers()
{
	for (int i = 0; i < 10; ++i)
		if (_vsBuffers[i].index != -1)
			_vsBuffers[i].ubo->BindUniform(_vsBuffers[i].binding, _vsBuffers[i].offset, _vsBuffers[i].size);

	for (int i = 0; i < 10; ++i)
		if (_fsBuffers[i].index != -1)
			_fsBuffers[i].ubo->BindUniform(_fsBuffers[i].binding, _fsBuffers[i].offset, _fsBuffers[i].size);
}

void GLShader::VSUniformBlockBinding(int location, const char *name)
{
	_vsBuffers[location].binding = _nextBinding++;
	GL_CHECK(_vsBuffers[location].index = glGetUniformBlockIndex(_program, name));
	GL_CHECK(glUniformBlockBinding(_program, _vsBuffers[location].index, _vsBuffers[location].binding));
}

void GLShader::FSUniformBlockBinding(int location, const char *name)
{
	_fsBuffers[location].binding = _nextBinding++;
	GL_CHECK(_fsBuffers[location].index = glGetUniformBlockIndex(_program, name));
	GL_CHECK(glUniformBlockBinding(_program, _fsBuffers[location].index, _fsBuffers[location].binding));
}

void GLShader::VSSetUniformBuffer(int location, uint64_t offset, uint64_t size, RBuffer *buf)
{
	_vsBuffers[location].offset = offset;
	_vsBuffers[location].size = size;
	_vsBuffers[location].ubo = (GLBuffer*)buf;
}

void GLShader::FSSetUniformBuffer(int location, uint64_t offset, uint64_t size, RBuffer *buf)
{
	_fsBuffers[location].offset = offset;
	_fsBuffers[location].size = size;
	_fsBuffers[location].ubo = (GLBuffer*)buf;
}

void GLShader::SetTexture(unsigned int location, RTexture *tex)
{
	GLTexture *glTex = (GLTexture *)tex;

	if (!glTex->IsResident())
		glTex->MakeResident();

	GLuint64 handle = glTex->GetHandle();

	GL_CHECK(glProgramUniform2uiv(_program, location, 1, (const GLuint *)&handle));
}

void GLShader::SetSubroutines(ShaderType type, int count, const unsigned int *indices)
{
	GL_CHECK(glUniformSubroutinesuiv(GL_ShaderType[(int)type], count, indices));
}

bool GLShader::LoadFromSource(ShaderType type, int count, const char **source, int *length)
{
	GLint compiled;

	int srcCount = count + 2;
	const char **src = (const char **)calloc(srcCount, sizeof(const char *));

	if (!src)
		return false;

	src[0] = "#version 450 core\n\
	#extension GL_ARB_bindless_texture : require\n\
	#define TEXTURE_2D uvec2\n\
	#define GET_TEX_2D(x) sampler2D(x) \n\
	#define TEXTURE_3D uvec2\n\
	#define GET_TEX_3D(x) sampler3D(x) \n\
	#define TEXTURE_RECT uvec2\n\
	#define GET_TEX_RECT(x) sampler2DRect(x) \n\
	#define TEXTURE_2D_ARRAY uvec2\n\
	#define GET_TEX_2D_ARRAY(x) sampler2DArray(x) \n\
	#define TEXTURE_CUBE uvec2\n\
	#define GET_TEX_CUBE(x) samplerCube(x) \n\
	#define TEXTURE_CUBE_ARRAY uvec2\n\
	#define GET_TEX_CUBE_ARRAY(x) samplerCubeArray(x) \n\
	#define TEXTURE_BUFFER uvec2\n\
	#define GET_TEX_BUFFER(x) samplerBuffer(x) \n\
	#define TEXTURE_2DMS uvec2\n\
	#define GET_TEX_2DMS(x) sampler2DMS(x) \n\
	#define TEXTURE_2DMS_ARRAY uvec2\n\
	#define GET_TEX_2DMS_ARRAY(x) sampler2DMSArray(x) \n\
	#define TEXTURE_2D_SH uvec2\n\
	#define GET_TEX_2D_SH(x) sampler2DShadow(x) \n\
	#define TEXTURE_CUBE_SH uvec2\n\
	#define GET_TEX_CUBE_SH(x) samplerCubeShadow(x) \n\
	#define TEXTURE_RECT_SH uvec2\n\
	#define GET_TEX_RECT_SH(x) sampler2DRectShadow(x) \n\
	#define TEXTURE_2D_ARRAY_SH uvec2\n\
	#define GET_TEX_2D_ARRAY_SH(x) sampler2DArrayShadow(x) \n\
	#define TEXTURE_CUBE_ARRAY_SH uvec2\n\
	#define GET_TEX_CUBE_ARRAY_SH(x) samplerCubeArrayShadow(x) \n\
	#define HAVE_SUBROUTINES\n\
	#define SUBROUTINE_DELEGATE(x) subroutine void x();\n\
	#define SUBROUTINE(x, y, z) layout(location = x) subroutine uniform y z;\n\
	#define SUBROUTINE_FUNC(x, y) layout(index = x) subroutine(y) \n";
	
	char defines[8192] { 0 };
	
	for (ShaderDefine &define : GLRenderer::GetShaderDefines())
	{
		int len = strlen(defines);

		if (len >= 8190)
		{
			free(src);
			return false;
		}

		if (snprintf(defines + len, 8192, "#define %s %s\n", define.name.c_str(), define.value.c_str()) >= 8192)
		{
			free(src);
			return false;
		}
	}

	defines[strlen(defines)] = 0x0;
	
	src[1] = defines;

	for (int i = 2; i < srcCount; i++)
		src[i] = source[i - 2];

	GL_CHECK(_shaders[(int)type] = glCreateShader(GL_ShaderType[(int)type]));
	GL_CHECK(glShaderSource(_shaders[(int)type], srcCount, src, length));
	glCompileShader(_shaders[(int)type]);
	GL_CHECK(glGetShaderiv(_shaders[(int)type], GL_COMPILE_STATUS, &compiled));

	free(src);

	if (!compiled)
	{
		GLint infoLen = 0;

		GL_CHECK(glGetShaderiv(_shaders[(int)type], GL_INFO_LOG_LENGTH, &infoLen));

		if (infoLen > 1)
		{
			char* infoLog = (char *)calloc(infoLen, sizeof(char));

			GL_CHECK(glGetShaderInfoLog(_shaders[(int)type], infoLen, NULL, infoLog));

#ifdef _WIN32
			OutputDebugStringA(infoLog);
			OutputDebugStringA("\n");
#else
			printf("%s\n", infoLog);
#endif

			free(infoLog);
			infoLog = nullptr;
		}

		GL_CHECK(glDeleteShader(_shaders[(int)type]));
		return false;
	}

	return true;
}

bool GLShader::LoadFromStageBinary(ShaderType type, const char *file)
{
	return false;
}

bool GLShader::LoadFromBinary(const char *file)
{
	return false;
}

bool GLShader::Link()
{
	GLint linked;

	GL_CHECK(_program = glCreateProgram());

	for (int i = 0; i < 6; i++)
	{
		if (_shaders[i] != -1)
		{ GL_CHECK(glAttachShader(_program, _shaders[i])); }
	}

	GL_CHECK(glLinkProgram(_program));
	GL_CHECK(glGetProgramiv(_program, GL_LINK_STATUS, &linked));

	for (int i = 0; i < 6; i++)
	{
		if (_shaders[i] != -1)
		{ GL_CHECK(glDeleteShader(_shaders[i])); }
		_shaders[i] = -1;
	}

	if (!linked)
	{
		GLint infoLen = 0;

		GL_CHECK(glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infoLen));

		if (infoLen > 1)
		{
			char* infoLog = (char *)calloc(infoLen, sizeof(char));

			GL_CHECK(glGetProgramInfoLog(_program, infoLen, NULL, infoLog));

#ifdef _WIN32
			OutputDebugStringA(infoLog);
			OutputDebugStringA("\n");
#else
			printf("%s\n", infoLog);
#endif

			free(infoLog);
			infoLog = nullptr;
		}
		
		GL_CHECK(glDeleteProgram(_program));
		return false;
	}

	return true;
}

GLShader::~GLShader()
{
	for (int i = 0; i < 6; i++)
	{
		if (_shaders[i] != -1)
		{ GL_CHECK(glDeleteShader(_shaders[i])); }
		_shaders[i] = -1;
	}

	GL_CHECK(glDeleteProgram(_program));
}
