/* NekoEngine
 *
 * Primitives.h
 * Author: Alexandru Naiman
 *
 * Basic 3D primitives
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

#include <Engine/Defs.h>
#include <Renderer/Renderer.h>
#include <Renderer/Primitives.h>

using namespace glm;

static Buffer *_primitiveBuffer{ nullptr };
static uint32_t _numIndices[PrimitiveID::EndEnum];
static uint32_t _indexOffsets[PrimitiveID::EndEnum];
static uint32_t _indexBufferOffset{ 0 };

int Primitives::Initialize()
{
	NArray<Vertex> vertices;
	NArray<uint16_t> indices;
	Vertex v{};
	uint32_t firstVertex{ 0 }, firstIndex{ 0 };
	VkDeviceSize bufferSizes[PrimitiveID::EndEnum], totalBufferSize{ 0 };

	v.position.z = 0;

	{ // Triangle
		v.normal = vec3( 0.f,  0.f, -1.f);

		v.position = vec3(-1.f, -1.f,  0.f);
		v.uv = vec2(0.f, 1.f);
		vertices.Add(v);

		v.position = vec3( 1.f, -1.f,  0.f);
		v.uv = vec2(1.f, 1.f);
		vertices.Add(v);

		v.position = vec3(-1.f, 1.f,  0.f);
		v.uv = vec2(0.f, 0.f);
		vertices.Add(v);

		indices.Add(firstVertex + 0);
		indices.Add(firstVertex + 1);
		indices.Add(firstVertex + 2);

		bufferSizes[PrimitiveID::Triangle] = sizeof(Vertex) * 3 + sizeof(uint16_t) * 3;
		totalBufferSize += bufferSizes[PrimitiveID::Triangle];

		_numIndices[PrimitiveID::Triangle] = 3;
		_indexOffsets[PrimitiveID::Triangle] = firstIndex;
		firstIndex += _numIndices[PrimitiveID::Triangle];

		firstVertex += 3;
	}

	{ // Quad
		v.normal = vec3( 0.f,  0.f, -1.f);

		v.position = vec3(-1.f, -1.f,  0.f);
		v.uv = vec2(0.f, 1.f);
		vertices.Add(v);

		v.position = vec3( 1.f, -1.f,  0.f);
		v.uv = vec2(1.f, 1.f);
		vertices.Add(v);

		v.position = vec3( 1.f,  1.f, 0.f);
		v.uv = vec2(1.f, 0.f);
		vertices.Add(v);

		v.position = vec3(-1.f,  1.f,  0.f);
		v.uv = vec2(0.f, 0.f);
		vertices.Add(v);

		indices.Add(firstVertex + 0);
		indices.Add(firstVertex + 1);
		indices.Add(firstVertex + 2);
		indices.Add(firstVertex + 0);
		indices.Add(firstVertex + 2);
		indices.Add(firstVertex + 3);

		bufferSizes[PrimitiveID::Quad] = sizeof(Vertex) * 4 + sizeof(uint16_t) * 6;
		totalBufferSize += bufferSizes[PrimitiveID::Quad];

		_numIndices[PrimitiveID::Quad] = 6;
		_indexOffsets[PrimitiveID::Quad] = firstIndex;
		firstIndex += _numIndices[PrimitiveID::Quad];

		firstVertex += 4;
	}

	{ // Box

		// Front
		v.normal = vec3( 0.f,  0.f, -1.f);

		v.position = vec3(-1.f, -1.f,  1.f);
		v.uv = vec2(0.f, 1.f);
		vertices.Add(v);

		v.position = vec3( 1.f, -1.f,  1.f);
		v.uv = vec2(1.f, 1.f);
		vertices.Add(v);

		v.position = vec3( 1.f,  1.f,  1.f);
		v.uv = vec2(1.f, 0.f);
		vertices.Add(v);

		v.position = vec3(-1.f,  1.f,  1.f);
		v.uv = vec2(0.f, 0.f);
		vertices.Add(v);

		indices.Add(firstVertex + 0);
		indices.Add(firstVertex + 1);
		indices.Add(firstVertex + 2);
		indices.Add(firstVertex + 0);
		indices.Add(firstVertex + 2);
		indices.Add(firstVertex + 3);
		
		// Right
		v.normal = vec3( 1.f,  0.f,  0.f);

		v.position = vec3( 1.f,  1.f, 1.f);
		v.uv = vec2(0.f, 1.f);
		vertices.Add(v);

		v.position = vec3( 1.f,  1.f, -1.f);
		v.uv = vec2(1.f, 1.f);
		vertices.Add(v);

		v.position = vec3( 1.f, -1.f, -1.f);
		v.uv = vec2(1.f, 0.f);
		vertices.Add(v);

		v.position = vec3( 1.f, -1.f,  1.f);
		v.uv = vec2(0.f, 0.f);
		vertices.Add(v);

		indices.Add(firstVertex + 4);
		indices.Add(firstVertex + 5);
		indices.Add(firstVertex + 6);
		indices.Add(firstVertex + 4);
		indices.Add(firstVertex + 6);
		indices.Add(firstVertex + 7);
		
		// Back
		v.normal = vec3( 0.f,  0.f,  1.f);

		v.position = vec3(-1.f, -1.f, -1.f);
		v.uv = vec2(0.f, 1.f);
		vertices.Add(v);

		v.position = vec3( 1.f, -1.f, -1.f);
		v.uv = vec2(1.f, 1.f);
		vertices.Add(v);

		v.position = vec3( 1.f,  1.f, -1.f);
		v.uv = vec2(1.f, 0.f);
		vertices.Add(v);

		v.position = vec3(-1.f,  1.f, -1.f);
		v.uv = vec2(0.f, 0.f);
		vertices.Add(v);

		indices.Add(firstVertex + 8);
		indices.Add(firstVertex + 9);
		indices.Add(firstVertex + 10);
		indices.Add(firstVertex + 8);
		indices.Add(firstVertex + 10);
		indices.Add(firstVertex + 11);
		
		// Left
		v.normal = vec3( -1.f,  0.f,  0.f);

		v.position = vec3(-1.f, -1.f, -1.f);
		v.uv = vec2(0.f, 1.f);
		vertices.Add(v);

		v.position = vec3(-1.f, -1.f,  1.f);
		v.uv = vec2(1.f, 1.f);
		vertices.Add(v);

		v.position = vec3(-1.f,  1.f,  1.f);
		v.uv = vec2(1.f, 0.f);
		vertices.Add(v);

		v.position = vec3(-1.f,  1.f, -1.f);
		v.uv = vec2(0.f, 0.f);
		vertices.Add(v);

		indices.Add(firstVertex + 12);
		indices.Add(firstVertex + 13);
		indices.Add(firstVertex + 14);
		indices.Add(firstVertex + 12);
		indices.Add(firstVertex + 14);
		indices.Add(firstVertex + 15);
		
		// Top
		v.normal = vec3( 0.f,  1.f,  0.f);

		v.position = vec3( 1.f,  1.f,  1.f);
		v.uv = vec2(0.f, 1.f);
		vertices.Add(v);

		v.position = vec3(-1.f,  1.f,  1.f);
		v.uv = vec2(1.f, 1.f);
		vertices.Add(v);

		v.position = vec3(-1.f,  1.f, -1.f);
		v.uv = vec2(1.f, 0.f);
		vertices.Add(v);

		v.position = vec3( 1.f,  1.f, -1.f);
		v.uv = vec2(0.f, 0.f);
		vertices.Add(v);

		indices.Add(firstVertex + 16);
		indices.Add(firstVertex + 17);
		indices.Add(firstVertex + 18);
		indices.Add(firstVertex + 16);
		indices.Add(firstVertex + 18);
		indices.Add(firstVertex + 19);
		
		// Bottom
		v.normal = vec3( 0.f, -1.f,  0.f);

		v.position = vec3(-1.f, -1.f, -1.f);
		v.uv = vec2(0.f, 1.f);
		vertices.Add(v);

		v.position = vec3( 1.f, -1.f, -1.f);
		v.uv = vec2(1.f, 1.f);
		vertices.Add(v);

		v.position = vec3( 1.f, -1.f,  1.f);
		v.uv = vec2(1.f, 0.f);
		vertices.Add(v);

		v.position = vec3(-1.f, -1.f,  1.f);
		v.uv = vec2(0.f, 0.f);
		vertices.Add(v);

		indices.Add(firstVertex + 20);
		indices.Add(firstVertex + 21);
		indices.Add(firstVertex + 22);
		indices.Add(firstVertex + 20);
		indices.Add(firstVertex + 22);
		indices.Add(firstVertex + 23);

		bufferSizes[PrimitiveID::Box] = sizeof(Vertex) * 24 + sizeof(uint16_t) * 36;
		totalBufferSize += bufferSizes[PrimitiveID::Box];

		_numIndices[PrimitiveID::Box] = 36;
		_indexOffsets[PrimitiveID::Box] = firstIndex;
		firstIndex += _numIndices[PrimitiveID::Box];

		firstVertex += 24;
	}

	{ // Pyramid
		v.position = vec3(-1.f, -1.f, -1.f);
		v.normal = vec3( 0.f,  0.f,  1.f);
		vertices.Add(v);
		
		v.position = vec3( 1.f, -1.f, -1.f);
		v.normal = vec3( 0.f,  0.f,  1.f);
		vertices.Add(v);
		
		v.position = vec3( 1.f, -1.f,  1.f);
		v.normal = vec3( 0.f,  0.f, -1.f);
		vertices.Add(v);
		
		v.position = vec3(-1.f, -1.f,  1.f);
		v.normal = vec3( 0.f,  0.f, -1.f);
		vertices.Add(v);

		v.position = vec3( 0.f,  1.f,  0.f);
		v.normal = vec3( 0.f,  1.f,  0.f);
		vertices.Add(v);

		// Front
		indices.Add(firstVertex + 2);
		indices.Add(firstVertex + 4);
		indices.Add(firstVertex + 3);

		// Right
		indices.Add(firstVertex + 1);
		indices.Add(firstVertex + 4);
		indices.Add(firstVertex + 2);

		// Back
		indices.Add(firstVertex + 0);
		indices.Add(firstVertex + 4);
		indices.Add(firstVertex + 1);

		// Left
		indices.Add(firstVertex + 4);
		indices.Add(firstVertex + 0);
		indices.Add(firstVertex + 3);

		bufferSizes[PrimitiveID::Pyramid] = sizeof(Vertex) * 5 + sizeof(uint16_t) * 12;
		totalBufferSize += bufferSizes[PrimitiveID::Pyramid];

		_numIndices[PrimitiveID::Pyramid] = 12;
		_indexOffsets[PrimitiveID::Pyramid] = firstIndex;
		firstIndex += _numIndices[PrimitiveID::Pyramid];

		firstVertex += 5;
	}

	{ // Sphere

	}

	{ // Cone

	}

	_primitiveBuffer = new Buffer(totalBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, nullptr, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (!_primitiveBuffer)
		return ENGINE_OUT_OF_RESOURCES;

	if (firstIndex % 2)
	{
		indices.Add(0);
		++firstIndex;
	}

	_indexBufferOffset = firstVertex * sizeof(Vertex);

	_primitiveBuffer->UpdateData((uint8_t *)*vertices, 0, firstVertex * sizeof(Vertex));
	_primitiveBuffer->UpdateData((uint8_t *)*indices, _indexBufferOffset, firstIndex * sizeof(uint16_t));

	return ENGINE_OK;
}

void Primitives::DrawPrimitive(PrimitiveID primitive, VkCommandBuffer commandBuffer)
{
	VkBuffer buffers[]{ _primitiveBuffer->GetHandle() };
	VkDeviceSize offsets[]{ 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, buffers[0], _indexBufferOffset, VK_INDEX_TYPE_UINT16);
	vkCmdDrawIndexed(commandBuffer, _numIndices[primitive], 1, _indexOffsets[primitive], 0, 0);
}

void Primitives::Release()
{
	delete _primitiveBuffer;
}