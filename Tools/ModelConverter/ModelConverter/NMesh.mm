//
//  NMeshUtils.m
//  ModelConverter
//
//  Created by Alexandru Naiman on 23/04/16.
//  Copyright © 2016 Alexandru Naiman. All rights reserved.
//

#import "NMesh.h"

#include <zlib.h>

#include <sstream>
#include <algorithm>

#define LINE_BUFF	2048

using namespace std;

// NFG Helper functions
static inline void RemoveNewline(char* str) noexcept
{
	size_t len = strlen(str);
	
	for (size_t i = 0; i < len; i++)
	{
		if (str[i] == '\n' || str[i] == '\r')
		{
			str[i] = 0x0;
			return;
		}
	}
}

static inline void ReadUIntArray(const char* str, int nInt, unsigned int *intBuff) noexcept
{
	int n = 0, i = 0, i_buff = 0;
	char c, buff[60] = { 0 };
	
	while (n < nInt)
	{
		while ((c = str[i]) != ',' && c != 0x0)
		{
			buff[i_buff++] = c;
			i++;
		}
		
		intBuff[n] = (unsigned int)atoi(buff);
		memset(buff, 0x0, i_buff);
		
		i_buff = 0;
		i++;
		n++;
	}
}

static inline void ReadFloatArray(const char *str, int nFloat, float *floatBuff) noexcept
{
	int n = 0, i = 0, i_buff = 0;
	char c, buff[60] = { 0 };
	
	while (n < nFloat)
	{
		while ((c = str[i]) != ',' && c != 0x0)
		{
			buff[i_buff++] = c;
			i++;
		}
		
		floatBuff[n] = (float)atof(buff);
		::memset(buff, 0x0, i_buff);
		
		i_buff = 0;
		i++;
		n++;
	}
}

static inline NMeshVertex _ReadVertex(const char* line) noexcept
{
	int n = 0, i = 0, i_buff = 0;
	char c, buff[60] = { 0 }, *pch;
	NMeshVertex v;
	
	v.color = glm::vec3(0, 0, 0);
	
	while (1)
	{
		while ((c = line[i]) != ';' && c != ' ' && c != 0x0)
		{
			buff[i_buff++] = c;
			i++;
		}
		
		if (c == 0x0 || c == '\n')
			break;
		
		if (c == ' ')
		{
			i++;
			continue;
		}
		
		if(i_buff > 0)
			buff[i_buff - 1] = 0x0;
		
		if ((pch = strstr(buff, "pos")) != NULL)
			ReadFloatArray(buff + 5, 3, &v.pos.x);
		else if ((pch = strstr(buff, "binorm")) != NULL)
			ReadFloatArray(buff + 8, 3, &v.binorm.x);
		else if ((pch = strstr(buff, "norm")) != NULL)
			ReadFloatArray(buff + 6, 3, &v.norm.x);
		else if ((pch = strstr(buff, "tgt")) != NULL)
			ReadFloatArray(buff + 5, 3, &v.tgt.x);
		else if ((pch = strstr(buff, "uv")) != NULL)
			ReadFloatArray(buff + 4, 2, &v.uv.x);
		
		memset(buff, 0x0, i_buff);
		
		i_buff = 0;
		i++;
		n++;
	}
	
	return v;
}

@implementation NMesh

- (id)init
{
	if((self = [super init]) == nil)
		return nil;
	
	return self;
}

- (size_t)numBones
{
	return _bones.size();
}

- (void)addVertex:(NMeshVertex)vertex
{
	_vertices.push_back(vertex);
}

- (void)addIndex:(uint32_t)index
{
	_indices.push_back(index);
}

- (size_t)addBone:(NMeshBoneInfo)bi
{
	_bones.push_back(bi);
	return _bones.size() - 1;
}

- (size_t)addTransformNode:(NMeshTransformNodeInfo)tni
{
	_nodes.push_back(tni);
	return _nodes.size() - 1;
}

