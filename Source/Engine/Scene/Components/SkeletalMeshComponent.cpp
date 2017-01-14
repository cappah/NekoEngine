/* NekoEngine
 *
 * SkeletalMeshComponent.cpp
 * Author: Alexandru Naiman
 *
 * SkeletalMeshComponent class implementation
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

#include <Scene/Components/SkeletalMeshComponent.h>
#include <Scene/Components/AnimatorComponent.h>
#include <Scene/Components/CameraComponent.h>
#include <Engine/SceneManager.h>
#include <Engine/ResourceManager.h>

#define SK_COMPONENT_MODULE		"SkeletalMeshComponent"

using namespace glm;

ENGINE_REGISTER_COMPONENT_CLASS(SkeletalMeshComponent);

SkeletalMeshComponent::SkeletalMeshComponent(ComponentInitializer *initializer)
	: StaticMeshComponent(initializer)
{
	_mesh = nullptr;
	_animatorId = initializer->arguments.find("animator")->second;
	_animator = nullptr;
}

int SkeletalMeshComponent::Load()
{
	int ret = ObjectComponent::Load();
	if (ret != ENGINE_OK)
		return ret;

	for (int id : _materialIds)
	{
		Material* mat = (Material*)ResourceManager::GetResource(id, ResourceType::RES_MATERIAL);

		if (mat == nullptr)
		{
			Unload();
			Logger::Log(SK_COMPONENT_MODULE, LOG_CRITICAL, "Failed to load material id %d", id);
			return ENGINE_INVALID_RES;
		}

		mat->SetAnimated(true);

		_materials.Add(mat);
	}

	if (!_mesh)
	{
		if (_meshId == SM_GENERATED)
			_mesh = new SkeletalMesh(nullptr);
		else
			_mesh = (SkeletalMesh*)ResourceManager::GetResourceByName(*_meshId, ResourceType::RES_SKELETAL_MESH);
	}

	if (!_mesh)
	{
		Logger::Log(SK_COMPONENT_MODULE, LOG_CRITICAL, "Failed to load SkeletalMesh %s", *_meshId);
		return ENGINE_INVALID_RES;
	}

	StaticMeshComponent::_mesh = (StaticMesh*)_mesh;

	if ((_materials.Count() != _mesh->GetGroupCount()) && (_meshId != SM_GENERATED))
	{
		Logger::Log(SK_COMPONENT_MODULE, LOG_CRITICAL, "Failed to load SkeletalMesh %s. The mesh requires %d materials, but %d are set", *_meshId, _mesh->GetGroupCount(), _materials.Count());
		return ENGINE_INVALID_RES;
	}

	if (_meshId != SM_GENERATED)
		_parent->SetBounds(_mesh->GetBounds());

	_loaded = true;

	return ENGINE_OK;
}

int SkeletalMeshComponent::InitializeComponent()
{
	int ret = StaticMeshComponent::InitializeComponent();
	
	if(ret != ENGINE_OK)
		return ret;
	
	_animator = (AnimatorComponent*)_parent->GetComponent(_animatorId.c_str());
	if(!_animator)
		return ENGINE_INVALID_ARGS;
	
	return ENGINE_OK;
}

bool SkeletalMeshComponent::Upload(Buffer *buffer)
{
	if (!StaticMeshComponent::Upload(buffer))
		return false;

	return _mesh->Upload(buffer);
}

void SkeletalMeshComponent::Update(double deltaTime) noexcept
{
	StaticMeshComponent::Update(deltaTime);
}

void SkeletalMeshComponent::UpdateData(VkCommandBuffer commandBuffer) noexcept
{
	StaticMeshComponent::UpdateData(commandBuffer);
}

void SkeletalMeshComponent::DrawShadow(VkCommandBuffer commandBuffer, uint32_t shadowId) const noexcept
{
	ObjectComponent::DrawShadow(commandBuffer, shadowId);
	_mesh->DrawShadow(commandBuffer, shadowId, _descriptorSet);
}

bool SkeletalMeshComponent::InitDrawables()
{
	for (Drawable &drawable : _drawables)
	{
		if (drawable.depthCommandBuffer != VK_NULL_HANDLE)
			Renderer::GetInstance()->FreeMeshCommandBuffer(drawable.depthCommandBuffer);
		Renderer::GetInstance()->FreeMeshCommandBuffer(drawable.sceneCommandBuffer);
	}

	if (_descriptorPool == VK_NULL_HANDLE)
	{
		_descriptorPool = Renderer::GetInstance()->CreateAnimatedMeshDescriptorPool();
		_descriptorSet = _mesh->CreateDescriptorSet(_descriptorPool, _ubo, _animator->GetSkeletonBuffer());

		for (Material *mat : _materials)
		{
			if (!mat->HasDescriptorSet() && !mat->CreateDescriptorSet())
			{
				Logger::Log(SK_COMPONENT_MODULE, LOG_CRITICAL, "Failed to create descriptor set for material %s", mat->GetResourceInfo()->name.c_str());
				return false;
			}
		}
	}

	return _mesh->BuildDrawables(_materials, _descriptorSet, _drawables, true);
}

bool SkeletalMeshComponent::RebuildCommandBuffers()
{
	for (Drawable &drawable : _drawables)
	{
		if (drawable.depthCommandBuffer != VK_NULL_HANDLE)
			Renderer::GetInstance()->FreeMeshCommandBuffer(drawable.depthCommandBuffer);
		Renderer::GetInstance()->FreeMeshCommandBuffer(drawable.sceneCommandBuffer);
	}

	return _mesh->BuildDrawables(_materials, _descriptorSet, _drawables, false);
}

bool SkeletalMeshComponent::Unload()
{
	if (!ObjectComponent::Unload())
		return false;

	for (Drawable &drawable : _drawables)
	{
		if (drawable.depthCommandBuffer != VK_NULL_HANDLE)
			Renderer::GetInstance()->FreeMeshCommandBuffer(drawable.depthCommandBuffer);
		Renderer::GetInstance()->FreeMeshCommandBuffer(drawable.sceneCommandBuffer);
	}

	for (NString matId : _materialIds)
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
