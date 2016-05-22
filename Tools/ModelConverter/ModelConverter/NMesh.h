//
//  NMesh.h
//  ModelConverter
//
//  Created by Alexandru Naiman on 23/04/16.
//  Copyright © 2016 Alexandru Naiman. All rights reserved.
//

#import <Foundation/Foundation.h>

#include <stdint.h>

#include <string>
#include <vector>
#include <glm/glm.hpp>

typedef struct NMESH_VERTEX
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 norm;
	glm::vec3 binorm;
	glm::vec3 tgt;
	glm::vec2 uv;
	glm::vec2 terrainUv;
    int boneIndices[4];
    float boneWeights[4];
	int numBones;
} NMeshVertex;

typedef struct NMESH_GROUPINFO
{
	uint32_t vertices;
	uint32_t indices;
} NMeshGroupInfo;

typedef struct NMESH_BONEINFO
{
	std::string name;
	glm::mat4 offset;
} NMeshBoneInfo;

typedef struct NMESH_TNODEINFO
{
	std::string name;
	glm::mat4 transform;
	int parentId;
	std::vector<int> childIds;
} NMeshTransformNodeInfo;

@interface NMesh : NSObject
{
	std::vector<NMeshVertex> _vertices;
	std::vector<uint32_t> _indices;
	std::vector<NMeshGroupInfo> _groups;
	std::vector<NMeshBoneInfo> _bones;
	std::vector<NMeshTransformNodeInfo> _nodes;
	glm::mat4 _globalInverseTransform;
}

- (id)init;

- (size_t)numBones;

- (void)addVertex:(NMeshVertex)vertex;
- (void)addIndex:(uint32_t)index;
- (size_t)addBone:(NMeshBoneInfo)bi;
- (size_t)addTransformNode:(NMeshTransformNodeInfo)tni;
- (void)addGroup:(NMeshGroupInfo)gi;
- (void)newGroup;

- (void)setGlobalInverseTransform:(glm::mat4)globalInverseTransform;

- (NMeshVertex&)getVertexAtIndex:(size_t)index;

- (int)getBoneIndex:(std::string&)name;

- (bool)loadZfg:(NSString *)path;

- (bool)writeFile:(NSString *)path;

- (void)dealloc;

@end