- (void)addGroup:(NMeshGroupInfo)gi
{
	_groups.push_back(gi);
}

- (void)newGroup
{
	NMeshGroupInfo gi;
	gi.indices = (uint32_t)_indices.size();
	gi.vertices = (uint32_t)_vertices.size();
	_groups.push_back(gi);
}

- (void)setGlobalInverseTransform:(glm::dmat4)globalInverseTransform
{
	_globalInverseTransform = globalInverseTransform;
}

- (NMeshVertex&)getVertexAtIndex:(size_t)index
{
	return _vertices[index];
}

- (int)getBoneIndex:(std::string&)name
{
	for(int i = 0; i < _bones.size(); ++i)
		if(!_bones[i].name.compare(name))
			return i;
	
	return -1;
}

- (bool)loadZfg:(NSString *)path
{
	uint32_t indexBuff[3];
	size_t indexCount = 0;
	size_t vertexCount = 0;
	char lineBuff[LINE_BUFF];
	memset(lineBuff, 0x0, LINE_BUFF);
	
	gzFile fp = gzopen([path UTF8String], "rb");
	if(!fp)
		return false;
	
	while (!gzeof(fp))
	{
		gzgets(fp, lineBuff, LINE_BUFF);
		
		if (lineBuff[0] == 0x0)
			continue;
		
		RemoveNewline(lineBuff);;
		
		if (lineBuff[0] == 0x0)
			continue;
		
		if (strstr(lineBuff, "NrVertices"))
		{
			char *ptr = strchr(lineBuff, ':');
			if (!ptr)
				break;
			
			vertexCount = atoi(++ptr);
		}
		else if (strstr(lineBuff, "NrIndices"))
		{
			char *ptr = strchr(lineBuff, ':');
			if (!ptr)
				break;
			
			indexCount = atoi(++ptr);
		}
		else if (strchr(lineBuff, '[')) // Vertex line
		{
			char *ptr = strchr(lineBuff, '.');
			if (!ptr)
				break;
			
			_vertices.push_back(_ReadVertex(++ptr));
		}
		else if (strstr(lineBuff, "NewVertexGroup")) { } // left for compatibility with older format models; no longer necessary
		else if (strstr(lineBuff, "NewIndexGroup"))
		{
			NMeshGroupInfo gi;
			gi.indices = (uint32_t)_indices.size();
			gi.vertices = (uint32_t)_vertices.size();
			_groups.push_back(gi);
		}
		else // Index line
		{
			char *ptr = strchr(lineBuff, '.');
			if (!ptr)
				break;
			
			ReadUIntArray(++ptr, 3, indexBuff);
			
			_indices.push_back(indexBuff[0]);
			_indices.push_back(indexBuff[1]);
			_indices.push_back(indexBuff[2]);
		}
	}
	
	//groupCount.push_back((uint32_t)_indices.size() - offset);
	
	gzclose(fp);
	
	return true;
}

