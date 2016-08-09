/* NekoEngine
 *
 * AssetLoader.cpp
 * Author: Alexandru Naiman
 *
 * Asset loading functions
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

#include <algorithm>
#include <vorbis/vorbisfile.h>

#include <System/AssetLoader/AssetLoader.h>
#include <System/AssetLoader/ogg_callbacks.h>
#include <System/VFS/VFS.h>

#include <Platform/Compat.h>

#define AL_MODULE	"AssetLoader"

#define BUFFER_SIZE		65536
#define DATA_SIZE		524288
#define AL_LINE_BUFF	1024

using namespace std;

/**
 * @struct RIFF_HEADER
 * @ingroup CoreWAVE
 * @brief Resource Interchange File Format header
 */
typedef struct RIFF_HEADER
{
	char chunk_id[4];	///< Contains the letters "RIFF" in ASCII form (0x52494646 big-endian form)
	int chunk_size;		///< 36 + SubChunk2Size. This is the size of the rest of the chunk following this number. This is the entire file minus 8 bytes for the two fields not included in this count: ChunkID and ChunkSize.
	char format[4];		///< Contains the letters "WAVE" (0x57415645 big-endian form).
} riff_hdr_t;

/**
 * @struct WAVE_FORMAT
 * @ingroup CoreWAVE
 * @brief WAVE format subchunk
 *
 * The "WAVE" format consists of two subchunks: "fmt " and "data".
 *
 * The "fmt " subchunk describes the sound data's format
 */
typedef struct WAVE_FORMAT
{
	char sub_chunk_id[4];	///< Contains the letters "fmt " (0x666d7420)
	int sub_chunk_size;	///< 16 for PCM. This is the size of the rest of the Subchunk which follows this number.
	short audio_format;	///< PCM = 1 (i.e. Linear quantization). Values other than 1 indicate some form of compression.
	short num_channels;	///< Mono = 1, Stereo = 2, etc.
	int sample_rate;	///< 8000, 44100, etc.
	int byte_rate;		///< == SampleRate * NumChannels * BitsPerSample/8
	short block_align;	///< == NumChannels + BitsPerSample/8
	short bits_per_sample;	///< 8 bits = 8, 16 bits = 16, etc.
} wave_fmt_t;

/**
 * @struct WAVE_FORMAT
 * @ingroup CoreWAVE
 * @brief WAVE data subchunk
 *
 * The "WAVE" format consists of two subchunks: "fmt " and "data".
 *
 * The "data" subchunk contains the size of the data and the actual sound
 */
typedef struct WAVE_DATA
{
	char sub_chunk_id[4];	///< Contains the letters "data" (0x64617461 big-endian form).
	int sub_chunk_2_size;	///< == NumSamples * NumChannels * BitsPerSammple/8. This is the number of bytes in the data. You can also think of this as the size of the read of the subchunk following this number.
} wave_data_t;

using namespace glm;

