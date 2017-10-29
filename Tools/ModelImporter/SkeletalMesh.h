/* NekoEngine - ModelImporter
 *
 * SkeletalMesh.h
 * Author: Alexandru Naiman
 *
 * SkeletalMesh class definition
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

#ifndef SKELETALMESH_H
#define SKELETALMESH_H

#include "StaticMesh.h"

#include <QObject>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define SKEL_HEADER	"SKELETAL"

struct AnimatedVertex
{
	AnimatedVertex() :
		position(glm::dvec3(0.f)),
		uv(glm::vec2(0.f)),
		normal(glm::dvec3(0.f)),
		tangent(glm::dvec3(0.f)),
		boneIndices(glm::ivec4(0)),
		boneWeights(glm::dvec4(0)),
		numBones(0)
	{ }

	AnimatedVertex(glm::dvec3 inPos) :
		position(inPos),
		uv(glm::vec2(0.f)),
		normal(glm::dvec3(0.f)),
		tangent(glm::dvec3(0.f)),
		boneIndices(glm::ivec4(0)),
		boneWeights(glm::dvec4(0.f)),
		numBones(0)
	{ }

	glm::dvec3 position;
	glm::vec2 uv;
	glm::dvec3 normal;
	glm::dvec3 tangent;
	glm::ivec4 boneIndices;
	glm::dvec4 boneWeights;
	int numBones;
};

struct Bone
{
	std::string name;
	glm::dmat4 offset;
};

struct Node
{
	std::string name;
	glm::dmat4 transform;
	uint16_t parentId;
	std::vector<uint16_t> children;
};

class SkeletalMesh : public QObject
{
	Q_OBJECT
public:
	explicit SkeletalMesh(QObject *parent = 0);

	void AddVertex(AnimatedVertex &v) { _vertices.push_back(v); ++_groupVertexCount; }
	void AddIndex(uint32_t index) { _indices.push_back(_startVertex + index); ++_groupCount; }
	size_t AddBone(Bone &b) { _bones.push_back(b); return _bones.size() - 1; }
	void AddNode(Node &n) { _nodes.push_back(n); }

	size_t VertexCount() { _vertices.size(); }
	size_t IndexCount() { _indices.size(); }

	void SetGlobalInverseTransform(glm::dmat4 &m) { _globalInverseTransform = m; }

	int GetBoneIndex(std::string name);
	uint32_t GetGroupStartVertex() { return _startVertex; }

	void BeginGroup() { _startIndex = (uint32_t)_indices.size(); _startVertex = (uint32_t)_vertices.size(); _groupCount = 0; _groupVertexCount = 0; }
	void SetMaterialID(int32_t index) { _materialId = index; }
	void EndGroup() { _groups.push_back({ _startVertex, _groupVertexCount, _startIndex, _groupCount, _materialId }); }

	AnimatedVertex &GetVertex(int index) { return _vertices[index]; }
	std::vector<GroupInfo> &GetGroups() { return _groups; }
	std::vector<Bone> &GetBones() { return _bones; }

	bool ExportBinary(const char *file);

	virtual ~SkeletalMesh();

signals:

public slots:

private:
	std::vector<AnimatedVertex> _vertices;
	std::vector<uint32_t> _indices;
	std::vector<GroupInfo> _groups;
	std::vector<Bone> _bones;
	std::vector<Node> _nodes;
	uint32_t _startIndex;
	uint32_t _startVertex;
	uint32_t _groupCount;
	uint32_t _groupVertexCount;
	int32_t _materialId;
	glm::dmat4 _globalInverseTransform;
};

#endif // SKELETALMESH_H