- (bool)writeFile:(NSString *)path
{
	uint32_t group_id = 0;
	stringstream ss("", ios_base::app | ios_base::out);
	
	ss << "vertices:" << _vertices.size() << endl;
	
	for(NMeshVertex &v : _vertices)
	{
		ss << "pos[" << v.pos.x << "," << v.pos.y << "," << v.pos.z << "];";
		ss << "norm[" << v.norm.x << "," << v.norm.y << "," << v.norm.z << "];";
		ss << "binorm[" << v.binorm.x << "," << v.binorm.y << "," << v.binorm.z << "];";
		ss << "tgt[" << v.tgt.x << "," << v.tgt.y << "," << v.tgt.z << "];";
		ss << "uv[" << v.uv.x << "," << v.uv.y << "," << "];";
        ss << "bonei[" << v.boneIndices[0] << "," << v.boneIndices[1] << "," << v.boneIndices[2] << "," << v.boneIndices[3] << "];";
        ss << "bonew[" << v.boneWeights[0] << "," << v.boneWeights[1] << "," << v.boneWeights[2] << "," << v.boneWeights[3] << "];";
        ss << "bonen[" << v.numBones << "];" << endl;
		
		float totalWeight = v.boneWeights[0] + v.boneWeights[1] + v.boneWeights[2] + v.boneWeights[3];
		
		if(totalWeight != 1.0)
			NSLog(@"Warning: total weight is not 1.0 (%.01f)", totalWeight);
	}

	ss << "indices:" << _indices.size() << endl;
	int pos = 0;
	
	for(uint32_t i = 0; i < _indices.size(); ++i)
	{
		if(group_id < _groups.size() && i == _groups[group_id].indices)
		{
			if(i != 0)
				ss << "newidgrp" << endl;
			++group_id;
		}
		
		if(pos <= 1)
			ss << _indices[i] << ",";
		else if(pos == 2)
		{
			ss << _indices[i] << endl;
			pos = 0;
			continue;
		}
		
		++pos;
	}
	
	if(_bones.size())
	{
		ss << "git:" << _globalInverseTransform[0][0] << "," << _globalInverseTransform[0][1] << "," << _globalInverseTransform[0][2] << "," << _globalInverseTransform[0][3] << ",";
		ss << _globalInverseTransform[1][0] << "," << _globalInverseTransform[1][1] << "," << _globalInverseTransform[1][2] << "," << _globalInverseTransform[1][3] << ",";
		ss << _globalInverseTransform[2][0] << "," << _globalInverseTransform[2][1] << "," << _globalInverseTransform[2][2] << "," << _globalInverseTransform[2][3] << ",";
		ss << _globalInverseTransform[3][0] << "," << _globalInverseTransform[3][1] << "," << _globalInverseTransform[3][2] << "," << _globalInverseTransform[3][3] << endl;
	}
	
	ss << "bones:" << _bones.size() << endl;
	
	for(NMeshBoneInfo &bi : _bones)
	{
		ss << "name{" << bi.name << "};" << "offset{";
		ss << bi.offset[0][0] << "," << bi.offset[0][1] << "," << bi.offset[0][2] << "," << bi.offset[0][3] << ",";
		ss << bi.offset[1][0] << "," << bi.offset[1][1] << "," << bi.offset[1][2] << "," << bi.offset[1][3] << ",";
		ss << bi.offset[2][0] << "," << bi.offset[2][1] << "," << bi.offset[2][2] << "," << bi.offset[2][3] << ",";
		ss << bi.offset[3][0] << "," << bi.offset[3][1] << "," << bi.offset[3][2] << "," << bi.offset[3][3] << "};" << endl;
	}
	
	ss << "nodes:" << _nodes.size() << endl;
	
	for(NMeshTransformNodeInfo &tni : _nodes)
	{
		ss << "name(" << tni.name << ");" << "transform(";
		ss << tni.transform[0][0] << "," << tni.transform[0][1] << "," << tni.transform[0][2] << "," << tni.transform[0][3] << ",";
		ss << tni.transform[1][0] << "," << tni.transform[1][1] << "," << tni.transform[1][2] << "," << tni.transform[1][3] << ",";
		ss << tni.transform[2][0] << "," << tni.transform[2][1] << "," << tni.transform[2][2] << "," << tni.transform[2][3] << ",";
		ss << tni.transform[3][0] << "," << tni.transform[3][1] << "," << tni.transform[3][2] << "," << tni.transform[3][3] << ");";
		ss << "parent(" << tni.parentId << ");" << "childn(" << tni.childIds.size() << ");children(";
		
		for(size_t i = 0; i < tni.childIds.size(); ++i)
			ss << tni.childIds[i] << ((i == tni.childIds.size() - 1) ? "" : ",");
		
		ss << ");" << endl;
	}
	
	gzFile fp = gzopen([path UTF8String], "wb");
	gzbuffer(fp, 1048576);
	gzwrite(fp, ss.str().c_str(), (unsigned int)ss.str().size());
	gzclose(fp);
	
	return true;
}

- (void)dealloc
{
}

@end
