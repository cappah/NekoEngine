/* NekoEngine
 *
 * TerrainComponent.cpp
 * Author: Alexandru Naiman
 *
 * TerrainComponent
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

#include <System/Logger.h>
#include <System/VFS/VFS.h>
#include <System/AssetLoader/AssetLoader.h>
#include <Engine/ResourceManager.h>
#include <Renderer/Debug.h>
#include <Scene/Object.h>
#include <Scene/Components/TerrainComponent.h>

using namespace glm;

#define TERRAIN_COMPONENT_MODULE	"TerrainComponent"

ENGINE_REGISTER_COMPONENT_CLASS(TerrainComponent);

TerrainComponent::TerrainComponent(ComponentInitializer *initializer) :
	StaticMeshComponent(initializer),
	_cellSize(20.f),
	_numCells(4),
	_uvStep(0.f),
	_heightmapPath(),
	_heightmapData(nullptr),
	_heightmapSize(0),
	_heightmapWidth(0), _heightmapHeight(0),
	_maxHeight(0)
{
	if (!initializer)
	{
		Logger::Log(TERRAIN_COMPONENT_MODULE, LOG_WARNING, "No initializer supplied, using default values");
		return;
	}

	ArgumentMapType::iterator it;
	const char *ptr = initializer->arguments.find("numcells")->second.c_str();
	_numCells = ptr ? (unsigned short)atoi(ptr) : 4;

	ptr = initializer->arguments.find("cellsize")->second.c_str();
	_cellSize = ptr ? (float)atof(ptr) : 20;
	
	if (((it = initializer->arguments.find("heightmap")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
	{
		_heightmapPath = initializer->arguments.find("heightmap")->second.c_str();
		ptr = initializer->arguments.find("maxheight")->second.c_str();
		_maxHeight = atoi(ptr);
	}
}

int TerrainComponent::Load()
{
	int ret = StaticMeshComponent::Load();
	if (ret != ENGINE_OK)
		return ret;

	if (!_GenerateTerrain())
		return ENGINE_FAIL;

	_mesh->_depthPipelineId = PIPE_Terrain_Depth;
	_mesh->_groupOffset.push_back(0);
	_mesh->_groupCount.push_back((uint32_t)_terrainIndices.size());

	_parent->SetNoCull(true);

	return ENGINE_OK;
}

VkDeviceSize TerrainComponent::GetRequiredMemorySize()
{
	VkDeviceSize size = sizeof(TerrainVertex) * _terrainVertices.size() + sizeof(uint32_t) * _terrainIndices.size();
	if (size % 256)
	{
		size = size / 256;
		++size *= 256;
	}
	return size;
}

bool TerrainComponent::Upload(Buffer *buffer)
{
	if (_mesh->_buffer)
		delete _mesh->_buffer;

	_mesh->_buffer = buffer;

	VkDeviceSize bufferSize = GetRequiredMemorySize();
	Buffer *stagingBuffer = new Buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, nullptr, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	uint8_t *ptr = stagingBuffer->Map();
	if (!ptr)
	{
		Logger::Log(TERRAIN_COMPONENT_MODULE, LOG_CRITICAL, "Failed to map memory");
		return false;
	}

	memcpy(ptr, _terrainVertices.data(), sizeof(_terrainVertices[0]) * _terrainVertices.size());
	memcpy(ptr + sizeof(_terrainVertices[0]) * _terrainVertices.size(), _terrainIndices.data(), sizeof(_terrainIndices[0]) * _terrainIndices.size());

	stagingBuffer->Unmap();

	stagingBuffer->Copy(_mesh->_buffer, bufferSize);

	delete stagingBuffer;

	_mesh->_vertexOffset = _mesh->_buffer->GetParentOffset();
	_mesh->_indexOffset = _mesh->_vertexOffset + sizeof(_terrainVertices[0]) * _terrainVertices.size();

	_mesh->_resident = true;

	return true;
}

bool TerrainComponent::_GenerateTerrain() noexcept
{
	_uvStep = 1.f / (float)_numCells;

	TerrainVertex v;
	v.position = vec3();
	v.normal = vec3(0.f, 1.f, 0.f);
	v.tangent = vec3();

	if (_heightmapPath.Length() && !_LoadHeightmap())
		return false;

	for (int i = 0; i <= _numCells; i++)
	{
		for (int j = 0; j <= _numCells; j++)
		{
			v.uv.x = (float)j;
			v.uv.y = (float)i;

			v.heightmapUV.x = _uvStep * j;
			v.heightmapUV.y = _uvStep * i;

			v.position.x = (j - _numCells / 2.f) * _cellSize;
			v.position.z = -((_numCells / 2.f) - i) * _cellSize;

			if (_heightmapPath.Length())
				v.position.y = _ReadHeightmap(v.heightmapUV.x, v.heightmapUV.y) * _maxHeight;

			_terrainVertices.push_back(v);

			if (i < _numCells && j < _numCells)
			{
				int indexOffset = j + i * (_numCells + 1);
				_terrainIndices.push_back(indexOffset + _numCells + 1);
				_terrainIndices.push_back(indexOffset + 1);
				_terrainIndices.push_back(indexOffset);

				indexOffset++;
				_terrainIndices.push_back(indexOffset + _numCells);
				_terrainIndices.push_back(indexOffset + _numCells + 1);
				_terrainIndices.push_back(indexOffset);
			}
		}
	}

	free(_heightmapData); _heightmapData = nullptr;

	return true;
}

bool TerrainComponent::_LoadHeightmap() noexcept
{
	uint8_t *heightmapData, bpp;
	size_t heightmapSize;

	VFSFile *heightmapFile = VFS::Open(_heightmapPath);
	if (!heightmapFile)
	{
		Logger::Log(TERRAIN_COMPONENT_MODULE, LOG_CRITICAL, "Failed to open heightmap file");
		return false;
	}
	
	if ((heightmapData = (uint8_t *)heightmapFile->ReadAll(heightmapSize)) == nullptr)
		return false;

	heightmapFile->Close();

	bool ret = (AssetLoader::LoadTGA(heightmapData, heightmapSize, _heightmapWidth, _heightmapHeight, bpp, &_heightmapData, _heightmapSize) == ENGINE_OK);
	
	free(heightmapData);

	return ret;
}

float TerrainComponent::_ReadHeightmap(float x, float y) noexcept
{
	uint32_t mapX = (uint32_t)(x * _heightmapWidth);
	uint32_t mapY = (uint32_t)(y * _heightmapHeight);
	uint32_t val = *(_heightmapData + (mapY * _heightmapWidth) + mapX);
	return (float)val / 255.f;
}

bool TerrainComponent::Unload()
{
	if (!StaticMeshComponent::Unload())
		return false;

	free(_heightmapData); _heightmapData = nullptr;

	return true;
}
