/* Neko Engine
 *
 * RShader.h
 * Author: Alexandru Naiman
 *
 * Rendering API abstraction
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

#include <stdint.h>
#include <string>
#include <vector>

#include <Renderer/RTexture.h>
#include <Renderer/RBuffer.h>

enum class ShaderType : uint8_t
{
	Vertex = 0,
	Fragment = 1,
	Geometry = 2,
	TesselationControl = 3,
	TesselationEval = 4,
	Compute = 5
};

class RShader
{

public:
	
	/**
	 * Create a shader object
	 */
	RShader() { };

	virtual void Enable() = 0;
	virtual void Disable() = 0;

	/**
	 * Set a texture sampler at the specified location
	 */
	virtual void SetTexture(unsigned int location, RTexture* tex) = 0;

	virtual void BindUniformBuffers() = 0;

	virtual void VSUniformBlockBinding(int location, const char *name) = 0;
	virtual void FSUniformBlockBinding(int location, const char *name) = 0;

	/**
	 * Set vertex shader uniform buffer
	 */
	virtual void VSSetUniformBuffer(int location, uint64_t offset, uint64_t size, RBuffer *buf) = 0;

	/**
	 * Set fragment shader uniform buffer
	 */
	virtual void FSSetUniformBuffer(int location, uint64_t offset, uint64_t size, RBuffer *buf) = 0;

	/**
	 * Set the active shader subroutines
	 */
	virtual void SetSubroutines(ShaderType type, int count, const uint32_t* indices) = 0;

	/**
	 * Compile program source for the specified type.
	 */
	virtual bool LoadFromSource(ShaderType type, int count, const char** source, int* length) = 0;

	/**
	 * Load shader from shader stage binary
	 */
	virtual bool LoadFromStageBinary(ShaderType type, const char* file) = 0;

	/**
	 * Load shader from binary
	 */
	virtual bool LoadFromBinary(const char* file) = 0;
	
	/**
	* Link the shader program
	*/
	virtual bool Link() = 0;

	/**
	 * Release resources
	 */
	virtual ~RShader() { };

protected:
};

