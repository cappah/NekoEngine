/* NekoEngine - ModelImporter
 *
 * StaticMesh.h
 * Author: Alexandru Naiman
 *
 * StaticMesh class definition
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

#ifndef STATICMESH_H
#define STATICMESH_H

#include <QObject>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define BIN_HEADER	"NMESH2 "
#define BIN_FOOTER	"ENDMESH"

struct Vertex
{
	Vertex() :
		position(glm::vec3(0.f)),
		uv(glm::vec2(0.f)),
		normal(glm::vec3(0.f)),
		tangent(glm::vec3(0.f))
	{ }

	Vertex(glm::vec3 inPos) :
		position(inPos),
		uv(glm::vec2(0.f)),
		normal(glm::vec3(0.f)),
		tangent(glm::vec3(0.f))
	{ }

	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec3 tangent;
};

struct GroupInfo
{
	uint32_t offset;
	uint32_t count;
	int32_t materialId;
};

class StaticMesh : public QObject
{
	Q_OBJECT
public:
	explicit StaticMesh(QObject *parent = 0);

	void AddVertex(Vertex &v) { _vertices.push_back(v); }
	void AddIndex(uint32_t index) { _indices.push_back(_startVertex + index); _groupCount++; }

	size_t VertexCount() { _vertices.size(); }
	size_t IndexCount() { _indices.size(); }

	void BeginGroup() { _startIndex = (uint32_t)_indices.size(); _startVertex = (uint32_t)_vertices.size(); _groupCount = 0; }
	void SetMaterialID(int32_t index) { _materialId = index; }
	void EndGroup() { _groups.push_back({ _startIndex, _groupCount, _materialId }); }

	std::vector<GroupInfo> &GetGroups() { return _groups; }

	bool ExportBinary(const char *file);

	virtual ~StaticMesh();

signals:

public slots:

private:
	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;
	std::vector<GroupInfo> _groups;
	uint32_t _startIndex;
	uint32_t _startVertex;
	uint32_t _groupCount;
	int32_t _materialId;
};

#endif // STATICMESH_H
