/* NekoEngine
 *
 * TerrainComponent.h
 * Author: Alexandru Naiman
 *
 * TerrainComponent class
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

#include <Engine/Engine.h>
#include <Renderer/Material.h>
#include <Renderer/StaticMesh.h>
#include <Scene/Components/StaticMeshComponent.h>

#define SM_GENERATED	"generated"

class TerrainComponent : public StaticMeshComponent
{
public:
	ENGINE_API TerrainComponent(ComponentInitializer *initializer);

	ENGINE_API NString &GetMeshID() noexcept { return _meshId; }
	ENGINE_API StaticMesh *GetMesh() noexcept { return _mesh; }
	ENGINE_API virtual size_t GetVertexCount() noexcept override { return _mesh->GetVertexCount(); }
	ENGINE_API virtual size_t GetTriangleCount() noexcept override { return _mesh->GetTriangleCount(); }

	ENGINE_API virtual int Load() override;
	ENGINE_API virtual bool Upload(Buffer *buffer = nullptr) override;

	ENGINE_API virtual bool Unload() override;
	ENGINE_API virtual ~TerrainComponent() { };

	virtual VkDeviceSize GetRequiredMemorySize() override;

protected:
	std::vector<TerrainVertex> _terrainVertices;
	std::vector<uint32_t> _terrainIndices;
	float _cellSize;
	unsigned short _numCells;
	float _uvStep;
	NString _heightmapPath;
	uint8_t *_heightmapData;
	uint64_t _heightmapSize;
	uint32_t _heightmapWidth, _heightmapHeight;
	uint32_t _maxHeight;

	bool _GenerateTerrain() noexcept;
	bool _LoadHeightmap() noexcept;
	float _ReadHeightmap(float x, float y) noexcept;
};