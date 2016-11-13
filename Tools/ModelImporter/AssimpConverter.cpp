/* NekoEngine - ModelImporter
 *
 * AssimpConverter.cpp
 * Author: Alexandru Naiman
 *
 * AssimpConverter implementation
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

#include "AssimpConverter.h"

#include <QDir>
#include <QMessageBox>

#include <vector>
#include <iostream>

#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <Material.h>
#include <AnimationClip.h>

using namespace std;
using namespace glm;
using namespace Assimp;

static std::vector<aiNode *> _nodes;

AssimpConverter::AssimpConverter(QObject *parent) : QObject(parent)
{
	_staticMesh = nullptr;
	_skeletalMesh = nullptr;
}

bool AssimpConverter::Convert(const char *inFile, const char *outFile, bool forceStaticMesh)
{
	Importer importer;

	importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);

	const aiScene *scene = importer.ReadFile(inFile, aiProcessPreset_TargetRealtime_MaxQuality);

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		QMessageBox::critical(nullptr, "Fatal Error", "Failed to load model file");
		return false;
	}

	if (scene->HasAnimations() && !forceStaticMesh)
	{
		_skeletalMesh = new SkeletalMesh();

		dmat4 inverseTransform = inverse(_ConvertMatrix(scene->mRootNode->mTransformation));
		_skeletalMesh->SetGlobalInverseTransform(inverseTransform);

		_BuildNodeList(scene->mRootNode);

		for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
			_ProcessSkeletalMesh(scene->mMeshes[i]);

		for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
			_ProcessBones(scene->mMeshes[i]);

		if (_skeletalMesh->GetBones().size() > 0)
		{
			for (uint32_t i = 0; i < _nodes.size(); ++i)
			{
				Node n;
				memset(&n, 0x0, sizeof(Node));

				n.name = _nodes[i]->mName.data;
				n.transform = _ConvertMatrix(_nodes[i]->mTransformation);
				n.parentId = _nodes[i]->mParent ? (uint16_t)std::distance(_nodes.begin(), std::find(_nodes.begin(), _nodes.end(), _nodes[i]->mParent)) : -1;

				for(uint32_t j = 0; j < _nodes[i]->mNumChildren; ++j)
					n.children.push_back((uint16_t)std::distance(_nodes.begin(), std::find(_nodes.begin(), _nodes.end(), _nodes[i]->mChildren[j])));

				_skeletalMesh->AddNode(n);
			}
		}

		_skeletalMesh->ExportBinary(outFile);
	}
	else
	{
		_staticMesh = new StaticMesh();

		for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
			_ProcessStaticMesh(scene->mMeshes[i]);

		_staticMesh->ExportBinary(outFile);
	}

	QDir dir(outFile);
	dir.cdUp();
	if (!dir.exists("Materials") && !dir.mkdir("Materials"))
	{
		delete _staticMesh;
		_staticMesh = nullptr;
		delete _skeletalMesh;
		_skeletalMesh = nullptr;

		QMessageBox::warning(nullptr, "Failed to create directory", "Failed to create Materials directory; materials will not be exported.");
		return true;
	}
	dir.cd("Materials");

	QDir::setCurrent(dir.absolutePath());

	QFileInfo fileInfo(outFile);

	QString str{};
	str.sprintf("%s.nscene", fileInfo.baseName().toStdString().c_str());

	FILE *fp = fopen(str.toStdString().c_str(), "w");
	if(!fp)
	{
		delete _staticMesh;
		_staticMesh = nullptr;

		delete _skeletalMesh;
		_skeletalMesh = nullptr;

		QMessageBox::warning(nullptr, "Failed to create scene file", "Failed to create scene file; materials will not be exported.");
		return true;
	}

	if (_staticMesh)
	{
		fprintf(fp, "Object\nComponent=StaticMeshComponent=Mesh\n\tmesh=stm_%s", fileInfo.baseName().toStdString().c_str());

		for (GroupInfo &gi : _staticMesh->GetGroups())
		{
			aiString name;
			if (scene->mMaterials[gi.materialId]->Get(AI_MATKEY_NAME, name))
				fprintf(fp, "\n\tmaterial=mat_%s", name.C_Str());
			else
				fprintf(fp, "\n\tmaterial=mat_material_%d", gi.materialId);
		}
	}
	else if (_skeletalMesh)
	{
		fprintf(fp, "Object\nComponent=SkeletalMeshComponent=Mesh\n\tmesh=skm_%s", fileInfo.baseName().toStdString().c_str());

		for (GroupInfo &gi : _skeletalMesh->GetGroups())
		{
			aiString name;
			if (scene->mMaterials[gi.materialId]->Get(AI_MATKEY_NAME, name))
				fprintf(fp, "\n\tmaterial=mat_%s", name.C_Str());
			else
				fprintf(fp, "\n\tmaterial=mat_material_%d", gi.materialId);
		}
	}

	fprintf(fp, "\nEndComponent\nEndObject");

	fclose(fp);

	for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
	{
		Material mat;
		aiMaterial *aiMat{ scene->mMaterials[i] };
		aiString path{};
		bool _hasSpecular{ false }, _hasNormals{ false }, _hasEmission{ false };
		QString matFile{};

		aiColor3D color(0.f, 0.f, 0.f);
		float f{ 0.f };
		int shadingMode = 0;

		memset(&mat, 0x0, sizeof(Material));

		if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color)) mat.diffuse = vec3(color.r, color.g, color.b);
		if (aiMat->Get(AI_MATKEY_COLOR_SPECULAR, color)) mat.specular = vec3(color.r, color.g, color.b);
		if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, color)) mat.emission = vec3(color.r, color.g, color.b);

		if (aiMat->Get(AI_MATKEY_SHININESS, f)) mat.shininess = f;
		if (aiMat->Get(AI_MATKEY_REFRACTI, f)) mat.ior = f;

		if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &path)) mat.diffuseTex = path.C_Str();
		if (aiMat->GetTexture(aiTextureType_NORMALS, 0, &path)) { mat.normalTex = path.C_Str(); _hasNormals = true; }
		if (aiMat->GetTexture(aiTextureType_SPECULAR, 0, &path)) { mat.specularTex = path.C_Str(); _hasSpecular = true; }
		if (aiMat->GetTexture(aiTextureType_EMISSIVE, 0, &path)) { mat.emissionTex = path.C_Str(); _hasEmission = true; }


		if (aiMat->Get(AI_MATKEY_SHADING_MODEL, shadingMode))
		{
			if (shadingMode == aiShadingMode_Flat)
			{
				mat.type = MT_Unlit;
				continue;
			}
		}

		if (_hasSpecular && _hasNormals && _hasEmission)
			mat.type = MT_NormalPhongSpecularEmission;
		else if (_hasSpecular && _hasNormals)
			mat.type = MT_NormalPhongSpecular;
		else if (_hasSpecular && _hasEmission)
			mat.type = MT_PhongSpecularEmission;
		else if (_hasNormals)
			mat.type = MT_NormalPhong;
		else if (_hasSpecular)
			mat.type = MT_PhongSpecular;
		else
			mat.type = MT_Phong;

		if (aiMat->Get(AI_MATKEY_NAME, path))
			matFile.sprintf("%s.nmtl", path.C_Str());
		else
			matFile.sprintf("material_%d.nmtl", i);

		mat.Save(matFile.toStdString().c_str());
	}

	if (_skeletalMesh)
	{
		QDir animDir(outFile);
		animDir.cdUp();
		if (!animDir.exists("Animations") && !dir.mkdir("Animations"))
		{
			QMessageBox::warning(nullptr, "Failed to create directory", "Failed to create Animations directory; animations will not be exported.");
			return true;
		}
		animDir.cd("Animations");

		QDir::setCurrent(animDir.absolutePath());

		for (uint32_t i = 0; i < scene->mNumAnimations; ++i)
			_ProcessAnimation(scene->mAnimations[i]);
	}

	delete _staticMesh;
	_staticMesh = nullptr;

	delete _skeletalMesh;
	_skeletalMesh = nullptr;

	return true;
}

void AssimpConverter::_ProcessAnimation(struct aiAnimation *animation)
{
	AnimationClip animClip(animation->mName.C_Str(), animation->mDuration, animation->mTicksPerSecond);

	for (uint32_t i = 0; i < animation->mNumChannels; ++i)
	{
		aiNodeAnim *aiChannel = animation->mChannels[i];
		AnimationNode channel;

		channel.name = aiChannel->mNodeName.C_Str();

		for(uint32_t j = 0; j < aiChannel->mNumPositionKeys; ++j)
		{
			VectorKey vk;
			vk.time = aiChannel->mPositionKeys[j].mTime;

			vk.value.x = aiChannel->mPositionKeys[j].mValue[0];
			vk.value.y = aiChannel->mPositionKeys[j].mValue[1];
			vk.value.z = aiChannel->mPositionKeys[j].mValue[2];

			channel.positionKeys.push_back(vk);
		}

		for(uint32_t j = 0; j < aiChannel->mNumRotationKeys; ++j)
		{
			QuatKey qk;
			qk.time = aiChannel->mRotationKeys[j].mTime;

			qk.value.x = aiChannel->mRotationKeys[j].mValue.x;
			qk.value.y = aiChannel->mRotationKeys[j].mValue.y;
			qk.value.z = aiChannel->mRotationKeys[j].mValue.z;
			qk.value.w = aiChannel->mRotationKeys[j].mValue.w;

			channel.rotationKeys.push_back(qk);
		}

		for(uint32_t j = 0; j < aiChannel->mNumScalingKeys; ++j)
		{
			VectorKey vk;
			vk.time = aiChannel->mScalingKeys[j].mTime;

			vk.value.x = aiChannel->mScalingKeys[j].mValue[0];
			vk.value.y = aiChannel->mScalingKeys[j].mValue[1];
			vk.value.z = aiChannel->mScalingKeys[j].mValue[2];

			channel.scalingKeys.push_back(vk);
		}

		animClip.AddChannel(channel);
	}

	char file[255];
	snprintf(file, 255, "%s.nanim", animation->mName.C_Str());

	animClip.Export(file);
}

void AssimpConverter::_ProcessStaticMesh(struct aiMesh *mesh)
{
	Vertex v;
	memset(&v, 0x0, sizeof(Vertex));

	_staticMesh->BeginGroup();

	for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
	{
		v.position = vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		v.normal = mesh->HasNormals() ? vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z) : vec3(0.f);
		v.tangent = mesh->HasTangentsAndBitangents() ? vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z) : vec3(0.f);
		v.uv = mesh->mTextureCoords[0] ? vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : vec2(0.f);

		_staticMesh->AddVertex(v);
	}

	for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
	{
		const aiFace &face = mesh->mFaces[i];

		_staticMesh->AddIndex(face.mIndices[0]);
		_staticMesh->AddIndex(face.mIndices[1]);
		_staticMesh->AddIndex(face.mIndices[2]);
	}

	_staticMesh->SetMaterialID(mesh->mMaterialIndex);
	_staticMesh->EndGroup();
}

void AssimpConverter::_ProcessSkeletalMesh(struct aiMesh *mesh)
{
	AnimatedVertex v;
	memset(&v, 0x0, sizeof(AnimatedVertex));

	_skeletalMesh->BeginGroup();

	for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
	{
		v.position = vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		v.normal = mesh->HasNormals() ? vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z) : vec3(0.f);
		v.tangent = mesh->HasTangentsAndBitangents() ? vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z) : vec3(0.f);
		v.uv = mesh->mTextureCoords[0] ? vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : vec2(0.f);

		_skeletalMesh->AddVertex(v);
	}

	for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
	{
		const aiFace &face = mesh->mFaces[i];

		_skeletalMesh->AddIndex(face.mIndices[0]);
		_skeletalMesh->AddIndex(face.mIndices[1]);
		_skeletalMesh->AddIndex(face.mIndices[2]);
	}

	_skeletalMesh->SetMaterialID(mesh->mMaterialIndex);
	_skeletalMesh->EndGroup();
}

void AssimpConverter::_ProcessBones(struct aiMesh *mesh)
{
	for (uint32_t i = 0; i < mesh->mNumBones; ++i)
	{
		Bone b;

		b.name = mesh->mBones[i]->mName.data;

		int32_t boneIndex = _skeletalMesh->GetBoneIndex(b.name);
		if (boneIndex == -1)
		{
			b.offset = _ConvertMatrix(mesh->mBones[i]->mOffsetMatrix);
			boneIndex = (int32_t)_skeletalMesh->AddBone(b);
		}

		for (uint32_t j = 0; j < mesh->mBones[i]->mNumWeights; ++j)
		{
			uint32_t vertexId = mesh->mBones[i]->mWeights[j].mVertexId;

			AnimatedVertex &av = _skeletalMesh->GetVertex(vertexId);
			if (av.numBones == 4)
				continue;

			av.boneWeights[av.numBones] = mesh->mBones[i]->mWeights[j].mWeight;
			av.boneIndices[av.numBones++] = boneIndex;

			fprintf(stderr, "adding weight: %d, %d, %.01f\n", vertexId, boneIndex, av.boneWeights[av.numBones - 1]);
		}
	}
}

void AssimpConverter::_BuildNodeList(struct aiNode *node)
{
	_nodes.push_back(node);

	for (uint32_t i = 0; i < node->mNumChildren; ++i)
		_BuildNodeList(node->mChildren[i]);
}

dmat4 AssimpConverter::_ConvertMatrix(aiMatrix4x4 &mat)
{
	dmat4 m;

	m[0][0] = mat.a1; m[1][0] = mat.a2;
	m[2][0] = mat.a3; m[3][0] = mat.a4;
	m[0][1] = mat.b1; m[1][1] = mat.b2;
	m[2][1] = mat.b3; m[3][1] = mat.b4;
	m[0][2] = mat.c1; m[1][2] = mat.c2;
	m[2][2] = mat.c3; m[3][2] = mat.c4;
	m[0][3] = mat.d1; m[1][3] = mat.d2;
	m[2][3] = mat.d3; m[3][3] = mat.d4;

	return m;
}

AssimpConverter::~AssimpConverter()
{
	delete _staticMesh;
	delete _skeletalMesh;
}
