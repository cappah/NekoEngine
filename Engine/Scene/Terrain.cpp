/* NekoEngine
 *
 * Terrain.cpp
 * Author: Alexandru Naiman
 *
 * Terrain class implementation
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

#include <glm/glm.hpp>

#include <Engine/Engine.h>
#include <Engine/CameraManager.h>
#include <Engine/SceneManager.h>
#include <System/Logger.h>
#include <Scene/Terrain.h>
#include <Scene/Components/StaticMeshComponent.h>

#define TERRAIN_MODULE	"Terrain"

using namespace glm;
using namespace std;

ENGINE_REGISTER_OBJECT_CLASS(Terrain);

Terrain::Terrain(ObjectInitializer *initializer) noexcept : Object(initializer),
	_heightmapParams{TextureFilter::Trilinear, TextureFilter::Linear, TextureWrap::ClampToEdge, TextureWrap::ClampToEdge},
	_cellSize(20),
	_numCells(4),
	_uvStep(.05f)
{
	if(!initializer)
	{
		Logger::Log(TERRAIN_MODULE, LOG_WARNING, "No initializer supplied, using default values");
		return;
	}
	
	const char *ptr = initializer->arguments.find("numcells")->second.c_str();
	_numCells = ptr ? (unsigned short)atoi(ptr) : 4;
	
	ptr = initializer->arguments.find("cellsize")->second.c_str();
	_cellSize = ptr ? (float)atof(ptr) : 20;
}

bool Terrain::_GenerateTerrain() noexcept
{
	_uvStep = 1.f / (float)_numCells;

	Vertex v;
	v.pos = vec3();
	v.color = vec3();
	v.norm = vec3(0.f, 1.f, 0.f);
	v.tgt = vec3();
	
	for (int i = 0; i <= _numCells; i++)
	{
		for (int j = 0; j <= _numCells; j++)
		{
			v.pos.x = (j - _numCells / 2.f) * _cellSize;
			v.pos.z = -((_numCells / 2.f) - i) * _cellSize;

			v.uv.x = (float)j;
			v.uv.y = (float)i;

			v.terrainUv.x = _uvStep * j;
			v.terrainUv.y = _uvStep * i;

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
	
	return true;
}

int Terrain::Load()
{
	if(!_GenerateTerrain())
		return ENGINE_OUT_OF_RESOURCES;

	int ret = Object::Load();

	if (ret != ENGINE_OK)
		return ret;

	vec3 posVector(0.f, -30.f, 0.f);
	SetPosition(posVector);
	_center = vec3(0.f, 0.f, 0.f);
	SetForwardDirection(ForwardDirection::NegativeZ);

	return ENGINE_OK;
}

int Terrain::CreateArrayBuffer()
{
	return ((StaticMeshComponent*)GetComponent("Mesh"))->GetMesh()->LoadDynamic(_terrainVertices, _terrainIndices);
}

void Terrain::Update(double deltaTime) noexcept
{
	Object::Update(deltaTime);

	static vec3 lastCamPos = vec3(0, 0, 0);
	bool modified = false;
	CameraComponent *cam = CameraManager::GetActiveCamera();
	vec3 camPos = cam->GetPosition();

	if ((camPos.x == lastCamPos.x) && (camPos.z == lastCamPos.z))
		return; // stop moving if the camera stops

	if (camPos.x - _center.x > _cellSize)
	{
		// Move terrain right
		MoveRight(_cellSize);
		_center.x += _cellSize;

		for (unsigned int i = 0; i < _terrainVertices.size(); i++)
		{
			vec2 v = _terrainVertices[i].terrainUv;
			v.x += _uvStep;
			_terrainVertices[i].terrainUv = v;

			modified = true;
		}
	}
	else if (camPos.x - _center.x < -_cellSize)
	{
		// Move terrain left
		MoveRight(-_cellSize);
		_center.x -= _cellSize;

		for (unsigned int i = 0; i < _terrainVertices.size(); i++)
		{
			vec2 v = _terrainVertices[i].terrainUv;
			v.x -= _uvStep;
			_terrainVertices[i].terrainUv = v;

			modified = true;
		}
	}
	else if (camPos.z - _center.z > _cellSize)
	{
		// Move terrain back
		MoveForward(-_cellSize);
		_center.z += _cellSize;

		for (unsigned int i = 0; i < _terrainVertices.size(); i++)
		{
			vec2 v = _terrainVertices[i].terrainUv;
			v.y += _uvStep;
			_terrainVertices[i].terrainUv = v;

			modified = true;
		}
	}
	else if (camPos.z - _center.z < -_cellSize)
	{
		// Move terrain forward
		MoveForward(_cellSize);
		_center.z -= _cellSize;

		for (unsigned int i = 0; i < _terrainVertices.size(); i++)
		{
			vec2 v = _terrainVertices[i].terrainUv;
			v.y -= _uvStep;
			_terrainVertices[i].terrainUv = v;

			modified = true;
		}
	}

	if (modified)
		((StaticMeshComponent*)GetComponent("Mesh"))->GetMesh()->UpdateVertices(_terrainVertices);

	lastCamPos = camPos;
}

bool Terrain::Unload() noexcept
{
	if (!Object::Unload())
		return false;

	_terrainVertices.clear();

	return true;
}
