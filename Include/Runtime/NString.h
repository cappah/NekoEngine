/* NekoEngine
 *
 * NString.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Runtime
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2017, Alexandru Naiman
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

#include <string>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#include <Platform/Compat.h>
#include <Runtime/NArray.h>
#include <Engine/Defs.h>

class ENGINE_API NString
{
public:
	NString() :
		_str(nullptr),
		_length(0),
		_size(0)
	{ }

	NString(size_t size) :
		_length(0),
		_size(size)
	{
		_str = (char *)reallocarray(nullptr, _size, sizeof(char));
		memset(_str, 0x0, _size);
	}

	NString(size_t length, const char *str) :
		_length(length),
		_size(length + 1)
	{
		_str = (char *)reallocarray(nullptr, _size, sizeof(char));
		memmove(_str, str, _length);
		*(_str + _length) = 0x0;
	}

	NString(const char *str)
	{
		_length = strlen(str);
		_size = _length + 1;
		_str = (char *)reallocarray(nullptr, _size, sizeof(char));
		memmove(_str, str, _length);
		*(_str + _length) = 0x0;
	}

	NString(std::string &str)
	{
		_length = str.length();
		_size = _length + 1;
		_str = (char *)reallocarray(nullptr, _size, sizeof(char));
		memmove(_str, str.c_str(), _length);
		*(_str + _length) = 0x0;
	}

	NString(const NString &other)
	{
		_length = other._length;
		_size = other._size;
		_str = (char *)reallocarray(nullptr, _size, sizeof(char));
		memmove(_str, other._str, _length);
		*(_str + _length) = 0x0;
	}

	NString(NString &&other)
	{
		_length = other._length;
		_size = other._size;
		_str = other._str;

		other._length = 0;
		other._size = 0;
		other._str = nullptr;
	}

	size_t Length() const { return _length; }
	size_t Count() { _length = 0; char *ptr = _str; while (*ptr++) ++_length; return _length; }
	bool IsEmpty() { return _str[0] == 0x0; }

	bool Contains(char c) { char f[2] = { c, 0x0 }; return Contains(f); }
	bool Contains(const NString &str) { return Contains(*str); }
	bool Contains(std::string &str) { return Contains(str.c_str()); }
	bool Contains(const char *str) { return Find(str) != NotFound; }

	void Append(NString &str) { Append(*str); }
	void Append(std::string &str) { Append(str.c_str()); }
	void Append(char c) { char f[2] = { c, 0x0 }; Append(f); }
	void Append(const char *str)
	{
		if (!str)
			return;

		size_t len = strlen(str);

		if (!len)
			return;

		if (!_str)
		{
			_size = len + 1;
			_length = len;
			if ((_str = (char *)realloc(nullptr, _size)) == nullptr)
				return;

			memcpy(_str, str, len);
			_str[len] = 0x0;
			return;
		}

		if (_length + len >= _size)
			if (!Resize(_length + len + 1))
				return;

		strncat(_str, str, len);
		_length += len;
		_str[_length] = 0x0;
	}

	void AppendFormat(size_t len, const char *fmt, ...)
	{
		NString str(len);
		va_list args;

		va_start(args, fmt);
		vsnprintf(*str, len, fmt, args);
		va_end(args);

		str._length = strlen(*str);

		Append(str);
	}

	size_t Find(char c) { char f[2] = { c, 0x0 }; return Find(f); }
	size_t Find(NString &str) { return Find(*str); }
	size_t Find(std::string &str) { return Find(str.c_str()); }
	size_t Find(const char *str)
	{
		char *ptr = strstr(_str, str);
		if (!ptr) return NotFound;
		return ptr - _str;
	}

	size_t FindFirst(char c)
	{
		char *ptr = strchr(_str, c);
		if (!ptr) return NotFound;
		return ptr - _str;
	}

	size_t FindLast(char c)
	{
		char *ptr = strrchr(_str, c);
		if (!ptr) return NotFound;
		return ptr - _str;
	}

	NString Substring(size_t start, size_t len = 0)
	{
		size_t end;
		if (!len) end = _length;
		else end = start + len;
		return NString(end - start, _str + start);
	}

	NArray<NString> Split(char delim)
	{
		const char *ptr = nullptr;
		const char *start = _str;
		NArray<NString> ret = NArray<NString>();
		size_t len = 0;

		if (!_str)
			return ret;

		if ((ptr = strchr(_str, delim)) == nullptr)
		{
			NString str = *this;
			ret.Add(str);
			return ret;
		}

		while (ptr)
		{
			len = ptr - start;
			NString sub(len, start);
			ret.Add(sub);

			if (ptr) start = ++ptr;
			if (ptr) ptr = strchr(++ptr, delim);
		}

		if (!start)
			return ret;

		len = strlen(start);
		NString sub(len, start);
		ret.Add(sub);

		return ret;
	}

	bool Resize(size_t size)
	{
		if (_size == size)
			return true;

		char *ptr = _str;
		if ((_str = (char *)reallocarray(_str, size, sizeof(char))) == nullptr)
		{
			_str = ptr;
			return false;
		}

		_size = size;

		if (_size < _length)
		{
			_length = _size - 1;
			_str[_length] = 0x0;
		}

		return true;
	}

	void RemoveLast() { if(_length) _str[--_length] = 0x0; }

	void RemoveNewLine()
	{
		size_t len = strlen(_str);

		for (size_t i = 0; i < len; i++)
		{
			if (_str[i] == '\n' || _str[i] == '\r')
			{
				_str[i] = 0x0;
				return;
			}
		}
	}

	void RemoveComment()
	{
		char* pos = strchr(_str, '#');
		if (pos)
			*pos = 0x0;
	}

	void Clear()
	{
		memset(_str, 0x0, _size);
		_length = 0;
	}

	virtual ~NString()
	{
		free(_str);
		_str = nullptr;
		_length = 0;
		_size = 0;
	}

	explicit operator int() { return (int)atof(_str); }
	explicit operator unsigned int() { return (unsigned int)atof(_str); }
	explicit operator long() { return (long)atof(_str); }
	explicit operator unsigned long() { return (unsigned long)atof(_str); }
	explicit operator float() { return (float)atof(_str); }
	explicit operator double() { return atof(_str); }
	explicit operator bool() { return *this == "true"; }

	NString &operator =(const NString &other)
	{
		free(_str);

		_length = other._length;
		_size = other._size;
		_str = (char *)reallocarray(nullptr, _size, sizeof(char));
		memmove(_str, other._str, _length);
		*(_str + _length) = 0x0;

		return *this;
	}

	NString &operator =(NString &&other)
	{
		free(_str);

		_length = other._length;
		_size = other._size;
		_str = other._str;
		
		other._length = 0;
		other._size = 0;
		other._str = nullptr;

		return *this;
	}

	NString &operator +=(NString other) { Append(other); return *this; }

	bool operator ==(NString const &other)
	{
		if (!_length || !other._length)
			return false;

		return !strncmp(_str, other._str, _length);
	}
	bool operator !=(NString const &other) { return strncmp(_str, other._str, _length) != 0 ? true : false; }

	char &operator [](size_t i) { return _str[i]; }
	char *operator *() { return _str; }
	const char *operator *() const { return _str; }

	inline bool operator < (const NString &other) const
	{
		if (!_length && other._length)
			return false;

		if (_length && !other._length)
			return true;

		return strncmp(_str, other._str, _length) < 0;
	}
	inline bool operator> (const NString other) const { return other < *this; }
	//inline bool operator<= (NString &other) { return !(*this > other); }

	static NString StringWithFormat(size_t len, const char *fmt, ...)
	{
		NString str(len);
		va_list args;

		va_start(args, fmt);
		vsnprintf(*str, len, fmt, args);
		va_end(args);

		str._length = strlen(*str);

		return str;
	}

	static constexpr size_t NotFound = -1;

private:
	char *_str;
	size_t _length, _size;
};