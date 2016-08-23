/* NekoEngine
 *
 * Terrain.h
 * Author: Alexandru Naiman
 *
 * Terrain class definition 
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

#include <stdint.h>
#include <glm/glm.hpp>

#include <Scene/Object.h>

class Terrain :
	public Object
{
public:
	ENGINE_API Terrain(ObjectInitializer *initializer) noexcept;

	ENGINE_API void SetCellSize(float cellSize) noexcept { _cellSize = cellSize; }
	ENGINE_API void SetNumCells(unsigned short numCells) noexcept { _numCells = numCells; }
	
	ENGINE_API virtual int Load() override;
	ENGINE_API virtual int CreateArrayBuffer() override;
	ENGINE_API virtual void Update(double deltaTime) noexcept override;

	ENGINE_API virtual bool Unload() noexcept override;

	ENGINE_API virtual ~Terrain() noexcept { };

protected:
	std::vector<Vertex> _terrainVertices;
	std::vector<uint32_t> _terrainIndices;
	TextureParams _heightmapParams;
	float _cellSize;
	unsigned short _numCells;
	float _uvStep;
	
	bool _GenerateTerrain() noexcept;
};