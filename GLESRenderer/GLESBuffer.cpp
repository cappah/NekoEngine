/* NekoEngine
 *
 * GLESBuffer.cpp
 * Author: Alexandru Naiman
 *
 * OpenGL|ES 3 Renderer Implementation
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

#include "GLESBuffer.h"
#include "GLESRenderer.h"

#include <Platform/Platform.h>

#ifdef NE_PLATFORM_IOS
#define GL_DOUBLE 0
#endif

GLenum GL_BufferTargets[3]
{
    GL_ARRAY_BUFFER,
    GL_ELEMENT_ARRAY_BUFFER,
    GL_UNIFORM_BUFFER
};

GLenum GL_GetBufferTargets[3]
{
	GL_ARRAY_BUFFER_BINDING,
	GL_ELEMENT_ARRAY_BUFFER_BINDING,
	GL_UNIFORM_BUFFER_BINDING
};

GLenum GL_AttribTypes[10]
{
    GL_BYTE,
    GL_UNSIGNED_BYTE,
    GL_SHORT,
    GL_UNSIGNED_SHORT,
    GL_INT,
    GL_UNSIGNED_INT,
    GL_HALF_FLOAT,
    GL_FLOAT,
    GL_DOUBLE,
    GL_FIXED
};

GLESBuffer::GLESBuffer(BufferType type, bool dynamic, bool persistent) : RBuffer(type)
{
    _dynamic = (dynamic || persistent);
	_persistent = false;
    _numBuffers = 1;
    _syncRanges = nullptr;
    _target = GL_BufferTargets[(int)type];

    GL_CHECK(glGenBuffers(1, &_id));
}

void GLESBuffer::Bind(int location)
{
	_target = GL_BufferTargets[location];
	
    GL_CHECK(glBindBuffer(_target, _id));
    
    if (_type == BufferType::Vertex)
    {
        for (BufferAttribute &attrib : _attributes)
        {
            GL_CHECK(glEnableVertexAttribArray(attrib.index));

			switch (attrib.type)
			{
				case BufferDataType::Int:
				case BufferDataType::Short:
				case BufferDataType::Byte:
				case BufferDataType::UnsignedInt:
				case BufferDataType::UnsignedShort:
				case BufferDataType::UnsignedByte:
				{
					GL_CHECK(glVertexAttribIPointer(attrib.index,
						attrib.size,
						GL_AttribTypes[(int)attrib.type],
						(GLsizei)attrib.stride,
						(GLsizei *)attrib.ptr));
				}
				break;
				default:
				{
					GL_CHECK(glVertexAttribPointer(attrib.index,
						attrib.size,
						GL_AttribTypes[(int)attrib.type],
						attrib.normalize ? GL_TRUE : GL_FALSE,
						(GLsizei)attrib.stride,
						(GLsizei *)attrib.ptr));
				}
				break;
			}
        }
    }
}

void GLESBuffer::Unbind()
{
    GL_CHECK(glBindBuffer(_target, 0));
}

uint8_t* GLESBuffer::GetData()
{
    // MacOS does not support GL_ARB_buffer_storage, required for persistent buffers
    return nullptr;
}

int GLESBuffer::GetCurrentBuffer()
{
    // MacOS does not support GL_ARB_buffer_storage, required for persistent buffers
    return 0;
}

uint64_t GLESBuffer::GetOffset()
{
    // MacOS does not support GL_ARB_buffer_storage, required for persistent buffers
    return 0;
}

void GLESBuffer::SetStorage(size_t size, void* data)
{
    int flags = _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    _size = size;
	
	GLint buff;
	GL_CHECK(glGetIntegerv(GL_GetBufferTargets[(int)_type], &buff));
	
    GL_CHECK(glBindBuffer(_target, _id));
    GL_CHECK(glBufferData(_target, size, data, flags));
	GL_CHECK(glBindBuffer(_target, buff));
}

void GLESBuffer::UpdateData(size_t offset, size_t size, void* data)
{
	if(!_dynamic)
		return;
	
	GLint buff;
	GL_CHECK(glGetIntegerv(GL_GetBufferTargets[(int)_type], &buff));
	
    GL_CHECK(glBindBuffer(_target, _id));
	GL_CHECK(glBufferSubData(_target, offset, size, data));
	GL_CHECK(glBindBuffer(_target, buff));
}

void GLESBuffer::SetNumBuffers(int n)
{
    // MacOS does not support GL_ARB_buffer_storage, required for persistent buffers
}

void GLESBuffer::BeginUpdate()
{
    // MacOS does not support GL_ARB_buffer_storage, required for persistent buffers
}

void GLESBuffer::EndUpdate()
{
    // MacOS does not support GL_ARB_buffer_storage, required for persistent buffers
}

void GLESBuffer::NextBuffer()
{
    // MacOS does not support GL_ARB_buffer_storage, required for persistent buffers
}

void GLESBuffer::BindUniform(int index, uint64_t offset, uint64_t size)
{
    GL_CHECK(glBindBufferRange(_target, index, _id, (GLintptr)offset, (GLintptr)size));
}

GLESBuffer::~GLESBuffer()
{
    GL_CHECK(glDeleteBuffers(1, &_id));
}
