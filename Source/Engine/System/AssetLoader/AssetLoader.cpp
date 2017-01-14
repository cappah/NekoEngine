/* NekoEngine
 *
 * AssetLoader.cpp
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

#include <algorithm>

#include <System/Logger.h>
#include <System/VFS/VFS.h>
#include <System/AssetLoader/AssetLoader.h>

#include <Platform/Compat.h>
#include <Renderer/StaticMesh.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <vorbis/vorbisfile.h>

#define AL_MODULE	"AssetLoader"

#define AL_BUFFER_SIZE	65536
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
	int sub_chunk_size;		///< 16 for PCM. This is the size of the rest of the Subchunk which follows this number.
	short audio_format;		///< PCM = 1 (i.e. Linear quantization). Values other than 1 indicate some form of compression.
	short num_channels;		///< Mono = 1, Stereo = 2, etc.
	int sample_rate;		///< 8000, 44100, etc.
	int byte_rate;			///< == SampleRate * NumChannels * BitsPerSample/8
	short block_align;		///< == NumChannels + BitsPerSample/8
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

size_t _al_ovCbRead(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	return (size_t)((VFSFile *)datasource)->Read(ptr, size, nmemb);
}

int _al_ovCbSeek(void *datasource, ogg_int64_t offset, int whence)
{
	return ((VFSFile *)datasource)->Seek((size_t)offset, whence);
}

int _al_ovCbClose(void *datasource)
{
	((VFSFile *)datasource)->Close();
	return 0;
}

long _al_ovCvTell(void *datasource)
{
	return (long)((VFSFile *)datasource)->Tell();
}

int AssetLoader::LoadStaticMesh(NString &file,
	vector<Vertex> &vertices,
	vector<uint32_t> &indices,
	vector<MeshGroup> &groups)
{
	char idBuff[8]{ 0x0 };
	int ret{ ENGINE_FAIL };

	vertices.clear();
	indices.clear();
	groups.clear();

	if (VFSFile *f = VFS::Open(file))
	{
		f->Read(idBuff, sizeof(char), 7);
		idBuff[7] = 0x0;

		if (!strncmp(idBuff, NMESH2_HEADER, 7))
			ret = _LoadStaticMeshV2(f, vertices, indices, groups, false);
		else if (!strncmp(idBuff, NMESH2A_HEADER, 7))
			ret = _LoadStaticMeshV2(f, vertices, indices, groups, true);
		else
		{
			f->Close();
			Logger::Log(AL_MODULE, LOG_CRITICAL, "File %s is not a valid mesh", *file);
			return ENGINE_INVALID_HEADER;
		}

		f->Close();

		return ret;
	}

	Logger::Log(AL_MODULE, LOG_CRITICAL, "Failed to open mesh file %s", *file);
	return ENGINE_IO_FAIL;
}

int AssetLoader::_LoadStaticMeshV2(VFSFile *file,
	vector<Vertex> &vertices,
	vector<uint32_t> &indices,
	vector<MeshGroup> &groups,
	bool readVertexGroup)
{
	uint32_t num{ 0 };
	char idBuff[8]{ 0x0 };

	file->Read(&num, sizeof(uint32_t), 1);
	vertices.resize(num);
	file->Read(vertices.data(), sizeof(Vertex), num);

	file->Read(&num, sizeof(uint32_t), 1);
	indices.resize(num);
	file->Read(indices.data(), sizeof(uint32_t), num);

	file->Read(&num, sizeof(uint32_t), 1);

	for (uint32_t i = 0; i < num; ++i)
	{
		MeshGroup group{};

		if (readVertexGroup)
		{
			file->Read(&group.vertexOffset, sizeof(uint32_t), 1);
			file->Read(&group.vertexCount, sizeof(uint32_t), 1);
		}

		file->Read(&group.indexOffset, sizeof(uint32_t), 1);
		file->Read(&group.indexCount, sizeof(uint32_t), 1);

		groups.push_back(group);
	}

	file->Read(idBuff, sizeof(char), 7);
	idBuff[7] = 0x0;

	if (strncmp(idBuff, NMESH2_FOOTER, 7))
		Logger::Log(AL_MODULE, LOG_WARNING, "Extra data in StaticMesh file %s", file->GetHeader().name);

	return ENGINE_OK;
}

int AssetLoader::LoadSkeletalMesh(NString &file,
	vector<SkeletalVertex> &vertices,
	vector<uint32_t> &indices,
	vector<MeshGroup> &groups,
	vector<Bone> &bones,
	vector<TransformNode> &nodes,
	dmat4 &globalInverseTransform)
{
	char idBuff[8]{ 0x0 }, skelBuff[9]{ 0x0 };
	int ret{ ENGINE_FAIL };

	vertices.clear();
	indices.clear();
	groups.clear();
	bones.clear();
	nodes.clear();
	globalInverseTransform = dmat4();

	if (VFSFile *f = VFS::Open(file))
	{
		f->Read(idBuff, sizeof(char), 7);
		idBuff[7] = 0x0;

		f->Read(skelBuff, sizeof(char), 8);
		skelBuff[8] = 0x0;

		if (strncmp(skelBuff, NMESH_SKELETAL, 8))
		{
			f->Close();
			Logger::Log(AL_MODULE, LOG_CRITICAL, "File %s is not a valid SkeletalMesh", *file);
			return ENGINE_INVALID_HEADER;
		}

		if (!strncmp(idBuff, NMESH2_HEADER, 7))
			ret = _LoadSkeletalMeshV2(f, vertices, indices, groups, bones, nodes, globalInverseTransform, false);
		else if (!strncmp(idBuff, NMESH2A_HEADER, 7))
			ret = _LoadSkeletalMeshV2(f, vertices, indices, groups, bones, nodes, globalInverseTransform, true);
		else
		{
			f->Close();
			Logger::Log(AL_MODULE, LOG_CRITICAL, "File %s is not a valid mesh", *file);
			return ENGINE_INVALID_HEADER;
		}

		f->Close();

		return ret;
	}

	Logger::Log(AL_MODULE, LOG_CRITICAL, "Failed to open mesh file %s", *file);
	return ENGINE_IO_FAIL;
}

int AssetLoader::_LoadSkeletalMeshV2(VFSFile *file,
	vector<SkeletalVertex> &vertices,
	vector<uint32_t> &indices,
	vector<MeshGroup> &groups,
	vector<Bone> &bones,
	vector<TransformNode> &nodes,
	dmat4 &globalInverseTransform,
	bool readVertexGroup)
{
	uint32_t num{ 0 };
	char idBuff[8]{ 0x0 };

	file->Read(&num, sizeof(uint32_t), 1);
	vertices.resize(num);
	file->Read(vertices.data(), sizeof(SkeletalVertex), num);

	file->Read(&num, sizeof(uint32_t), 1);
	indices.resize(num);
	file->Read(indices.data(), sizeof(uint32_t), num);

	file->Read(&num, sizeof(uint32_t), 1);

	for (uint32_t i = 0; i < num; ++i)
	{
		MeshGroup group{};

		if (readVertexGroup)
		{
			file->Read(&group.vertexOffset, sizeof(uint32_t), 1);
			file->Read(&group.vertexCount, sizeof(uint32_t), 1);
		}

		file->Read(&group.indexOffset, sizeof(uint32_t), 1);
		file->Read(&group.indexCount, sizeof(uint32_t), 1);

		groups.push_back(group);
	}

	file->Read(value_ptr(globalInverseTransform), sizeof(dmat4), 1);

	file->Read(&num, sizeof(uint32_t), 1);
	for (uint32_t i = 0; i < num; ++i)
	{
		Bone bone{};
		uint16_t len{ 0 };

		file->Read(&len, sizeof(uint16_t), 1);
		bone.name.resize(len);
		file->Read(&bone.name[0], sizeof(char), len);

		file->Read(value_ptr(bone.offset), sizeof(dmat4), 1);

		bones.push_back(bone);
	}
	
	file->Read(&num, sizeof(uint32_t), 1);
	for (uint32_t i = 0; i < num; ++i)
	{
		TransformNode node{};
		uint16_t id{ 0 };
		
		file->Read(&id, sizeof(uint16_t), 1);
		node.name.resize(id);
		file->Read(&node.name[0], sizeof(char), id);

		file->Read(value_ptr(node.transform), sizeof(dmat4), 1);
		file->Read(&node.parentId, sizeof(uint16_t), 1);
		file->Read(&node.numChildren, sizeof(uint16_t), 1);
		
		for (uint16_t j = 0; j < node.numChildren; ++j)
		{
			file->Read(&id, sizeof(uint16_t), 1);
			node.childrenIds.push_back(id);
		}

		nodes.push_back(node);
	}

	file->Read(idBuff, sizeof(char), 7);
	idBuff[7] = 0x0;

	if (strncmp(idBuff, NMESH2_FOOTER, 7))
		Logger::Log(AL_MODULE, LOG_WARNING, "Extra data in SkeletalMesh file %s", file->GetHeader().name);

	return ENGINE_OK;
}

int AssetLoader::LoadAnimation(NString &file,
						 std::string &name,
						 double *duration,
						 double *ticksPerSecond,
						 vector<AnimationNode> &channels)
{
	char idBuff[8]{ 0x0 };
	int ret{ ENGINE_FAIL };
	memset(idBuff, 0x0, 8);

	if (VFSFile *f = VFS::Open(file))
	{
		f->Read(idBuff, sizeof(char), 7);
		idBuff[7] = 0x0;

		if (!strncmp(idBuff, NANIM2_HEADER, 7))
			ret = _LoadAnimationV2(f, name, duration, ticksPerSecond, channels);
		else
		{
			f->Close();
			Logger::Log(AL_MODULE, LOG_CRITICAL, "File %s is not a valid animation", *file);
			return ENGINE_INVALID_HEADER;
		}

		f->Close();

		return ret;
	}

	Logger::Log(AL_MODULE, LOG_CRITICAL, "Failed to open AnimationClip file %s", *file);
	return ENGINE_IO_FAIL;
}

int AssetLoader::_LoadAnimationV2(VFSFile *file,
	std::string &name,
	double *duration,
	double *ticksPerSecond,
	std::vector<AnimationNode> &channels)
{
	char idBuff[8]{ 0x0 };
	uint32_t num{ 0 }, numChannels{ 0 }, numKeys{ 0 };
	NString str{};

	file->Read(&num, sizeof(uint32_t), 1);
	str.Resize(num + 1);
	file->Read(*str, sizeof(char), num);
	str[num] = 0x0;
	name = *str;

	file->Read(duration, sizeof(double), 1);
	file->Read(ticksPerSecond, sizeof(double), 1);

	file->Read(&numChannels, sizeof(uint32_t), 1);
	channels.resize(numChannels);
	for (uint32_t i = 0; i < numChannels; ++i)
	{
		file->Read(&num, sizeof(uint32_t), 1);
		channels[i].name.Resize(num + 1);
		file->Read(*channels[i].name, sizeof(char), num);
		channels[i].name[num] = 0x0;

		file->Read(&numKeys, sizeof(uint32_t), 1);
		channels[i].positionKeys.resize(numKeys);
		for (uint32_t j = 0; j < numKeys; ++j)
		{
			file->Read(&channels[i].positionKeys[j].value.x, sizeof(double), 3);
			file->Read(&channels[i].positionKeys[j].time, sizeof(double), 1);
		}

		file->Read(&numKeys, sizeof(uint32_t), 1);
		channels[i].rotationKeys.resize(numKeys);
		for (uint32_t j = 0; j < numKeys; ++j)
		{
			file->Read(&channels[i].rotationKeys[j].value.x, sizeof(double), 4);
			file->Read(&channels[i].rotationKeys[j].time, sizeof(double), 1);
		}

		file->Read(&numKeys, sizeof(uint32_t), 1);
		channels[i].scalingKeys.resize(numKeys);
		for (uint32_t j = 0; j < numKeys; ++j)
		{
			file->Read(&channels[i].scalingKeys[j].value.x, sizeof(double), 3);
			file->Read(&channels[i].scalingKeys[j].time, sizeof(double), 1);
		}
	}

	file->Read(idBuff, sizeof(char), 7);
	idBuff[7] = 0x0;

	if (strncmp(idBuff, NANIM2_FOOTER, 7))
		Logger::Log(AL_MODULE, LOG_WARNING, "Extra data in AnimationClip file %s", file->GetHeader().name);

	return ENGINE_OK;
}

int AssetLoader::LoadWAV(NString &file, AudioFormat *format, void **data, size_t *size, size_t *freq)
{
	wave_fmt_t wave_fmt{};
	riff_hdr_t riff_hdr{};
	wave_data_t wave_data{};
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

	if((*data = (void *)malloc(wave_data.sub_chunk_2_size)) == NULL)
	{ ret = ENGINE_FAIL; goto exit; }

	if(f->Read(*data, 1, wave_data.sub_chunk_2_size) != (uint64_t)wave_data.sub_chunk_2_size)
	{ ret = ENGINE_IO_FAIL; goto exit; }
	
	*size = wave_data.sub_chunk_2_size;
	*freq = wave_fmt.sample_rate;

	if(wave_fmt.num_channels == 1)
	{
		if(wave_fmt.bits_per_sample == 8)
			*format = AudioFormat::Mono_8Bit;
		else if(wave_fmt.bits_per_sample == 16)
			*format = AudioFormat::Mono_16Bit;
		else
		{ ret = ENGINE_INVALID_RES; goto exit; }
	}
	else if(wave_fmt.num_channels == 2)
	{
		if(wave_fmt.bits_per_sample == 8)
			*format = AudioFormat::Stereo_8Bit;
		else if(wave_fmt.bits_per_sample == 16)
			*format = AudioFormat::Stereo_16Bit;
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

int AssetLoader::LoadOGG(NString &file, AudioFormat *format, unsigned char **data, size_t *size, size_t *freq)
{
	int bitStream{ 0 };
	long bytes{ 0 };
	char *buff{ nullptr };
	long dataSize{ DATA_SIZE };
	long dataUsed{ 0 };

	buff = (char*)calloc(AL_BUFFER_SIZE, sizeof(char));
	if (!buff)
		return ENGINE_FAIL;

	VFSFile *f = VFS::Open(file);
	if (!f)
	{
		free(buff);
		return ENGINE_FAIL;
	}

	vorbis_info *info{ nullptr };
	OggVorbis_File oggFile{};

	ov_callbacks callbacks;
	callbacks.read_func = _al_ovCbRead;
	callbacks.seek_func = _al_ovCbSeek;
	callbacks.close_func = _al_ovCbClose;
	callbacks.tell_func = _al_ovCvTell;

	if (ov_open_callbacks(f, &oggFile, NULL, 0, callbacks) < 0)
	{
		free(buff);
		return ENGINE_IO_FAIL;
	}

	info = ov_info(&oggFile, -1);

	if (info->channels == 1)
		*format = AudioFormat::Mono_16Bit;
	else
		*format = AudioFormat::Stereo_16Bit;

	*freq = (int)info->rate;
	*data = (unsigned char *)reallocarray(NULL, dataSize, sizeof(unsigned char));

	do
	{
		memset(buff, 0x0, AL_BUFFER_SIZE);
		bytes = ov_read(&oggFile, buff, AL_BUFFER_SIZE, 0, 2, 1, &bitStream);

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
