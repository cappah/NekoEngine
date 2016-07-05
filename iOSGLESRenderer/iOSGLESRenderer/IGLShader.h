/* Neko Engine
 *
 * IGLShader.h
 * Author: Alexandru Naiman
 *
 * iOS OpenGL|ES Renderer Implementation
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

#ifndef IGLShader_h
#define IGLShader_h

#include <Renderer/RShader.h>
#include <OpenGLES/ES3/gl.h>
#include <unordered_map>
#include <vector>

#include "IGLBuffer.h"

typedef struct GL_UNIF_BUF
{
    IGLBuffer *ubo;
    size_t offset;
    size_t size;
    uint32_t binding;
    int32_t index;
} GLUniformBuffer;

typedef struct UNIFORM_INFO
{
	std::string location;
	std::string name;
} UniformInfo;

class IGLShader : public RShader
{
public:
    IGLShader();
    
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
    
    virtual ~IGLShader();
	
	void EnableTextures();
	bool Validate();
    
private:
    GLint _program;
    GLint _shaders[6];
    GLUniformBuffer _vsBuffers[10];
    GLUniformBuffer _fsBuffers[10];
    uint8_t _vsNumBuffers;
    uint8_t _fsNumBuffers;
    uint8_t _nextBinding;
	std::vector<UniformInfo> _uniformInfo;
	std::unordered_map<unsigned int, class IGLTexture*> _textures;
	std::unordered_map<int, int> _uniformLocations;
	
	char* _ExtractUniforms(const char* source);
};

#endif /* IGLShader_h */
