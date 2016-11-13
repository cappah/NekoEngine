/* NekoEngine
 *
 * MovingObject.h
 * Author: Alexandru Naiman
 *
 * MovingObject class definition 
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

#if ENABLE_SPONZA

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace tinyobj;

#endif

#include <algorithm>

#include "Sponza.h"
#include <System/Logger.h>

using namespace std;
using namespace glm;

REGISTER_OBJECT_CLASS(Sponza);

static inline Texture *_LoadTextureFromFile(const char *file)
{
#if ENABLE_SPONZA
	int texChannels, width, height;
	stbi_uc *pixels = stbi_load(file, (int *)&width, (int *)&height, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = width * height * 4;
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

	int mipLevels = (int)floor(std::log2(std::max(width, height))) + 1;

	Texture *tex = new Texture(width, height, mipLevels, format, imageSize, pixels);
	if (tex) tex->GenerateMipmaps();

	free(pixels);

	return tex;
#else
	return nullptr;
#endif
}

Sponza::Sponza(ObjectInitializer *initializer) noexcept : Object(initializer)
{
	
}

int Sponza::Load()
{
	int ret = Object::Load();
	if (ret != ENGINE_OK)
		return ret;

#if ENABLE_SPONZA

	_position = vec3(0.f, 0.f, 0.f);
	_rotation = vec3(0.f, 0.f, 0.f);
	_scale = vec3(.1f, .1f, .1f);
	_id = 15000;

	ComponentInitializer ci = {};
	ci.parent = this;
	ci.arguments.insert(make_pair("mesh", SM_GENERATED));

	_meshComponent = (StaticMeshComponent *)Engine::NewComponent("StaticMeshComponent", &ci);
	_meshComponent->Load();

	AddComponent("SponzaMesh", _meshComponent);

	_LoadOBJ();

	_meshComponent->LoadStatic(_vertices, _indices, false);

	for (pair<string, Material *> kvp : _materials)
		kvp.second->CreateDescriptorSet();

	for (GroupInfo &gi : _groups)
		_meshComponent->AddGroup(gi.offset, gi.count, gi.mat);

	_meshComponent->Upload();

#endif

	return ENGINE_OK;
}

Sponza::~Sponza() noexcept
{
#if ENABLE_SPONZA
	for (pair<string, Material *> kvp : _materials)
		delete kvp.second;

	for (Texture *tex : _textures)
		delete tex;
#endif
}

bool Sponza::_LoadOBJ()
{
#if ENABLE_SPONZA
	attrib_t attrib;
	vector<shape_t> shapes;
	vector<material_t> materials;
	string err;

	if (!LoadObj(&attrib, &shapes, &materials, &err, "sponza/sponza.obj", "sponza/"))
	{
		Logger::Log("Sponza", LOG_CRITICAL, "Failed to load OBJ");
		return false;
	}

	_vertices.reserve(1000000);
	_indices.reserve(1000000);

	for (shape_t &shape : shapes)
	{
		GroupInfo gi;
		gi.offset = (uint32_t)_indices.size();
		gi.count = 0;

		for (index_t index : shape.mesh.indices)
		{
			Vertex v = {};

			v.position =
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			v.uv =
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			v.normal =
			{
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

			_vertices.push_back(v);
			_indices.push_back((uint32_t)_indices.size());
			++gi.count;
		}

		material_t &mat = materials[shape.mesh.material_ids[0]];

		if (_materials.find(mat.name) == _materials.end())
		{
			MaterialData mtlData = {};
			memcpy(&mtlData.Diffuse.x, &mat.diffuse[0], sizeof(float) * 3);
			memcpy(&mtlData.Specular.x, &mat.specular[0], sizeof(float) * 3);
			memcpy(&mtlData.Emission.x, &mat.emission[0], sizeof(float) * 3);
			mtlData.Shininess = mat.shininess * 4;
			mtlData.IndexOfRefraction = mat.ior;

			if (mat.illum == 0)
				mtlData.Type = MT_Phong;
			else if (mat.illum == 1)
				mtlData.Type = MT_NormalPhong;
			else if (mat.illum == 2)
				mtlData.Type = MT_NormalPhongSpecular;

			Material *mtl = new Material(mtlData);
			Texture *tex;

			string texPath = "sponza/";
			texPath.append(mat.diffuse_texname);

			tex = _LoadTextureFromFile(texPath.c_str());
			mtl->SetDiffuseTexture(tex);
			_textures.push_back(tex);

			texPath = "sponza/";
			if (mat.normal_texname.length())
			{
				texPath.append(mat.normal_texname);
				tex = _LoadTextureFromFile(texPath.c_str());
				mtl->SetNormalTexture(tex);
				_textures.push_back(tex);
			}

			texPath = "sponza/";
			if (mat.specular_texname.length())
			{
				texPath.append(mat.specular_texname);
				tex = _LoadTextureFromFile(texPath.c_str());
				mtl->SetSpecularTexture(tex);
				_textures.push_back(tex);
			}

			texPath = "sponza/";
			if (mat.emissive_texname.length())
			{
				texPath.append(mat.emissive_texname);
				tex = _LoadTextureFromFile(texPath.c_str());
				mtl->SetEmissionTexture(tex);
				_textures.push_back(tex);
			}
			
			_materials.insert(make_pair(mat.name, mtl));
		}

		gi.mat = _materials[mat.name];
		_groups.push_back(gi);
	}

#endif

	return true;
}
