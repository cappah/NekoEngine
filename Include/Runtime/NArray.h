/* NekoEngine
 *
 * NArray.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Runtime
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

#include <stdlib.h>
#include <stdint.h>

#include <functional>

#define NARRAY_DEFAULT_INCREMENT	20

template<class T>
class NArray
{
public:
	NArray(size_t size = 10) :
		_data(nullptr),
		_count(0),
		_size(size)
	{
		_data = (uint8_t *)realloc(nullptr, sizeof(T) * _size);
	}

	NArray(const NArray<T> &other)
	{
		_count = other._count;
		_size = other._size;
		
		for(size_t i = 0; i < _count; ++i)
			new (_data + sizeof(T) * _count++)T(((T*)other._data)[i]);
	}

	NArray(NArray<T> &&other)
	{
		_count = other._count;
		_size = other._size;
		_data = other._data;

		other._count = other._size = 0;
		other._data = nullptr;
	}

	T* begin() { return &((T*)_data)[0]; }
	T* end() { return &((T*)_data)[_count]; }

	size_t Count() { return _count; }
	size_t Size() { return _size; }

	void Add(const T &item)
	{
		if (_count == _size)
			if (!Resize(_size + NARRAY_DEFAULT_INCREMENT))
				return;

		new (_data + sizeof(T) * _count++)T(item);
	}

	void Insert(uint32_t index, T &item)
	{
		//
	}

	void Remove(uint32_t index)
	{
		if (index == --_count)
			return;

		((T*)_data)[index].~T();

		for (uint32_t i = index + 1; i <= _count; ++i)
			((T*)_data)[i - 1] = ((T*)_data)[i];
	}

	size_t Find(T item, std::function<bool(T, T)> cmpfunc = [](T a, T b) -> bool { return a == b; })
	{
		for (uint32_t i = 0; i <= _count; ++i)
			if (cmpfunc(item, ((T*)_data)[i]))
				return i;
		return NotFound;
	}

	bool Resize(size_t size)
	{
		if (_size == size)
			return true;

		T* ptr = (T*)_data;
		if ((_data = (uint8_t *)realloc(_data, sizeof(T) * size)) == nullptr)
		{
			_data = (uint8_t *)ptr;
			return false;
		}

		_size = size;

		if (_size < _count)
			_count = _size;

		return true;
	}

	void Fill()
	{
		_count = _size;
	}

	void Clear()
	{
		for (size_t i = 0; i < _count; ++i)
			((T*)_data)[i].~T();

		_count = _size = 0;

		free(_data);
		_data = nullptr;
	}

	virtual ~NArray()
	{
		Clear();
	}

	T &operator [](const size_t i) { return ((T*)_data)[i]; }
	T *operator *() { return (T*)_data; }

	NArray<T> &operator =(const NArray<T> &other)
	{
		_count = other._count;
		_size = other._size;

		Resize(other._size);

		for (size_t i = 0; i < _count; ++i)
			new (_data + sizeof(T) * i)T(((T*)other._data)[i]);

		return *this;
	}

	static constexpr size_t NotFound = -1;

protected:
	uint8_t *_data;
	size_t _count, _size;
};
