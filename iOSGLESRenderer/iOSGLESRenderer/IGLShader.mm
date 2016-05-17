/* Neko Engine
 *
 * IGLShader.mm
 * Author: Alexandru Naiman
 *
 * iOS OpenGL|ES Renderer Implementation
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

#include "IGLShader.h"
#include "IGLTexture.h"
#include "IGLRenderer.h"

#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GLenum GL_ShaderType[6] =
{
	GL_VERTEX_SHADER,
	GL_FRAGMENT_SHADER,
	0,
	0,
	0,
	0 // GL_COMPUTE_SHADER
};

static inline void str_remove(char *str, const char *toRemove)
{
	if (!(str = strstr(str, toRemove)))
		return;
	
	const size_t remLen = strlen(toRemove);
	char *copyEnd;
	char *copyFrom = str + remLen;
	while ((copyEnd = strstr(copyFrom, toRemove)) != NULL)
	{
		memmove(str, copyFrom, copyEnd - copyFrom);
		str += copyEnd - copyFrom;
		copyFrom = copyEnd + remLen;
	}
	memmove(str, copyFrom, 1 + strlen(copyFrom));
}

IGLShader::IGLShader()
: RShader()
{
	_shaders[0] = -1;
	_shaders[1] = -1;
	_shaders[2] = -1;
	_shaders[3] = -1;
	_shaders[4] = -1;
	_shaders[5] = -1;
	
	memset(_vsBuffers, 0x0, sizeof(_vsBuffers));
	memset(_fsBuffers, 0x0, sizeof(_fsBuffers));
	
	for (int i = 0; i < 10; ++i)
		_vsBuffers[i].index = GL_INVALID_INDEX;
	
	for (int i = 0; i < 10; ++i)
		_fsBuffers[i].index = GL_INVALID_INDEX;
	
	_nextBinding = 0;
}

void IGLShader::Enable()
{	
	GL_CHECK(glUseProgram(_program));
	IGLRenderer::SetActiveShader(this);
}

void IGLShader::Disable()
{
	GL_CHECK(glUseProgram(0));
	IGLRenderer::SetActiveShader(nullptr);
}

void IGLShader::BindUniformBuffers()
{
	for (int i = 0; i < 10; ++i)
		if ((_vsBuffers[i].index != GL_INVALID_INDEX)) && _vsBuffers[i].ubo)
			_vsBuffers[i].ubo->BindUniform(_vsBuffers[i].binding, _vsBuffers[i].offset, _vsBuffers[i].size);
	
	for (int i = 0; i < 10; ++i)
		if ((_fsBuffers[i].index != GL_INVALID_INDEX)) && _fsBuffers[i].ubo)
			_fsBuffers[i].ubo->BindUniform(_fsBuffers[i].binding, _fsBuffers[i].offset, _fsBuffers[i].size);
}

void IGLShader::VSUniformBlockBinding(int location, const char *name)
{
	_vsBuffers[location].binding = _nextBinding++;
	GL_CHECK(_vsBuffers[location].index = glGetUniformBlockIndex(_program, name));
	
#ifdef _DEBUG
	printf("VSUniformBlockBinding: name: %s location: %d index: %d binding: %d\n", name, location, _vsBuffers[location].index, _vsBuffers[location].binding);
#endif
	
	if(_vsBuffers[location].index == GL_INVALID_INDEX)
		return;
	
	GL_CHECK(glUniformBlockBinding(_program, _vsBuffers[location].index, _vsBuffers[location].binding));
}

void IGLShader::FSUniformBlockBinding(int location, const char *name)
{
	_fsBuffers[location].binding = _nextBinding++;
	GL_CHECK(_fsBuffers[location].index = glGetUniformBlockIndex(_program, name));
	
#ifdef _DEBUG
	printf("FSUniformBlockBinding: name: %s location: %d index: %d binding: %d\n", name, location, _fsBuffers[location].index, _fsBuffers[location].binding);
#endif
	
	if(_fsBuffers[location].index == GL_INVALID_INDEX)
		return;
	
	GL_CHECK(glUniformBlockBinding(_program, _fsBuffers[location].index, _fsBuffers[location].binding));
}

void IGLShader::VSSetUniformBuffer(int location, uint64_t offset, uint64_t size, RBuffer *buf)
{
	_vsBuffers[location].offset = offset;
	_vsBuffers[location].size = size;
	_vsBuffers[location].ubo = (IGLBuffer*)buf;
}

void IGLShader::FSSetUniformBuffer(int location, uint64_t offset, uint64_t size, RBuffer *buf)
{
	_fsBuffers[location].offset = offset;
	_fsBuffers[location].size = size;
	_fsBuffers[location].ubo = (IGLBuffer*)buf;
}

void IGLShader::SetTexture(unsigned int location, RTexture *tex)
{
	_textures[location] = (IGLTexture*)tex;
}

void IGLShader::SetSubroutines(ShaderType type, int count, const unsigned int *indices)
{
	//GL_CHECK(glUniformSubroutinesuiv(GL_ShaderType[(int)type], count, indices));
}

bool IGLShader::LoadFromSource(ShaderType type, int count, const char **source, int *length)
{
	GLint compiled;
	
	int srcCount = count + 2;
	const char **src = (const char **)calloc(srcCount, sizeof(const char *));
	
	if (!src)
		return false;
	
	src[0] = "#version 300 es\n\
	precision highp float;\n\
	#define TEXTURE_2D sampler2D\n\
	#define GET_TEX_2D(x) x \n\
	#define TEXTURE_3D sampler3D\n\
	#define GET_TEX_3D(x) GET_TEX_2D(x) \n\
	#define TEXTURE_RECT sampler2DRect\n\
	#define GET_TEX_RECT(x) GET_TEX_2D(x) \n\
	#define TEXTURE_2D_ARRAY sampler2DArray\n\
	#define GET_TEX_2D_ARRAY(x) GET_TEX_2D(x) \n\
	#define TEXTURE_CUBE samplerCube\n\
	#define GET_TEX_CUBE(x) GET_TEX_2D(x) \n\
	#define TEXTURE_CUBE_ARRAY samplerCubeArray\n\
	#define GET_TEX_CUBE_ARRAY(x) GET_TEX_2D(x) \n\
	#define TEXTURE_BUFFER samplerBuffer\n\
	#define GET_TEX_BUFFER(x) GET_TEX_2D(x) \n\
	#define TEXTURE_2DMS sampler2D\n\
	#define GET_TEX_2DMS(x) GET_TEX_2D(x) \n\
	#define TEXTURE_2DMS_ARRAY sampler2DArray\n\
	#define GET_TEX_2DMS_ARRAY(x) GET_TEX_2D(x) \n\
	#define TEXTURE_2D_SH sampler2DShadow\n\
	#define GET_TEX_2D_SH(x) GET_TEX_2D(x) \n\
	#define TEXTURE_CUBE_SH samplerCubeShadow\n\
	#define GET_TEX_CUBE_SH(x) GET_TEX_2D(x) \n\
	#define TEXTURE_RECT_SH sampler2DRectShadow\n\
	#define GET_TEX_RECT_SH(x) GET_TEX_2D(x) \n\
	#define TEXTURE_2D_ARRAY_SH sampler2DArrayShadow\n\
	#define GET_TEX_2D_ARRAY_SH(x) GET_TEX_2D(x) \n\
	#define TEXTURE_CUBE_ARRAY_SH samplerCubeArrayShadow\n\
	#define GET_TEX_CUBE_ARRAY_SH(x) GET_TEX_2D(x) \n\
	#define SUBROUTINE_DELEGATE(x) \n\
	#define SUBROUTINE(x, y, z) \n\
	#define SUBROUTINE_FUNC(x, y) \n\
	#define gl_SampleID 0\n";
	
	char defines[8192] { 0 };
	
	for (ShaderDefine &define : IGLRenderer::GetShaderDefines())
    {
		if (snprintf(defines + strlen(defines), 8192, "#define %s %s\n", define.name.c_str(), define.value.c_str()) >= 8192)
        {
            for (int i = 2; i < srcCount; i++)
                free((void*)src[i]);
            
            free(src);
            
			return false;
        }
    }
	defines[strlen(defines)] = 0x0;
	
	src[1] = defines;
	
	for (int i = 2; i < srcCount; i++)
		src[i] = _ExtractUniforms(source[i - 2]);
	
	GL_CHECK(_shaders[(int)type] = glCreateShader(GL_ShaderType[(int)type]));
	GL_CHECK(glShaderSource(_shaders[(int)type], srcCount, src, length));
	GL_CHECK(glCompileShader(_shaders[(int)type]));
	GL_CHECK(glGetShaderiv(_shaders[(int)type], GL_COMPILE_STATUS, &compiled));

	for (int i = 2; i < srcCount; i++)
		free((void*)src[i]);
	
	free(src);
	
	if (!compiled)
	{
		GLint infoLen = 0;
		
		GL_CHECK(glGetShaderiv(_shaders[(int)type], GL_INFO_LOG_LENGTH, &infoLen));
		
		if (infoLen > 1)
		{
			char* infoLog = (char *)calloc(infoLen, sizeof(char));
			
			GL_CHECK(glGetShaderInfoLog(_shaders[(int)type], infoLen, NULL, infoLog));
			
			fprintf(stderr, "%s\n", infoLog);
			
			free(infoLog);
			infoLog = nullptr;
		}
		
		GL_CHECK(glDeleteShader(_shaders[(int)type]));
		return false;
	}
	
	return true;
}

bool IGLShader::LoadFromStageBinary(ShaderType type, const char *file)
{
	return false;
}

bool IGLShader::LoadFromBinary(const char *file)
{
	return false;
}

bool IGLShader::Link()
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
			GL_CHECK(glDeleteShader(_shaders[i]));
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
			
			fprintf(stderr, "%s\n", infoLog);
			
			free(infoLog);
			infoLog = nullptr;
		}
		
		GL_CHECK(glDeleteProgram(_program));
		
		return false;
	}
	
	for(UniformInfo &info : _uniformInfo)
	{
		char *ptr = NULL;
		int location = (int)strtol(info.location.c_str(), &ptr, 10);
		
		if(ptr)
		{
			for (ShaderDefine &define : IGLRenderer::GetShaderDefines())
			{
				if(!info.location.compare(define.name))
					location = atoi(define.value.c_str());
			}
		}
		
		_uniformLocations[location] = glGetUniformLocation(_program, info.name.c_str());
		glProgramUniform1iEXT(_program, _uniformLocations[location], location);
	}
	
	return true;
}

void IGLShader::EnableTextures()
{
	int i = 0;
	
	for (std::pair<unsigned int, IGLTexture*> kvp : _textures)
	{
		GL_CHECK(glActiveTexture(GL_TEXTURE0+i));
		kvp.second->Bind();
		GL_CHECK(glUniform1i(_uniformLocations[kvp.first], i));
		++i;
	}
}

IGLShader::~IGLShader()
{
	for (int i = 0; i < 6; i++)
	{
		if (_shaders[i] != -1)
			GL_CHECK(glDeleteShader(_shaders[i]));
		_shaders[i] = -1;
	}
	
	GL_CHECK(glDeleteProgram(_program));
}

char* IGLShader::_ExtractUniforms(const char *shaderSource)
{
	std::vector<std::string> replace;
	
	char *source = (char*)calloc(1, strlen(shaderSource) + 1);
	snprintf(source, strlen(shaderSource) + 1, "%s", shaderSource);
	
	char *ptr = strstr(source, "layout(location");
	char buff[1024], rep_buff[1024];
	
	while(ptr)
	{
		UniformInfo info;
		char *end = strchr(ptr, ';');
		size_t len = end - ptr + 1;
		memcpy(buff, ptr, len);
		buff[len] = 0x0;
		
		ptr += len;
		ptr = strstr(ptr, "layout(location");
		
		if(!strstr(buff, "uniform"))
			continue;
		
		char *pstart = strchr(buff, '(');
		char *pend = strchr(pstart, ')');
		
		snprintf(rep_buff, pend - buff + 3, "%s", buff);
		replace.push_back(rep_buff);
		
		pstart = strchr(pstart, '=');
		snprintf(rep_buff, pend - pstart, "%s", pstart+1);
		info.location = rep_buff;
		
		pstart = strrchr(pstart, ' ');
		pend = strchr(pstart, ';');
		snprintf(rep_buff, pend - pstart, "%s", pstart+1);
		info.name = rep_buff;
		
		_uniformInfo.push_back(info);
	}
	
	for(std::string &s : replace)
		str_remove(source, s.c_str());
	
	return source;
}

bool IGLShader::Validate()
{
#ifndef _DEBUG
	return true;
#else
	
	/*GLint valid;
	
	GL_CHECK(glValidateProgram(_program));
	GL_CHECK(glGetProgramiv(_program, GL_VALIDATE_STATUS, &valid));
	
	if (!valid)
	{
		GLint infoLen = 0;
		
		GL_CHECK(glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infoLen));
		
		if (infoLen > 1)
		{
			char* infoLog = (char *)calloc(infoLen, sizeof(char));
			
			GL_CHECK(glGetProgramInfoLog(_program, infoLen, NULL, infoLog));
			
			fprintf(stderr, "Program %d validation failed:\n%s\n", _program, infoLog);
			
			free(infoLog);
			infoLog = nullptr;
		}
		else
			return true;
		
		return false;
	}*/
	
	return true;
#endif
}