int AssetLoader::LoadMesh(NString &file,
						  MeshType type,
						  vector<Vertex> &vertices,
						  vector<uint32_t> &indices,
						  vector<uint32_t> &groupOffset,
						  vector<uint32_t> &groupCount,
						  vector<Bone> *bones,
						  vector<TransformNode> *nodes,
						  dmat4 *globalInverseTransform)
{
	unsigned int offset = 0;
	uint32_t indexBuff[3];
	/*size_t indexCount = 0;
	size_t vertexCount = 0;
	size_t boneCount = 0;
	size_t nodeCount = 0;*/
	char lineBuff[AL_LINE_BUFF];
	memset(lineBuff, 0x0, AL_LINE_BUFF);

	VFSFile *f = VFS::Open(file);
	if(!f)
	{
		Logger::Log(AL_MODULE, LOG_CRITICAL, "Failed to open mesh file %s", *file);
		return ENGINE_IO_FAIL;
	}

	while (!f->EoF())
	{
		f->Gets(lineBuff, AL_LINE_BUFF);

		if (lineBuff[0] == 0x0)
			continue;

		EngineUtils::RemoveNewline(lineBuff);;

		if (lineBuff[0] == 0x0)
			continue;

		if (strstr(lineBuff, "vertices"))
		{
			char *ptr = strchr(lineBuff, ':');
			if (!ptr)
				break;

			//vertexCount = atoi(++ptr);
		}
		else if (strstr(lineBuff, "indices"))
		{
			char *ptr = strchr(lineBuff, ':');
			if (!ptr)
				break;

			//indexCount = atoi(++ptr);
		}
		else if (strstr(lineBuff, "bones"))
		{
			char *ptr = strchr(lineBuff, ':');
			if (!ptr)
				break;
			
			//boneCount = atoi(++ptr);
		}
		else if (strstr(lineBuff, "git"))
		{
			char *ptr = strchr(lineBuff, ':');
			if(!ptr)
				break;
			
			EngineUtils::ReadDoubleArray(++ptr, 16, &(*globalInverseTransform)[0][0]);
		}
		else if (strstr(lineBuff, "nodes"))
		{
			char *ptr = strchr(lineBuff, ':');
			if (!ptr)
				break;
			
			//nodeCount = atoi(++ptr);
		}
		else if (strchr(lineBuff, '[')) // Vertex line
			vertices.push_back(_ReadVertex(lineBuff));
		else if (strstr(lineBuff, "newidgrp"))
		{
			groupOffset.push_back((uint32_t)indices.size());
			groupCount.push_back((uint32_t)indices.size() - offset);
			offset = (uint32_t)indices.size();
		}
		else if(strchr(lineBuff, '{')) // Bone line
		{
			if(bones)
				bones->push_back(_ReadBone(lineBuff));
			else
			{
				Logger::Log(AL_MODULE, LOG_CRITICAL, "Bone line found, but no Bone array provided");
				f->Close();
				return ENGINE_FAIL;
			}
		}
		else if(strchr(lineBuff, '(')) // TransformNode line
		{
			if(nodes)
				nodes->push_back(_ReadTransformNode(lineBuff));
			else
			{
				Logger::Log(AL_MODULE, LOG_CRITICAL, "TransformNode line found, but no TransformNode array provided");
				f->Close();
				return ENGINE_FAIL;
			}
		}
		else // Index line
		{
			EngineUtils::ReadUIntArray(lineBuff, 3, indexBuff);
			
			indices.push_back(indexBuff[0]);
			indices.push_back(indexBuff[1]);
			indices.push_back(indexBuff[2]);
		}
	}

    groupCount.push_back((uint32_t)indices.size() - offset);
    
    f->Close();
	
	return ENGINE_OK;
}

