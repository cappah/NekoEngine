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
	aiProcess_JoinIdenticalVertices    | // join identical vertices/ optimize indexing
	//aiProcess_ValidateDataStructure  | // perform a full validation of the loader's output
	aiProcess_Triangulate              | // Ensure all verticies are triangulated (each 3 vertices are triangle)
	aiProcess_RemoveRedundantMaterials | // remove redundant materials
	aiProcess_FindDegenerates          | // remove degenerated polygons from the import
	aiProcess_FindInvalidData          | // detect invalid model data, such as invalid normal vectors
	aiProcess_FindInstances            | // search for instanced meshes and remove them by references to one master
	aiProcess_LimitBoneWeights         | // limit bone weights to 4 per vertex
	0;
	
	const aiScene *scene = _importer.ReadFile([srcFile UTF8String], processFlags);
	
	if(!scene)
		return false;
	
	_globalInverseTransform = scene->mRootNode->mTransformation;
	_globalInverseTransform.Inverse();
	
	NMesh *mesh = [[NMesh alloc] init];
	
	for(uint32_t i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh *inMesh = scene->mMeshes[i];
		for(uint32_t j = 0; j < inMesh->mNumVertices; ++j)
		{
			NMeshVertex v;
			memset(&v, 0x0, sizeof(NMeshVertex));
			
			v.boneIndices = glm::ivec4(-1);
		
			v.pos.x = inMesh->mVertices[j].x;
			v.pos.y = inMesh->mVertices[j].y;
			v.pos.z = inMesh->mVertices[j].z;
			
			v.norm.x = inMesh->mNormals[j].x;
			v.norm.y = inMesh->mNormals[j].y;
			v.norm.z = inMesh->mNormals[j].z;
			
			v.uv.x = inMesh->mTextureCoords[0][j].x;
			v.uv.y = inMesh->mTextureCoords[0][j].y;
			
			[mesh addVertex:v];
		}
		
		for(uint32_t j = 0; j < inMesh->mNumFaces; ++j)
		{
			aiFace &face = inMesh->mFaces[j];
			[mesh addIndex:face.mIndices[0]];
			[mesh addIndex:face.mIndices[1]];
			[mesh addIndex:face.mIndices[2]];
		}
		
		for(uint32_t j = 0; j < inMesh->mNumBones; ++j)
		{
			NMeshBoneInfo bi;
			memset(&bi, 0x0, sizeof(NMeshBoneInfo));
			
			bi.name = inMesh->mBones[j]->mName.data;
			
			int boneIndex = [mesh getBoneIndex:bi.name];
			if(boneIndex == -1)
			{
				bi.offset = glm::make_mat4((float *)&inMesh->mBones[j]->mOffsetMatrix.a1);
				boneIndex = (int)[mesh addBone:bi];
			}
			
			for(uint32_t k = 0; k < inMesh->mBones[j]->mNumWeights; ++k)
			{
				uint32_t vertexId = inMesh->mBones[j]->mWeights[k].mVertexId;
				
				if([mesh getVertexAtIndex:vertexId].numBones == 4)
					return false;
				
				[mesh getVertexAtIndex:vertexId].boneWeights[[mesh getVertexAtIndex:vertexId].numBones] = inMesh->mBones[i]->mWeights[j].mWeight;
				[mesh getVertexAtIndex:vertexId].boneIndices[[mesh getVertexAtIndex:vertexId].numBones++] = boneIndex;
			}
		}
		
		if(inMesh->mNumBones)
			[mesh setGlobalInverseTransform:glm::make_mat4(&_globalInverseTransform.a1)];
		
		if(i != scene->mNumMeshes - 1)
			[mesh newGroup];
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

@end
