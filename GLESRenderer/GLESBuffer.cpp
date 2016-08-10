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

#include <string.h>

#include <Platform/Platform.h>

#if defined(NE_PLATFORM_IOS) || defined(NE_PLATFORM_BB10)
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
	_haveBufferStorage = GLESRenderer::HasExtension("GL_EXT_buffer_storage");
	if (_haveBufferStorage)
	{
		_dynamic = dynamic;
		_persistent = persistent;
	}
	else
	{
		_dynamic = (dynamic || persistent);
		_persistent = false;
	}

    _numBuffers = 1;
    _syncRanges = nullptr;
    _target = GL_BufferTargets[(int)type];
    _totalSize = 0;
    _size = 0;
    _currentBuffer = 0;

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

uint8_t *GLESBuffer::GetData()
{
	if (!_persistent)
		return nullptr;

	return (_data + _size * _currentBuffer);
}

int GLESBuffer::GetCurrentBuffer()
{
	return _currentBuffer;
}

uint64_t GLESBuffer::GetOffset()
{
    return _syncRanges[_currentBuffer].offset;
}

void GLESBuffer::SetStorage(size_t size, void* data)
{
	int flags = 0;

	_size = size;

	GLint buff;
	GL_CHECK(glGetIntegerv(GL_GetBufferTargets[(int)_type], &buff));
	GL_CHECK(glBindBuffer(_target, _id));

	if (_haveBufferStorage)
	{
		if (_persistent)
			flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT;
		else if (_dynamic)
			flags = GL_DYNAMIC_STORAGE_BIT_EXT;

		if (!_persistent)
		{
			_totalSize = size;
			GL_CHECK(glBufferStorageEXT(_target, _size, data, flags));
			return;
		}

		if (!_syncRanges)
			SetNumBuffers(_numBuffers);

		for (int i = 0; i < _numBuffers; i++)
		{
			_syncRanges[i].offset = _size * i;
			_syncRanges[i].sync = 0;
		}

		_totalSize = _size * _numBuffers;
		GL_CHECK(glBufferStorageEXT(_target, _totalSize, data, flags));
		GL_CHECK(_data = (uint8_t*)glMapBufferRange(_target, 0, _totalSize, flags));

		_currentBuffer = 0;

		GL_CHECK(glBindBuffer(_target, buff));
	}
	else
	{
		flags = _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

		GL_CHECK(glBufferData(_target, size, data, flags));
		GL_CHECK(glBindBuffer(_target, buff));
	}
}

void GLESBuffer::UpdateData(size_t offset, size_t size, void* data)
{
	if (_persistent)
		memcpy(GetData() + offset, data, size);
	else
	{
		if(!_dynamic)
			return;

		GLint buff;
		GL_CHECK(glGetIntegerv(GL_GetBufferTargets[(int)_type], &buff));

		GL_CHECK(glBindBuffer(_target, _id));
		GL_CHECK(glBufferSubData(_target, offset, size, data));
		GL_CHECK(glBindBuffer(_target, buff));
	}
}

void GLESBuffer::SetNumBuffers(int n)
{
	if (_syncRanges)
	{
		GLSyncRange *oldRanges = _syncRanges;

		_syncRanges = (GLSyncRange*)calloc(n, sizeof(GLSyncRange));
		if(!_syncRanges)
		{ DIE("Reallocation failed"); }
		memcpy(_syncRanges, oldRanges, sizeof(GLSyncRange) * n);

		free(oldRanges);
	}
	else
		_syncRanges = (GLSyncRange*)calloc(n, sizeof(GLSyncRange));

	_numBuffers = n;
}

void GLESBuffer::BeginUpdate()
{
	if (!_persistent)
		return;

	if (_syncRanges[_currentBuffer].sync)
	{
		while (true)
		{
			GL_CHECK(GLenum wait = glClientWaitSync(_syncRanges[_currentBuffer].sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1));
			if (wait == GL_ALREADY_SIGNALED || wait == GL_CONDITION_SATISFIED)
				return;
		}
	}
}

void GLESBuffer::EndUpdate()
{
	if (!_persistent)
		return;

	if (_syncRanges[_currentBuffer].sync)
	{ GL_CHECK(glDeleteSync(_syncRanges[_currentBuffer].sync)); }

	GL_CHECK(_syncRanges[_currentBuffer].sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
}

void GLESBuffer::NextBuffer()
{
	if(_persistent)
		_currentBuffer = (_currentBuffer + 1) % _numBuffers;
}

void GLESBuffer::BindUniform(int index, uint64_t offset, uint64_t size)
{
    GL_CHECK(glBindBufferRange(_target, index, _id, (GLintptr)offset, (GLintptr)size));
}

GLESBuffer::~GLESBuffer()
{
	if (_persistent)
	{
		GL_CHECK(glBindBuffer(_target, _id));
		GL_CHECK(glUnmapBuffer(_target));
		GL_CHECK(glBindBuffer(_target, 0));
	}

    GL_CHECK(glDeleteBuffers(1, &_id));

	free(_syncRanges);
	_syncRanges = nullptr;
	_persistent = false;
}