int AssetLoader::LoadAnimation(NString &file,
						 std::string &name,
						 double *duration,
						 double *ticksPerSecond,
						 vector<AnimationNode> &channels)
{
	char lineBuff[AL_LINE_BUFF], *pch = nullptr;
	VectorKey vk;
	QuatKey qk;
	AnimationNode channel;
	memset(lineBuff, 0x0, AL_LINE_BUFF);
	
	VFSFile *f = VFS::Open(file);
	if(!f)
	{
		Logger::Log(AL_MODULE, LOG_CRITICAL, "Failed to open animation file %s", *file);
		return ENGINE_IO_FAIL;
	}
	
	while (!f->EoF())
	{
		f->Gets(lineBuff, AL_LINE_BUFF);
		
		if (lineBuff[0] == 0x0)
			continue;
		
		EngineUtils::RemoveNewline(lineBuff);;
		
		if (lineBuff[0] == 0x0)
			continue;
		
		if (strstr(lineBuff, "name"))
		{
			char *ptr = strchr(lineBuff, ':');
			if (!ptr)
				break;
			
			name = ++ptr;
		}
		else if (strstr(lineBuff, "duration"))
		{
			char *ptr = strchr(lineBuff, ':');
			if (!ptr)
				break;
			
			*duration = atof(++ptr);
		}
		else if (strstr(lineBuff, "tickspersecond"))
		{
			char *ptr = strchr(lineBuff, ':');
			if (!ptr)
				break;
			
			*ticksPerSecond = atof(++ptr);
		}
		else if (strstr(lineBuff, "channels"))
		{
			char *ptr = strchr(lineBuff, ':');
			if (!ptr)
				break;
			
			channels.resize(atoi(++ptr));
		}
		else if (strstr(lineBuff, "EndChannel"))
			channels.push_back(channel);
		else if (strstr(lineBuff, "Channel"))
		{
			vector<char*> split = EngineUtils::SplitString(lineBuff, '=');
			
			channel.name = split[1];
			channel.positionKeys.clear();
			channel.rotationKeys.clear();
			channel.scalingKeys.clear();
			
			for (char* c : split)
				free(c);
		}		
		else if ((pch = strstr(lineBuff, "poskey")) != nullptr)
		{
			vector<char *> split = EngineUtils::SplitString(lineBuff + 7, '|');
			
			vk.time = atof(split[0]);
			EngineUtils::ReadDoubleArray(split[1], 3, &vk.value.x);
			channel.positionKeys.push_back(vk);
			
			for (char* c : split)
				free(c);
		}
		else if ((pch = strstr(lineBuff, "rotkey")) != nullptr)
		{
			vector<char *> split = EngineUtils::SplitString(lineBuff + 7, '|');
			
			qk.time = atof(split[0]);
			EngineUtils::ReadDoubleArray(split[1], 4, &qk.value.x);
			channel.rotationKeys.push_back(qk);
			
			for (char* c : split)
				free(c);
		}
		else if ((pch = strstr(lineBuff, "scalekey")) != nullptr)
		{
			vector<char *> split = EngineUtils::SplitString(lineBuff + 9, '|');
			
			vk.time = atof(split[0]);
			EngineUtils::ReadDoubleArray(split[1], 3, &vk.value.x);
			channel.scalingKeys.push_back(vk);
			
			for (char* c : split)
				free(c);
		}
	}
	
	f->Close();
	
	return ENGINE_OK;
}

int AssetLoader::LoadWAV(NString &file, ALenum *format, ALvoid **data, ALsizei *size, ALsizei *freq)
{
	wave_fmt_t wave_fmt;
	riff_hdr_t riff_hdr;
	wave_data_t wave_data;
	int ret = ENGINE_FAIL;

	if(format == NULL)
		return ENGINE_INVALID_ARGS;

	if(data == NULL)
		return ENGINE_INVALID_ARGS;

	if(size == NULL)
		return ENGINE_INVALID_ARGS;

	if(freq == NULL)
		return ENGINE_INVALID_ARGS;

	VFSFile *f = VFS::Open(file);
	if(!f)
	{ ret = ENGINE_IO_FAIL; goto exit; }

	memset(&riff_hdr, 0x0, sizeof(riff_hdr_t));
	if(f->Read(&riff_hdr, sizeof(riff_hdr_t), 1) != 1)
	{ ret = ENGINE_IO_FAIL; goto exit; }

	if((riff_hdr.chunk_id[0] != 'R' ||
	    riff_hdr.chunk_id[1] != 'I' ||
            riff_hdr.chunk_id[2] != 'F' ||
	    riff_hdr.chunk_id[3] != 'F') ||
	   (riff_hdr.format[0] != 'W' ||
	    riff_hdr.format[1] != 'A' ||
            riff_hdr.format[2] != 'V' ||
	    riff_hdr.format[3] != 'E'))
	{ ret = ENGINE_INVALID_RES; goto exit; }

	memset(&wave_fmt, 0x0, sizeof(wave_fmt_t));
	if(f->Read(&wave_fmt, sizeof(wave_fmt_t), 1) != 1)
	{ ret = ENGINE_IO_FAIL; goto exit; }

	if(wave_fmt.sub_chunk_id[0] != 'f' ||
	   wave_fmt.sub_chunk_id[1] != 'm' ||
           wave_fmt.sub_chunk_id[2] != 't' ||
	   wave_fmt.sub_chunk_id[3] != ' ')
	{ ret = ENGINE_INVALID_RES; goto exit; }

	if(wave_fmt.sub_chunk_size > 16)
	{
		if(f->Seek(sizeof(short), SEEK_CUR) != 0)
		{ ret = ENGINE_IO_FAIL; goto exit; }
	}

	memset(&wave_data, 0x0, sizeof(wave_data_t));
	if(f->Read(&wave_data, sizeof(wave_data_t), 1) != 1)
	{ ret = ENGINE_IO_FAIL; goto exit; }
	
	if(wave_data.sub_chunk_id[0] != 'd' ||
	   wave_data.sub_chunk_id[1] != 'a' ||
	   wave_data.sub_chunk_id[2] != 't' ||
	   wave_data.sub_chunk_id[3] != 'a')
	{ ret = ENGINE_INVALID_RES; goto exit; }

	if((*data = (ALvoid *)malloc(wave_data.sub_chunk_2_size)) == NULL)
	{ ret = ENGINE_FAIL; goto exit; }

	if(f->Read(*data, 1, wave_data.sub_chunk_2_size) != (uint64_t)wave_data.sub_chunk_2_size)
	{ ret = ENGINE_IO_FAIL; goto exit; }
	
	*size = wave_data.sub_chunk_2_size;
	*freq = wave_fmt.sample_rate;

	if(wave_fmt.num_channels == 1)
	{
		if(wave_fmt.bits_per_sample == 8)
			*format = AL_FORMAT_MONO8;
		else if(wave_fmt.bits_per_sample == 16)
			*format = AL_FORMAT_MONO16;
		else
		{ ret = ENGINE_INVALID_RES; goto exit; }
	}
	else if(wave_fmt.num_channels == 2)
	{
		if(wave_fmt.bits_per_sample == 8)
			*format = AL_FORMAT_STEREO8;
		else if(wave_fmt.bits_per_sample == 16)
			*format = AL_FORMAT_STEREO16;
		else
		{ ret = ENGINE_INVALID_RES; goto exit; }
	}
	else
	{ ret = ENGINE_INVALID_RES; goto exit; }

	f->Close();
	return ENGINE_OK;

exit:
	if(f != NULL)
		f->Close();

	if(*data != NULL)
		free(*data);

	return ret;
}

