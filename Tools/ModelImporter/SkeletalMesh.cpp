/* NekoEngine - ModelImporter
 *
 * SkeletalMesh.cpp
 * Author: Alexandru Naiman
 *
 * SkeletalMesh implementation
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

#include <zlib.h>

#include "SkeletalMesh.h"

using namespace glm;

SkeletalMesh::SkeletalMesh(QObject *parent) : QObject(parent)
{
	_startIndex = 0;
	_startVertex = 0;
	_groupCount = 0;
}

bool SkeletalMesh::ExportBinary(const char *file)
{
	gzFile fp = gzopen(file, "wb");
	uint32_t num, len;

	if (!fp)
		return false;

	for (size_t i = 0; i < _vertices.size(); ++i)
	{
		AnimatedVertex &av = _vertices[i];
		float weight = av.boneWeights[0] + av.boneWeights[1] + av.boneWeights[2] + av.boneWeights[3];
		if (weight > 1.0 || weight < 0.98)
		{
			fprintf(stderr, "ERROR: weight is not 1.0 ! (index: %d, weight: %.01f, num: %d)", i, weight, av.numBones);
			return false;
		}
	}

	gzwrite(fp, BIN_HEADER, 7);
	gzwrite(fp, SKEL_HEADER, 8);

	// vertices
	num = (uint32_t)_vertices.size();
	gzwrite(fp, &num, sizeof(uint32_t));
	gzwrite(fp, _vertices.data(), (int)(sizeof(AnimatedVertex) * _vertices.size()));

	// indices
	num = (uint32_t)_indices.size();
	gzwrite(fp, &num, sizeof(uint32_t));
	gzwrite(fp, _indices.data(), (int)(sizeof(uint32_t) * _indices.size()));

	// groups
	num = (uint32_t)_groups.size();
	gzwrite(fp, &num, sizeof(uint32_t));

	for (GroupInfo &gi : _groups)
	{
		gzwrite(fp, &gi.offset, sizeof(uint32_t));
		gzwrite(fp, &gi.count, sizeof(uint32_t));
	}

	gzwrite(fp, value_ptr(_globalInverseTransform), sizeof(dmat4));

	// bones
	num = (uint32_t)_bones.size();
	gzwrite(fp, &num, sizeof(uint32_t));
	for (Bone &b : _bones)
	{
		len = (uint16_t)strlen(b.name.c_str());
		gzwrite(fp, &len, sizeof(uint16_t));
		gzwrite(fp, b.name.c_str(), len);

		gzwrite(fp, value_ptr(b.offset), sizeof(dmat4));
	}

	// nodes
	num = (uint32_t)_nodes.size();
	gzwrite(fp, &num, sizeof(uint32_t));
	for (Node &n : _nodes)
	{
		uint16_t numChildren = (uint16_t)n.children.size();
		len = (uint16_t)strlen(n.name.c_str());
		gzwrite(fp, &len, sizeof(uint16_t));
		gzwrite(fp, n.name.c_str(), len);

		gzwrite(fp, value_ptr(n.transform), sizeof(dmat4));
		gzwrite(fp, &n.parentId, sizeof(uint16_t));
		gzwrite(fp, &numChildren, sizeof(uint16_t));

		for (uint16_t i = 0 ; i < numChildren; ++i)
			gzwrite(fp, &n.children[i], sizeof(uint16_t));
	}

	gzwrite(fp, BIN_FOOTER, 7);

	gzclose(fp);

	return true;
}

int SkeletalMesh::GetBoneIndex(std::string name)
{
	for (int i = 0; i < _bones.size(); ++i)
		if (!_bones[i].name.compare(name))
			return i;

	return -1;
}

SkeletalMesh::~SkeletalMesh()
{
	//
}
