/* NekoEngine
 *
 * StaticMeshComponent.cpp
 * Author: Alexandru Naiman
 *
 * Component class implementation
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

#include <Scene/Object.h>
#include <Scene/Components/StaticMeshComponent.h>
#include <Engine/CameraManager.h>
#include <Engine/ResourceManager.h>
#include <System/Logger.h>

using namespace glm;
using namespace std;

#define SM_COMPONENT_MODULE		"StaticMeshComponent"

ENGINE_REGISTER_COMPONENT_CLASS(StaticMeshComponent);

StaticMeshComponent::StaticMeshComponent(ComponentInitializer *initializer)
	: ObjectComponent(initializer)
{
	_ubo = nullptr;
	_mesh = nullptr;

	_loaded = false;
	_blend = false;
	_updateModelMatrix = false;

	_descriptorPool = VK_NULL_HANDLE;
	_descriptorSet = VK_NULL_HANDLE;

	_rotationQuaternion = quat();
	_translationMatrix = mat4();
	_scaleMatrix = mat4();

	_meshId = initializer->arguments.find("mesh")->second;

	memset(&_objectData, 0x0, sizeof(ObjectData));
	
	ArgumentMapRangeType range = initializer->arguments.equal_range("material");
		
	for (ArgumentMapType::iterator it = range.first; it != range.second; ++it)
		_materialIds.push_back(ResourceManager::GetResourceID(it->second.c_str(), ResourceType::RES_MATERIAL));

	SetPosition(_position);
	SetRotation(_rotation);
	SetScale(_scale);
}

void StaticMeshComponent::SetPosition(vec3 &position) noexcept
{
	ObjectComponent::SetPosition(position);

	_translationMatrix = translate(mat4(), _position);
	_updateModelMatrix = true;
}

void StaticMeshComponent::SetRotation(vec3 &rotation) noexcept
{
	ObjectComponent::SetRotation(rotation);
	
	_rotationQuaternion = rotate(quat(), radians(rotation));
	_updateModelMatrix = true;
}

void StaticMeshComponent::SetScale(vec3 &newScale) noexcept
{
	ObjectComponent::SetScale(newScale);

	_scaleMatrix = scale(mat4(), newScale);
	_updateModelMatrix = true;
}

int StaticMeshComponent::Load()
{
	int ret = ObjectComponent::Load();
	if(ret != ENGINE_OK)
		return ret;
	
	for (int id : _materialIds)
	{
		Material *mat = (Material *)ResourceManager::GetResource(id, ResourceType::RES_MATERIAL);

		if (mat == nullptr)
		{
			Unload();
			Logger::Log(SM_COMPONENT_MODULE, LOG_CRITICAL, "Failed to load material id %d", id);
			return ENGINE_INVALID_RES;
		}

		_materials.Add(mat);
	}
	
	if(!_mesh)
	{
		if(_meshId == SM_GENERATED)
			_mesh = new StaticMesh(nullptr);
		else if (_meshId.Contains("pr_"))
		{
			// Primitive mesh
		}
		else
			_mesh = (StaticMesh *)ResourceManager::GetResourceByName(*_meshId, ResourceType::RES_STATIC_MESH);
	}

	if (!_mesh)
	{
		Logger::Log(SM_COMPONENT_MODULE, LOG_CRITICAL, "Failed to load StaticMesh %s", *_meshId);
		return ENGINE_INVALID_RES;
	}
	
	if ((_materials.Count() != _mesh->GetGroupCount()) && (_meshId != SM_GENERATED))
	{
		Logger::Log(SM_COMPONENT_MODULE, LOG_CRITICAL, "Failed to load StaticMesh %s. The mesh requires %d materials, but %d are set", *_meshId, _mesh->GetGroupCount(), _materials.Count());
		return ENGINE_INVALID_RES;
	}

	_loaded = true;

	if (_meshId != SM_GENERATED)
		_parent->SetBounds(_mesh->GetBounds());

	_objectData.objectId = _parent->GetId();

	return ENGINE_OK;
}

bool StaticMeshComponent::Upload(Buffer *buffer)
{
	if (!ObjectComponent::Upload(buffer))
		return false;

	return _mesh->Upload(buffer);
}

void StaticMeshComponent::Update(double deltaTime) noexcept
{
	ObjectComponent::Update(deltaTime);
}

void StaticMeshComponent::UpdatePosition() noexcept
{
	ObjectComponent::UpdatePosition();
	_updateModelMatrix = true;
}

void StaticMeshComponent::UpdateData(VkCommandBuffer commandBuffer) noexcept
{
	ObjectComponent::UpdateData(commandBuffer);

	if (_updateModelMatrix)
		_UpdateModelMatrix();

	Camera *cam = CameraManager::GetActiveCamera();
	_objectData.modelViewProjection = cam->GetProjectionMatrix() * (cam->GetView() * _objectData.model);

	_ubo->UpdateData((uint8_t *)&_objectData, 0, sizeof(_objectData), commandBuffer);
}

void StaticMeshComponent::DrawShadow(VkCommandBuffer commandBuffer, uint32_t shadowId) const noexcept
{
	ObjectComponent::DrawShadow(commandBuffer, shadowId);
	_mesh->DrawShadow(commandBuffer, shadowId, _descriptorSet);
}

bool StaticMeshComponent::Unload()
{
	if(!ObjectComponent::Unload())
		return false;
	
	for (Drawable &drawable : _drawables)
	{
		if (drawable.depthCommandBuffer != VK_NULL_HANDLE)
			Renderer::GetInstance()->FreeMeshCommandBuffer(drawable.depthCommandBuffer);
		Renderer::GetInstance()->FreeMeshCommandBuffer(drawable.sceneCommandBuffer);
	}

	for(NString matId : _materialIds)
		ResourceManager::UnloadResourceByName(*matId, ResourceType::RES_MATERIAL);

	_materials.Clear();
	_materialIds.clear();

	if (_mesh && _meshId != SM_GENERATED)
	{
		ResourceManager::UnloadResourceByName(*_meshId, _mesh->GetResourceInfo()->meshType == MeshType::Static ?
			ResourceType::RES_STATIC_MESH : ResourceType::RES_SKELETAL_MESH);
	}
	else
		delete _mesh;

	_mesh = nullptr;

	Renderer::GetInstance()->FreeMeshDescriptorPool(_descriptorPool);

	_loaded = false;

	return true;
}

bool StaticMeshComponent::InitDrawables()
{
	bool buildDepth = true;

	for (Drawable &drawable : _drawables)
	{
		if (drawable.depthCommandBuffer != VK_NULL_HANDLE)
			Renderer::GetInstance()->FreeMeshCommandBuffer(drawable.depthCommandBuffer);
		Renderer::GetInstance()->FreeMeshCommandBuffer(drawable.sceneCommandBuffer);
	}

	if (_materials[0]->GetType() == MT_Skysphere)
		buildDepth = false;

	if (_descriptorPool == VK_NULL_HANDLE)
	{
		_descriptorPool = Renderer::GetInstance()->CreateMeshDescriptorPool();
		_descriptorSet = _mesh->CreateDescriptorSet(_descriptorPool, _ubo);

		for (Material *mat : _materials)
		{
			if (!mat->HasDescriptorSet() && !mat->CreateDescriptorSet())
			{
				Logger::Log(SM_COMPONENT_MODULE, LOG_CRITICAL, "Failed to create descriptor set for material %s", mat->GetResourceInfo()->name.c_str());
				return false;
			}
		}
	}

	_SortGroups();

	return _mesh->BuildDrawables(_materials, _descriptorSet, _drawables, buildDepth, true);
}

bool StaticMeshComponent::RebuildCommandBuffers()
{
	bool buildDepth = true;

	for (Drawable &drawable : _drawables)
	{
		if (drawable.depthCommandBuffer != VK_NULL_HANDLE)
			Renderer::GetInstance()->FreeMeshCommandBuffer(drawable.depthCommandBuffer);
		Renderer::GetInstance()->FreeMeshCommandBuffer(drawable.sceneCommandBuffer);
	}

	if (_materials[0]->GetType() == MT_Skysphere)
		buildDepth = false;

	return _mesh->BuildDrawables(_materials, _descriptorSet, _drawables, buildDepth, false);
}

void StaticMeshComponent::_SortGroups()
{
	struct GroupInfo
	{
		MeshGroup group;
		Material *mat;
	};

	vector<GroupInfo> opaque;
	vector<GroupInfo> transparent;

	for(uint32_t i = 0; i < _materials.Count(); ++i)
	{
		if (_materials[i]->IsTransparent())
			transparent.push_back({ _mesh->GetGroup(i), _materials[i] });
		else
			opaque.push_back({ _mesh->GetGroup(i), _materials[i] });
	}
	
	_materials.Clear();
	_mesh->ResetGroups();

	for (GroupInfo &gi : opaque)
	{
		_materials.Add(gi.mat);
		_mesh->AddGroup(gi.group);
	}

	for (GroupInfo &gi : transparent)
	{
		_materials.Add(gi.mat);
		_mesh->AddGroup(gi.group);
	}
}

void StaticMeshComponent::_UpdateModelMatrix()
{
	_objectData.model = ((_translationMatrix * mat4_cast(_rotationQuaternion)) * _scaleMatrix) * _parent->GetModelMatrix();
	_objectData.normal = transpose(inverse(_objectData.model));

	for (Drawable &drawable : _drawables)
		drawable.bounds.Transform(_objectData.model, &drawable.transformedBounds);

	_updateModelMatrix = false;
}