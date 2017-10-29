/* NekoEngine - ModelImporter
 *
 * StaticMesh.cpp
 * Author: Alexandru Naiman
 *
 * StaticMesh implementation
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

#include <zlib.h>

#include "StaticMesh.h"

StaticMesh::StaticMesh(QObject *parent) : QObject(parent)
{
	_startIndex = 0;
	_startVertex = 0;
	_groupCount = 0;
}

bool StaticMesh::ExportBinary(const char *file)
{
	gzFile fp = gzopen(file, "wb");
	uint32_t num;

	if (!fp)
		return false;

	gzwrite(fp, BIN_HEADER, 7);

	// vertices
	num = (uint32_t)_vertices.size();
	gzwrite(fp, &num, sizeof(uint32_t));
	gzwrite(fp, _vertices.data(), (int)(sizeof(Vertex) * _vertices.size()));

	// indices
	num = (uint32_t)_indices.size();
	gzwrite(fp, &num, sizeof(uint32_t));
	gzwrite(fp, _indices.data(), (int)(sizeof(uint32_t) * _indices.size()));

	// groups
	num = (uint32_t)_groups.size();
	gzwrite(fp, &num, sizeof(uint32_t));

	for(GroupInfo &gi : _groups)
	{
		gzwrite(fp, &gi.vertexOffset, sizeof(uint32_t));
		gzwrite(fp, &gi.vertexCount, sizeof(uint32_t));
		gzwrite(fp, &gi.indexOffset, sizeof(uint32_t));
		gzwrite(fp, &gi.indexCount, sizeof(uint32_t));
	}

	gzwrite(fp, BIN_FOOTER, 7);

	gzclose(fp);

	return true;
}

StaticMesh::~StaticMesh()
{

}
