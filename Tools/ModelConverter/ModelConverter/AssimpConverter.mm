//
//  AssimpConverter.m
//  ModelConverter
//
//  Created by Alexandru Naiman on 23/04/16.
//  Copyright © 2016 Alexandru Naiman. All rights reserved.
//

#import "AssimpConverter.h"

#import "NMesh.h"
#import "NAnim.h"

#include <glm/gtc/type_ptr.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>

static std::vector<aiNode*> _nodes;

void buildNodeList(aiNode *node)
{
	_nodes.push_back(node);
	
	for(unsigned int i = 0; i < node->mNumChildren; ++i)
		buildNodeList(node->mChildren[i]);
}

@implementation AssimpConverter

- (id)init
{
	if((self = [super init]) == nil)
		return nil;
	
	return self;
}

- (bool)convert:(NSString *)srcFile output:(NSString *)nmeshFile
{
	unsigned int processFlags =
	//aiProcess_CalcTangentSpace         | // calculate tangents and bitangents if possible
	//aiProcess_JoinIdenticalVertices    | // join identical vertices/ optimize indexing
//	aiProcess_ValidateDataStructure  | // perform a full validation of the loader's output
	aiProcess_Triangulate              | // Ensure all verticies are triangulated (each 3 vertices are triangle)
	//aiProcess_RemoveRedundantMaterials | // remove redundant materials
//	aiProcess_FindDegenerates          | // remove degenerated polygons from the import
//	aiProcess_FindInvalidData          | // detect invalid model data, such as invalid normal vectors
//	aiProcess_FindInstances            | // search for instanced meshes and remove them by references to one master
	aiProcess_LimitBoneWeights         | // limit bone weights to 4 per vertex
//	aiProcess_FixInfacingNormals |
//	aiProcess_OptimizeMeshes |
//	aiProcess_OptimizeGraph;
	aiProcess_GenSmoothNormals;
	
	_importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
	
	const aiScene *scene = _importer.ReadFile([srcFile UTF8String], processFlags);
	
	if(!scene)
		return false;
	
	_globalInverseTransform = scene->mRootNode->mTransformation;
	_globalInverseTransform.Inverse();
	
	buildNodeList(scene->mRootNode);
	
	NMesh *mesh = [[NMesh alloc] init];
	
	for(uint32_t i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh *inMesh = scene->mMeshes[i];
		for(uint32_t j = 0; j < inMesh->mNumVertices; ++j)
		{
			NMeshVertex v;
			memset(&v, 0x0, sizeof(NMeshVertex));
		
			v.pos.x = inMesh->mVertices[j].x;
			v.pos.y = inMesh->mVertices[j].y;
			v.pos.z = inMesh->mVertices[j].z;
			
			v.norm.x = inMesh->mNormals[j].x;
			v.norm.y = inMesh->mNormals[j].y;
			v.norm.z = inMesh->mNormals[j].z;
			
			if(inMesh->mTextureCoords[0])
			{
				v.uv.x = inMesh->mTextureCoords[0][j].x;
				v.uv.y = inMesh->mTextureCoords[0][j].y;
			}
			
			[mesh addVertex:v];
		}
		
		for(uint32_t j = 0; j < inMesh->mNumFaces; ++j)
		{
			aiFace &face = inMesh->mFaces[j];
			[mesh addIndex:face.mIndices[0]];
			[mesh addIndex:face.mIndices[1]];
			[mesh addIndex:face.mIndices[2]];
		}
		
		if(i != scene->mNumMeshes - 1)
			[mesh newGroup];
	}
	
	for(uint32_t i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh *inMesh = scene->mMeshes[i];
	
		for(uint32_t j = 0; j < inMesh->mNumBones; ++j)
		{
			NMeshBoneInfo bi;
			memset(&bi, 0x0, sizeof(NMeshBoneInfo));
		
			bi.name = inMesh->mBones[j]->mName.data;
		
			int boneIndex = [mesh getBoneIndex:bi.name];
			if(boneIndex == -1)
			{
				bi.offset = [self convertMatrix:inMesh->mBones[j]->mOffsetMatrix];
				boneIndex = (int)[mesh addBone:bi];
			}
		
			for(uint32_t k = 0; k < inMesh->mBones[j]->mNumWeights; ++k)
			{
				uint32_t vertexId = inMesh->mBones[j]->mWeights[k].mVertexId;
			
				if(inMesh->mBones[j]->mNumWeights == 0)
					continue;
			
				if([mesh getVertexAtIndex:vertexId].numBones == 4)
					continue;
			
				[mesh getVertexAtIndex:vertexId].boneWeights[[mesh getVertexAtIndex:vertexId].numBones] = inMesh->mBones[j]->mWeights[k].mWeight;
				[mesh getVertexAtIndex:vertexId].boneIndices[[mesh getVertexAtIndex:vertexId].numBones] = boneIndex;
				[mesh getVertexAtIndex:vertexId].numBones++;
			}
		}
	}
	
	if([mesh numBones] > 0)
	{
		[mesh setGlobalInverseTransform:[self convertMatrix:_globalInverseTransform]];
		
		for(size_t i = 0; i < _nodes.size(); ++i)
		{
			NMeshTransformNodeInfo tni;
			memset(&tni, 0x0, sizeof(NMeshTransformNodeInfo));
			
			tni.name = _nodes[i]->mName.data;
			tni.transform = [self convertMatrix:_nodes[i]->mTransformation];
			tni.parentId = _nodes[i]->mParent ? (int)std::distance(_nodes.begin(), std::find(_nodes.begin(), _nodes.end(), _nodes[i]->mParent)) : -1;
			
			for(uint32_t j = 0; j < _nodes[i]->mNumChildren; ++j)
				tni.childIds.push_back((int)std::distance(_nodes.begin(), std::find(_nodes.begin(), _nodes.end(), _nodes[i]->mChildren[j])));
			
			[mesh addTransformNode:tni];
		}
	}
	
	for(int i = 0; i < scene->mNumAnimations; ++i)
	{
		aiAnimation *aiAnim = scene->mAnimations[i];
		NAnim *anim = [[NAnim alloc] initWithName:aiAnim->mName.C_Str() duration:aiAnim->mDuration ticksPerSecond:aiAnim->mTicksPerSecond];
		
		for(int j = 0; j < aiAnim->mNumChannels; ++j)
		{
			aiNodeAnim *aiChannel = aiAnim->mChannels[j];
			AnimationNode channel;
			
			channel.name = aiChannel->mNodeName.C_Str();
			
			for(int k = 0; k < aiChannel->mNumPositionKeys; ++k)
			{
				VectorKey vk;
				vk.time = aiChannel->mPositionKeys[k].mTime;
				
				vk.value.x = aiChannel->mPositionKeys[k].mValue[0];
				vk.value.y = aiChannel->mPositionKeys[k].mValue[1];
				vk.value.z = aiChannel->mPositionKeys[k].mValue[2];
				
				channel.positionKeys.push_back(vk);
			}
			
			for(int k = 0; k < aiChannel->mNumRotationKeys; ++k)
			{
				QuatKey qk;
				qk.time = aiChannel->mRotationKeys[k].mTime;
				
				qk.value.x = aiChannel->mRotationKeys[k].mValue.x;
				qk.value.y = aiChannel->mRotationKeys[k].mValue.y;
				qk.value.z = aiChannel->mRotationKeys[k].mValue.z;
				qk.value.w = aiChannel->mRotationKeys[k].mValue.w;
				
				channel.rotationKeys.push_back(qk);
			}
			
			for(int k = 0; k < aiChannel->mNumScalingKeys; ++k)
			{
				VectorKey vk;
				vk.time = aiChannel->mScalingKeys[k].mTime;
				
				vk.value.x = aiChannel->mScalingKeys[k].mValue[0];
				vk.value.y = aiChannel->mScalingKeys[k].mValue[1];
				vk.value.z = aiChannel->mScalingKeys[k].mValue[2];
				
				channel.scalingKeys.push_back(vk);
			}
			
			[anim addChannel:channel];
		}
		
		[anim writeFile:[NSString stringWithFormat:@"%s/%s.nanim", [[nmeshFile stringByDeletingLastPathComponent] UTF8String], [anim name].c_str()]];
	}
	
	return [mesh writeFile:nmeshFile];
}

- (glm::mat4)convertMatrix:(aiMatrix4x4&)mat
{
	glm::mat4 m;
	
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

@end
