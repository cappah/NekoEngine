//
//  AssimpConverter.m
//  ModelConverter
//
//  Created by Alexandru Naiman on 23/04/16.
//  Copyright © 2016 Alexandru Naiman. All rights reserved.
//

#import "AssimpConverter.h"

#import "NMesh.h"

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
			
			v.index = glm::vec4(-1.f);
		
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
				
				[mesh getVertexAtIndex:vertexId].weight[[mesh getVertexAtIndex:vertexId].numBones] = inMesh->mBones[i]->mWeights[j].mWeight;
				[mesh getVertexAtIndex:vertexId].index[[mesh getVertexAtIndex:vertexId].numBones++] = boneIndex;
			}
		}
		
		if(i != scene->mNumMeshes - 1)
			[mesh newGroup];
	}
	
	return [mesh writeFile:nmeshFile];
}

@end
