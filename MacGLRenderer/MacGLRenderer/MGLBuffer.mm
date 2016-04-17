/* Neko Engine
 *
 * MGLBuffer.cpp
 * Author: Alexandru Naiman
 *
 * MacOS X OpenGL Renderer Implementation
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

#include "MGLBuffer.h"
#include "MGLRenderer.h"

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

MGLBuffer::MGLBuffer(BufferType type, bool dynamic, bool persistent) : RBuffer(type)
{
    _dynamic = (dynamic || persistent);
	_persistent = false;
    _numBuffers = 1;
    _syncRanges = nullptr;
    _target = GL_BufferTargets[(int)type];

    GL_CHECK(glGenBuffers(1, &_id));
}

void MGLBuffer::Bind(int location)
{
	_target = GL_BufferTargets[location];
	
    GL_CHECK(glBindBuffer(_target, _id));
    
    if (_type == BufferType::Vertex)
    {
        for (BufferAttribute &attrib : _attributes)
        {
            GL_CHECK(glEnableVertexAttribArray(attrib.index));
            GL_CHECK(glVertexAttribPointer(attrib.index,
                                           attrib.size,
                                           GL_AttribTypes[(int)attrib.type],
                                           attrib.normalize ? GL_TRUE : GL_FALSE,
                                           (GLsizei)attrib.stride,
                                           (GLsizei *)attrib.ptr));
        }
    }
}

void MGLBuffer::Unbind()
{
    GL_CHECK(glBindBuffer(_target, 0));
}

uint8_t* MGLBuffer::GetData()
{
    // MacOS does not support GL_ARB_buffer_storage, required for persistent buffers
    return nullptr;
}

int MGLBuffer::GetCurrentBuffer()
{
    // MacOS does not support GL_ARB_buffer_storage, required for persistent buffers
    return 0;
}

uint64_t MGLBuffer::GetOffset()
{
    // MacOS does not support GL_ARB_buffer_storage, required for persistent buffers
    return 0;
}

void MGLBuffer::SetStorage(size_t size, void* data)
{
    int flags = _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    _size = size;
	
	GLint buff;
	GL_CHECK(glGetIntegerv(GL_GetBufferTargets[(int)_type], &buff));
	
    GL_CHECK(glBindBuffer(_target, _id));
    GL_CHECK(glBufferData(_target, size, data, flags));
	GL_CHECK(glBindBuffer(_target, buff));
}

void MGLBuffer::UpdateData(size_t offset, size_t size, void* data)
{
	if(!_dynamic)
		return;
	
	GLint buff;
	GL_CHECK(glGetIntegerv(GL_GetBufferTargets[(int)_type], &buff));
	
    GL_CHECK(glBindBuffer(_target, _id));
	GL_CHECK(glBufferSubData(_target, offset, size, data));
	GL_CHECK(glBindBuffer(_target, buff));
}

void MGLBuffer::SetNumBuffers(int n)
{
    // MacOS does not support GL_ARB_buffer_storage, required for persistent buffers
}

void MGLBuffer::BeginUpdate()
{
    // MacOS does not support GL_ARB_buffer_storage, required for persistent buffers
}

void MGLBuffer::EndUpdate()
{
    // MacOS does not support GL_ARB_buffer_storage, required for persistent buffers
}

void MGLBuffer::NextBuffer()
{
    // MacOS does not support GL_ARB_buffer_storage, required for persistent buffers
}

void MGLBuffer::BindUniform(int index, uint64_t offset, uint64_t size)
{
    GL_CHECK(glBindBufferRange(_target, index, _id, offset, size));
}

MGLBuffer::~MGLBuffer()
{
    GL_CHECK(glDeleteBuffers(1, &_id));
}
