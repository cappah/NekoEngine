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
    glm::ivec4 boneIndices;
    glm::vec4 boneWeights;
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
	glm::mat4 finalTransform;
    int parentId;
} NMeshBoneInfo;

@interface NMesh : NSObject
{
	std::vector<NMeshVertex> _vertices;
	std::vector<uint32_t> _indices;
	std::vector<NMeshGroupInfo> _groups;
	std::vector<NMeshBoneInfo> _bones;
}

- (id)init;

- (void)addVertex:(NMeshVertex)vertex;
- (void)addIndex:(uint32_t)index;
- (size_t)addBone:(NMeshBoneInfo)bi;
- (void)addGroup:(NMeshGroupInfo)gi;
- (void)newGroup;

- (NMeshVertex&)getVertexAtIndex:(size_t)index;

- (int)getBoneIndex:(std::string&)name;

- (bool)loadZfg:(NSString *)path;

- (bool)writeFile:(NSString *)path;

- (void)dealloc;

@end
