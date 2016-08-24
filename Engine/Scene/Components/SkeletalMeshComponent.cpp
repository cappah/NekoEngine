/* NekoEngine
 *
 * SkeletalMeshComponent.cpp
 * Author: Alexandru Naiman
 *
 * SkeletalMeshComponent class implementation
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

#include <Scene/Components/SkeletalMeshComponent.h>
#include <Scene/Components/AnimatorComponent.h>
#include <Scene/Components/CameraComponent.h>
#include <Engine/SceneManager.h>
#include <Engine/ResourceManager.h>

#define SKCOMPONENT_MODULE	"SkeletalMeshComponent"

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
	
	if(ret != ENGINE_OK)
		return ret;
	
	bool noMaterial = _materialIds.size() == 0;

	for (int id : _materialIds)
	{
		Material* mat = (Material*)ResourceManager::GetResource(id, ResourceType::RES_MATERIAL);

		if (mat == nullptr)
		{
			Unload();
			Logger::Log(SKCOMPONENT_MODULE, LOG_CRITICAL, "Failed to load material id %d", id);
			return ENGINE_INVALID_RES;
		}

		_blend |= mat->EnableBlend();
		mat->SetAnimatedMesh(0);
		_materials.push_back(mat);
	}
	
	if(!_mesh)
		_mesh = (SkeletalMesh*)ResourceManager::GetResourceByName(_meshId.c_str(), ResourceType::RES_SKELETAL_MESH);

	if (!_mesh)
	{
		Logger::Log(SKCOMPONENT_MODULE, LOG_CRITICAL, "Failed to load SkeletalMesh %s", _meshId.c_str());
		return ENGINE_INVALID_RES;
	}
	
	StaticMeshComponent::_mesh = (StaticMesh*)_mesh;
	
	if (!noMaterial && (_materials.size() != _mesh->GetGroupCount()))
	{
		Logger::Log(SKCOMPONENT_MODULE, LOG_CRITICAL, "Failed to load SkeletalMesh %s. The mesh requires %d materials, but only %d are set", _meshId.c_str(), _mesh->GetGroupCount(), _materials.size());
		return ENGINE_INVALID_RES;
	}
	
	if((_matrixUbo = _renderer->CreateBuffer(BufferType::Uniform, true, false)) == nullptr)
	{
		Unload();
		return ENGINE_OUT_OF_RESOURCES;
	}
	_matrixUbo->SetStorage(sizeof(MatrixBlock), nullptr);

	_loaded = true;

	return ENGINE_OK;
}

int SkeletalMeshComponent::InitializeComponent()
{
	int ret = ObjectComponent::InitializeComponent();
	
	if(ret != ENGINE_OK)
		return ret;
	
	_animator = (AnimatorComponent*)_parent->GetComponent(_animatorId.c_str());
	if(!_animator)
		return ENGINE_INVALID_ARGS;
	
	return ENGINE_OK;
}

void SkeletalMeshComponent::Draw(RShader *shader, Camera *camera) noexcept
{
	if (!_loaded)
		return;
	
	_animator->BindSkeleton(shader);
	
	StaticMeshComponent::Draw(shader, camera);
}

void SkeletalMeshComponent::Update(double deltaTime) noexcept
{
	if (!_loaded)
		return;

	StaticMeshComponent::Update(deltaTime);
}

bool SkeletalMeshComponent::Unload()
{
	return StaticMeshComponent::Unload();
}
