/* NekoEngine
 *
 * AssetLoader.h
 * Author: Alexandru Naiman
 *
 * Asset loading functions
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
#include <vector>

#include <Engine/Vertex.h>
#include <Engine/Engine.h>
#include <Runtime/Runtime.h>
#include <Animation/Bone.h>
#include <Animation/TransformNode.h>
#include <Animation/AnimationClip.h>
#include <Resource/MeshResource.h>
#include <Audio/AudioBuffer.h>

#define NMESH1_HEADER	"NMESH1 "
#define NMESH2_HEADER	"NMESH2 "
#define NMESH2_FOOTER	"ENDMESH"
#define NMESH_SKELETAL	"SKELETAL"
#define NMESH2A_HEADER	"NMESH2A"
#define NMESH2B_HEADER	"NMESH2B"

#define NANIM1_HEADER	"NANIM1 "
#define NANIM2_HEADER	"NANIM2 "
#define NANIM2_FOOTER	"ENDANIM"

class AssetLoader
{
public:
	// Mesh
	static int LoadStaticMesh(NString &file,
		std::vector<Vertex> &vertices,
		std::vector<uint32_t> &indices,
		std::vector<struct MeshGroup> &groups);

	static int LoadSkeletalMesh(NString &file,
		std::vector<SkeletalVertex> &vertices,
		std::vector<uint32_t> &indices,
		std::vector<struct MeshGroup> &groups,
		std::vector<Bone> &bones,
		std::vector<TransformNode> &nodes,
		glm::dmat4 &globalInverseTransform);
	
	// Animation clip
	static int LoadAnimation(NString &file,
							 std::string &name,
							 double *duration,
							 double *ticksPerSecond,
							 std::vector<AnimationNode> &channels);
	
	// Sound
	ENGINE_API static int LoadWAV(NString &file, AudioFormat *format, void **data, size_t *size, size_t *freq);
	ENGINE_API static int LoadOGG(NString &file, AudioFormat *format, unsigned char **data, size_t *size, size_t *freq);
	
	// Images
	ENGINE_API static int LoadTGA(const uint8_t *data, uint64_t dataSize, uint32_t &width, uint32_t &height, uint8_t &bpp, uint8_t **imgData, uint64_t &imgDataSize);
	ENGINE_API static int LoadDDS(const uint8_t *data, uint64_t dataSize, uint32_t &width, uint32_t &height, uint32_t &depth, uint32_t &format, uint32_t &mipLevels, uint8_t **imgData, uint64_t &imgDataSize);
	ENGINE_API static int LoadASTC(const uint8_t *data, uint64_t dataSize, uint32_t &width, uint32_t &height, uint32_t &depth, uint32_t &format, uint32_t &mipLevels, uint8_t **imgData, uint64_t &imgDataSize);

	/**
	 * Read a unsigned integer array from a comma separated string
	 */
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

	/**
	 * Read a integer array from a comma separated string
	 */
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

	/**
	 * Read a double array from a comma separated string
	 */
	static inline void ReadDoubleArray(const char *str, int nDouble, double *doubleBuff) noexcept
	{
		int n = 0, i = 0, i_buff = 0;
		char c, buff[60] = { 0 };

		while (n < nDouble)
		{
			while ((c = str[i]) != ',' && c != 0x0)
			{
				buff[i_buff++] = c;
				i++;
			}

			doubleBuff[n] = atof(buff);
			::memset(buff, 0x0, i_buff);

			i_buff = 0;
			i++;
			n++;
		}
	}

	/**
	 * Remove comments from string. If no comment is found, the string is unchanged.
	 */
	static inline char* RemoveComment(char* str) noexcept
	{
		char* pos = strchr(str, '#');
		if (pos)
			*pos = 0x0;
		return str;
	}

	/**
	 * Remove new line character from string. If no new line character is found, the string is unchanged.
	 */
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

	/**
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
		return n < lower ? lower : n > upper ? upper : n;
	}

	/**
	 * Convert cstring to lowercase
	 */
	static inline void to_lower(char *str)
	{
		while (*str)
		{
			*str = tolower(*str); ++str;
		} // Silence C++14 warning
	}

private:
	static int _LoadStaticMeshV2(class VFSFile *file,
		std::vector<Vertex> &vertices,
		std::vector<uint32_t> &indices,
		std::vector<struct MeshGroup> &groups,
		bool readVertexGroup);

	static int _LoadSkeletalMeshV2(VFSFile *file,
		std::vector<SkeletalVertex> &vertices,
		std::vector<uint32_t> &indices,
		std::vector<struct MeshGroup> &groups,
		std::vector<Bone> &bones,
		std::vector<TransformNode> &nodes,
		glm::dmat4 &globalInverseTransform,
		bool readVertexGroup);

	static int _LoadStaticMeshV2B(class VFSFile *file,
		std::vector<Vertex> &vertices,
		std::vector<uint32_t> &indices,
		std::vector<struct MeshGroup> &groups);

	static int _LoadSkeletalMeshV2B(VFSFile *file,
		std::vector<SkeletalVertex> &vertices,
		std::vector<uint32_t> &indices,
		std::vector<struct MeshGroup> &groups,
		std::vector<Bone> &bones,
		std::vector<TransformNode> &nodes,
		glm::dmat4 &globalInverseTransform);

	static int _LoadAnimationV2(VFSFile *file,
		std::string &name,
		double *duration,
		double *ticksPerSecond,
		std::vector<AnimationNode> &channels);
};