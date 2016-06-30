/* Neko Engine
 *
 * GLShader.h
 * Author: Alexandru Naiman
 *
 * OpenGL Renderer Implementation
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

#include <Renderer/RShader.h>

#include <unordered_map>

#include "GLBuffer.h"

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	#include <OpenGL/gl3ext.h>
#else
	#include "glad.h"
#endif

typedef struct GL_UNIF_BUF
{
	GLBuffer *ubo;
	size_t offset;
	size_t size;
	uint32_t binding;
	uint32_t index;
} GLUniformBuffer;

typedef struct UNIFORM_INFO
{
	std::string location;
	std::string name;
} UniformInfo;

class GLShader :
	public RShader
{
public:
	GLShader();

	virtual void Enable() override;
	virtual void Disable() override;

	virtual void BindUniformBuffers() override;

	virtual void VSUniformBlockBinding(int location, const char *name) override;
	virtual void FSUniformBlockBinding(int location, const char *name) override;
	virtual void VSSetUniformBuffer(int location, uint64_t offset, uint64_t size, RBuffer *buf) override;
	virtual void FSSetUniformBuffer(int location, uint64_t offset, uint64_t size, RBuffer *buf) override;

	virtual void SetTexture(unsigned int location, RTexture *tex) override;

	virtual void SetSubroutines(ShaderType type, int count, const unsigned int *indices) override;

	virtual bool LoadFromSource(ShaderType type, int count, const char **source, int *length) override;
	virtual bool LoadFromStageBinary(ShaderType type, const char *file) override;
	virtual bool LoadFromBinary(const char *file) override;

	virtual bool Link() override;
	
	void EnableTextures();

	virtual ~GLShader();

private:
	GLint _program;
	GLint _shaders[6];
	GLUniformBuffer _vsBuffers[10];
	GLUniformBuffer _fsBuffers[10];
	uint8_t _vsNumBuffers;
	uint8_t _fsNumBuffers;
	uint8_t _nextBinding;
	bool _haveExplicitUniforms, _haveBindlessTexture, _haveSubroutines;
	std::vector<UniformInfo> _uniformInfo;
	std::unordered_map<unsigned int, class GLTexture*> _textures;
	std::unordered_map<int, int> _uniformLocations;
	
	char *_ExtractUniforms(const char *shaderSource);
};
