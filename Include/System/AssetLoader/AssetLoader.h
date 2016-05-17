/* Neko Engine
 *
 * AssetLoader.h
 * Author: Alexandru Naiman
 *
 * Asset loading functions
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

#include <string>
#include <vector>

#include <Engine/Bone.h>
#include <Engine/Vertex.h>
#include <Engine/Engine.h>
#include <Renderer/Renderer.h>
#include <Engine/AnimationClip.h>
#include <Resource/MeshResource.h>

class AssetLoader
{
public:
	// Mesh
	static int LoadMesh(std::string &file, MeshType type,
		std::vector<Vertex> &vertices,
		std::vector<uint32_t> &indices,
		std::vector<uint32_t> &groupOffset,
		std::vector<uint32_t> &groupCount,
		std::vector<Bone> *bones = nullptr);
	
	// Sound
	static int LoadWAV(std::string &file, ALenum *format, ALvoid **data, ALsizei *size, ALsizei *freq);
	static int LoadOGG(std::string &file, ALenum *format, unsigned char **data, ALsizei *size, ALsizei *freq);

private:

	/**
	 * Read a vertex from a mesh file
	 */
	static inline Vertex _ReadVertex(const char *line) noexcept
	{
		int n = 0, i = 0, i_buff = 0;
		char c, buff[60] = { 0 }, *pch;		
		Vertex v;

		v.color = glm::vec3(0, 0, 0);

		while (1)
		{
			while ((c = line[i]) != ';' && c != ' ' && c != 0x0)
			{
				buff[i_buff++] = c;
				i++;
			}

			if (c == 0x0 || c == '\n')
				break;

			if (c == ' ')
			{
				i++;
				continue;
			}

			if(i_buff > 0)
				buff[i_buff - 1] = 0x0;

			if ((pch = strstr(buff, "pos")) != NULL)
				EngineUtils::ReadFloatArray(buff + 4, 3, &v.pos.x);
			else if ((pch = strstr(buff, "binorm")) != NULL)
				EngineUtils::ReadFloatArray(buff + 7, 3, &v.binorm.x);
			else if ((pch = strstr(buff, "norm")) != NULL)
				EngineUtils::ReadFloatArray(buff + 5, 3, &v.norm.x);
			else if ((pch = strstr(buff, "tgt")) != NULL)
				EngineUtils::ReadFloatArray(buff + 4, 3, &v.tgt.x);
			else if ((pch = strstr(buff, "uv")) != NULL)
				EngineUtils::ReadFloatArray(buff + 3, 2, &v.uv.x);
			else if ((pch = strstr(buff, "bonei")) != NULL)
				EngineUtils::ReadFloatArray(buff + 6, SH_BONES_PER_VERTEX, v.boneIndices);
			else if ((pch = strstr(buff, "bonew")) != NULL)
				EngineUtils::ReadFloatArray(buff + 6, SH_BONES_PER_VERTEX, v.boneWeights);
			else if ((pch = strstr(buff, "bonen")) != NULL)
				EngineUtils::ReadIntArray(buff + 6, 1, &v.numBones);

			memset(buff, 0x0, i_buff);

			i_buff = 0;
			i++;
			n++;
		}

		return v;
	}
	
	static inline Bone _ReadBone(const char *line) noexcept
	{
		int n = 0, i = 0, i_buff = 0;
		char c, buff[512] = { 0 }, *pch;
		Bone b;
		
		while (1)
		{
			while ((c = line[i]) != ';' && c != ' ' && c != 0x0)
			{
				if(i_buff == 511)
				{ DIE("Bone string too long"); }
				buff[i_buff++] = c;
				i++;
			}
			
			if (c == 0x0 || c == '\n')
				break;
			
			if (c == ' ')
			{
				i++;
				continue;
			}
			
			if(i_buff > 0)
				buff[i_buff - 1] = 0x0;
			
			if ((pch = strstr(buff, "name")) != NULL)
				b.name = buff + 5;
			else if ((pch = strstr(buff, "offset")) != NULL)
				EngineUtils::ReadFloatArray(buff + 7, 16, &b.offset[0][0]);
			else if ((pch = strstr(buff, "parent")) != NULL)
				b.parentId = atoi(buff + 7);
										
			memset(buff, 0x0, i_buff);
										
			i_buff = 0;
			i++;
			n++;
		}
		
		return b;
	}
};

