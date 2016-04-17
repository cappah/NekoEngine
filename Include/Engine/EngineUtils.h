/* Neko Engine
 *
 * EngineUtils.h
 * Author: Alexandru Naiman
 *
 * Engine utility functions
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

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <string.h>
#include <Engine/Vertex.h>

// Pre-calculated value of PI / 180
#define kPI180	0.017453f
// Pre-calculated value of 180 / PI
#define k180PI	57.295780f
// Convert from degrees to radians
#define DEG2RAD(x) (x * kPI180)
// Convert from radians to degrees
#define RAD2DEG(x) (x * k180PI)

/**
 * Engine helper functions
 */
class EngineUtils
{
public:
	/**
	 * Split string
	 */
	static inline std::vector<char*> SplitString(const char* str, char delim) noexcept
	{
		const char *ptr = strchr(str, delim);
		const char *start = str;
		std::vector<char*> vec;

		if (!ptr)
			return vec;

		while (ptr)
		{
			size_t len = ptr - start;
			char* sub = (char*)calloc(len + 1, sizeof(char));
			memmove(sub, start, len);
			vec.push_back(sub);
			start = ++ptr;
			ptr = strchr(++ptr, delim);
		}

		size_t len = strlen(start);
		char* sub = (char*)calloc(len + 1, sizeof(char));
		memmove(sub, start, len);
		vec.push_back(sub);

		return vec;
	}

	static inline void ReadUIntArray(const char* str, int nInt, unsigned int *intBuff) noexcept
	{
		int n = 0, i = 0, i_buff = 0;
		char c, buff[60] = { 0 };

		while (n < nInt)
		{
			while ((c = str[i]) != ',' && c != 0x0)
			{
				buff[i_buff++] = c;
				i++;
			}

			intBuff[n] = (unsigned int)atoi(buff);
			memset(buff, 0x0, i_buff);

			i_buff = 0;
			i++;
			n++;
		}
	}

	static inline void ReadIntArray(const char* str, int nInt, int *intBuff) noexcept
	{
		int n = 0, i = 0, i_buff = 0;
		char c, buff[60] = { 0 };

		while (n < nInt)
		{
			while ((c = str[i]) != ',' && c != 0x0)
			{
				buff[i_buff++] = c;
				i++;
			}

			intBuff[n] = atoi(buff);
			memset(buff, 0x0, i_buff);

			i_buff = 0;
			i++;
			n++;
		}
	}

	/**
	* Read a float array from a comma separated string
	*/
	static inline void ReadFloatArray(const char *str, int nFloat, float *floatBuff) noexcept
	{
		int n = 0, i = 0, i_buff = 0;
		char c, buff[60] = { 0 };

		while (n < nFloat)
		{
			while ((c = str[i]) != ',' && c != 0x0)
			{
				buff[i_buff++] = c;
				i++;
			}

			floatBuff[n] = (float)atof(buff);
			::memset(buff, 0x0, i_buff);

			i_buff = 0;
			i++;
			n++;
		}
	}

	/*
	 * Remove comments from string. If no comment is found, the string is unchanged.
	 */
	static inline char* RemoveComment(char* str) noexcept
	{
		char* pos = strchr(str, '#');
		if (pos)
			*pos = 0x0;
		return str;
	}

	static inline void RemoveNewline(char* str) noexcept
	{
		size_t len = strlen(str);

		for (size_t i = 0; i < len; i++)
		{
			if (str[i] == '\n' || str[i] == '\r')
			{
				str[i] = 0x0;
				return;
			}
		}
	}

	/*
	 * Clamp a number between 2 values.
	 *
	 * if(n < lower)
	 *   return lower;
	 * else if(n > upper)
	 *   return upper;
	 * else
	 *   return n;    
	 */
	static inline float clamp(float n, float lower, float upper) noexcept
	{
		return std::max(lower, std::min(n, upper));
	}

	/*
	* Convert cstring to lowercase
	 */
	static inline void to_lower(char *str)
	{
		while (*str)
		{ *str = tolower(*str); ++str; } // Silence C++14 warning
	}
};

