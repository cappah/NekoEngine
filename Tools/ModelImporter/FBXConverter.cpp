/* NekoEngine - ModelImporter
 *
 * FBXConverter.cpp
 * Author: Alexandru Naiman
 *
 * Converter for static & skeletal meshes using the FBX SDK
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

#include "FBXConverter.h"

using namespace glm;

FBXConverter::FBXConverter()
{
	_fbxManager = FbxManager::Create();
	FbxIOSettings *ioSettings = FbxIOSettings::Create(_fbxManager, IOSROOT);
	_fbxManager->SetIOSettings(ioSettings);
}

bool FBXConverter::Convert(const char *inFile, const char *outFile, bool forceStaticMesh)
{
	FbxImporter *importer = FbxImporter::Create(_fbxManager, "");
	FbxScene *scene = FbxScene::Create(_fbxManager, "");
	FbxNode *rootNode = nullptr;

	if (!importer->Initialize(inFile, -1, _fbxManager->GetIOSettings()))
		return false;

	if (!importer->Import(scene))
	{
		importer->Destroy();
		return false;
	}

	importer->Destroy();

	rootNode = scene->GetRootNode();

	if (!rootNode)
	{
		scene->Destroy();
		return false;
	}

	_staticMesh = new StaticMesh();

	FbxGeometryConverter converter(_fbxManager);
	converter.Triangulate(scene, true, false);
	converter.SplitMeshesPerMaterial(scene, true);

	for (int i = 0; i < rootNode->GetChildCount(); ++i)
	{
		FbxNode *node = rootNode->GetChild(i);

		if (node->GetNodeAttribute() == NULL)
			continue;

		if (node->GetNodeAttribute()->GetAttributeType() != FbxNodeAttribute::eMesh)
			continue;

		FbxMesh *mesh = (FbxMesh *)node->GetNodeAttribute();

		_ProcessStaticMesh(mesh);

		/*FbxVector4 *vertices = mesh->GetControlPoints();

		for (int j = 0; mesh->GetPolygonCount(); ++j)
		{
			int numVertices = mesh->GetPolygonSize(j);
			assert(numVertices == 3);

			for (int k = 0; k < numVertices; ++k)
			{
				Vertex v{};
				bool unmapped;

				int id = mesh->GetPolygonVertex(j, k);
				FbxVector4 position = mesh->GetControlPointAt(id);
				v.position = vec3(position[0], position[1], position[2]);

				FbxVector4 normal;
				if (mesh->GetPolygonVertexNormal(j, id, normal))
					v.normal = vec3(normal[0], normal[1], normal[2]);

				FbxVector2 uv;
				if (mesh->GetPolygonVertexUV(j, id, "", uv, unmapped))
					v.uv = vec2(uv[0], uv[1]);

				_staticMesh->AddVertex(v);
				_staticMesh->AddIndex(id);
			}
		}*/
	}

	scene->Destroy();

	return true;
}

void FBXConverter::_ProcessControlPoints(FbxMesh *mesh)
{
	uint32_t pointCount = mesh->GetControlPointsCount();
	for (uint32_t i = 0; i < pointCount; ++i)
	{
		ControlPoint point;
		point.position = vec3(mesh->GetControlPointAt(i).mData[0], mesh->GetControlPointAt(i).mData[1], mesh->GetControlPointAt(i).mData[2]);
		_controlPoints[i] = point;
	}
}

void FBXConverter::_ProcessStaticMesh(FbxMesh *mesh)
{
	int triangleCount = mesh->GetPolygonCount();
	int vertexCounter = 0;

	_staticMesh->BeginGroup();

	for (uint32_t i = 0; i < triangleCount; ++i)
	{
		Vertex v{};

		for (uint32_t j = 0; j < 3; ++j)
		{
			int index = mesh->GetPolygonVertex(i, j);
			ControlPoint &controlPoint = _controlPoints[index];

			_GetNormal(mesh, index, vertexCounter, v.normal);
			// uv


		}
	}

	_staticMesh->EndGroup();
}

void FBXConverter::_GetNormal(FbxMesh *mesh, int index, int vertexCounter, glm::vec3 &normal)
{
	FbxGeometryElementNormal *vertexNormal = mesh->GetElementNormal(0);
	int normalIndex = index;

	if (vertexNormal->GetMappingMode() == FbxGeometryElement::eByControlPoint)
	{
		if (vertexNormal->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
			normalIndex = vertexNormal->GetIndexArray().GetAt(index);
	}
	else if (vertexNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
	{
		if (vertexNormal->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
			normalIndex = vertexNormal->GetIndexArray().GetAt(vertexCounter);
		else
			normalIndex = vertexCounter;
	}

	normal = vec3(
			vertexNormal->GetDirectArray().GetAt(normalIndex).mData[0],
			vertexNormal->GetDirectArray().GetAt(normalIndex).mData[1],
			vertexNormal->GetDirectArray().GetAt(normalIndex).mData[2]
			);
}

FBXConverter::~FBXConverter()
{
	//
}