int AssetLoader::LoadOGG(NString &file, ALenum *format, unsigned char **data, ALsizei *size, ALsizei *freq)
{
	int bitStream;
	long bytes;
	char *buff;
	long dataSize = DATA_SIZE;
	long dataUsed = 0;

	buff = (char*)calloc(BUFFER_SIZE, sizeof(char));
	if (!buff)
		return ENGINE_FAIL;

	VFSFile *f = VFS::Open(file);
	if (!f)
	{
		free(buff);
		return ENGINE_FAIL;
	}

	vorbis_info *info;
	OggVorbis_File oggFile;

	ov_callbacks callbacks;
	callbacks.read_func = ovCbRead;
	callbacks.seek_func = ovCbSeek;
	callbacks.close_func = ovCbClose;
	callbacks.tell_func = ovCvTell;

	if (ov_open_callbacks(f, &oggFile, NULL, 0, callbacks) < 0)
	{
		free(buff);
		return ENGINE_IO_FAIL;
	}

	info = ov_info(&oggFile, -1);

	if (info->channels == 1)
		*format = AL_FORMAT_MONO16;
	else
		*format = AL_FORMAT_STEREO16;

	*freq = (int)info->rate;
	*data = (unsigned char *)reallocarray(NULL, dataSize, sizeof(unsigned char));

	do
	{
		memset(buff, 0x0, BUFFER_SIZE);
		bytes = ov_read(&oggFile, buff, BUFFER_SIZE, 0, 2, 1, &bitStream);

		if (dataUsed + bytes >= dataSize)
		{
			unsigned char *newptr = (unsigned char *)reallocarray(*data, dataSize + DATA_SIZE, sizeof(unsigned char));

			if (newptr == nullptr)
			{
				free(buff);
				free(*data);
				return ENGINE_FAIL;
			}

			*data = newptr;
			dataSize += DATA_SIZE;
		}

		memcpy(*data + dataUsed, buff, bytes);
		dataUsed += bytes;
	}
	while (bytes > 0);

	ov_clear(&oggFile);

	*size = (int)dataUsed;

	f->Close();
	free(buff);

	return ENGINE_OK;
}
