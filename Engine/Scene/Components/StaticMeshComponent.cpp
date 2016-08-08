/* NekoEngine
 *
 * StaticMeshComponent.cpp
 * Author: Alexandru Naiman
 *
 * Component class implementation
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

#include <Scene/Components/StaticMeshComponent.h>
#include <Scene/Components/CameraComponent.h>
#include <Engine/SceneManager.h>
#include <Engine/CameraManager.h>
#include <Engine/ResourceManager.h>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#define SMCOMPONENT_MODULE	"StaticMeshComponent"

ENGINE_REGISTER_COMPONENT_CLASS(StaticMeshComponent);

StaticMeshComponent::StaticMeshComponent(ComponentInitializer *initializer)
	: ObjectComponent(initializer)
{
	_mesh = nullptr;
	_loaded = false;
	_meshId = initializer->arguments.find("mesh")->second;
	
	ArgumentMapRangeType range = initializer->arguments.equal_range("material");
		
	for (ArgumentMapType::iterator it = range.first; it != range.second; ++it)
		_materialIds.push_back(ResourceManager::GetResourceID(it->second.c_str(), ResourceType::RES_MATERIAL));

	_renderer = Engine::GetRenderer();

	_matrixUbo = nullptr;
	_matrixBlock.Model = _parent->GetModelMatrix();
	_mmNeedsUpdate = false;
	_blend = false;
}

void StaticMeshComponent::SetLocalPosition(vec3 &position) noexcept
{
	ObjectComponent::SetLocalPosition(position);
	_translationMatrix = translate(mat4(), _localPosition);
	_mmNeedsUpdate = true;
}

void StaticMeshComponent::SetLocalRotation(vec3 &rotation) noexcept
{
	ObjectComponent::SetLocalRotation(rotation);

	mat4 rotXMatrix = rotate(mat4(), DEG2RAD(_localRotation.x), vec3(1.f, 0.f, 0.f));
	mat4 rotYMatrix = rotate(mat4(), DEG2RAD(_localRotation.y), vec3(0.f, 1.f, 0.f));
	mat4 rotZMatrix = rotate(mat4(), DEG2RAD(_localRotation.z), vec3(0.f, 0.f, 1.f));

	_rotationMatrix = rotZMatrix * rotXMatrix * rotYMatrix;
//	SetForwardDirection(_objectForward);

	_mmNeedsUpdate = true;
}

void StaticMeshComponent::SetLocalScale(vec3 &newScale) noexcept
{
	ObjectComponent::SetLocalScale(newScale);

	_scaleMatrix = scale(mat4(), newScale);
	_mmNeedsUpdate = true;
}

int StaticMeshComponent::Load()
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
			Logger::Log(SMCOMPONENT_MODULE, LOG_CRITICAL, "Failed to load material id %d", id);
			return ENGINE_INVALID_RES;
		}

		_blend |= mat->EnableBlend();
		mat->SetAnimatedMesh(0);
		_materials.push_back(mat);
	}
	
	if(!_mesh)
	{
		if(!strncmp(_meshId.c_str(), "generated", 9))
		{
			_mesh = new StaticMesh(nullptr);
			noMaterial = true; // skip material size check
		}
		else
			_mesh = (StaticMesh*)ResourceManager::GetResourceByName(_meshId.c_str(), ResourceType::RES_STATIC_MESH);
	}
    
	if (!_mesh)
	{
		Logger::Log(SMCOMPONENT_MODULE, LOG_CRITICAL, "Failed to load StaticMesh %s", _meshId.c_str());
		return ENGINE_INVALID_RES;
	}
	
	if (!noMaterial && (_materials.size() != _mesh->GetGroupCount()))
	{
		Logger::Log(SMCOMPONENT_MODULE, LOG_CRITICAL, "Failed to load StaticMesh %s. The mesh requires %d materials, but only %d are set", _meshId.c_str(), _mesh->GetGroupCount(), _materials.size());
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

void StaticMeshComponent::Draw(RShader *shader) noexcept
{
	if (!_loaded)
		return;

	_renderer->EnableDepthTest(true);

	_mesh->Bind();
	
	if (!_materials.size()) // used only for lighting pass
		_mesh->Draw(_renderer, 0);
	else
	{
		CameraComponent *cam = CameraManager::GetActiveCamera();

		if(_mmNeedsUpdate)
		{
			_matrixBlock.Model = (_translationMatrix * _rotationMatrix) * _scaleMatrix;
			_matrixBlock.Model *= _parent->GetModelMatrix();
			_mmNeedsUpdate = false;
		}	

		_matrixBlock.View = cam->GetView();
		_matrixBlock.ModelViewProjection = (cam->GetProjectionMatrix() * cam->GetView()) * _matrixBlock.Model;
		
		shader->VSSetUniformBuffer(0, 0, sizeof(MatrixBlock), _matrixUbo);
		_matrixUbo->UpdateData(0, sizeof(MatrixBlock), &_matrixBlock);
		_parent->BindUniformBuffer(shader);
        
		for (size_t i = 0; i < _materials.size(); i++)
		{
			_materials[i]->Enable(shader);
			if (_materials[i]->DisableCulling())
				Engine::GetRenderer()->EnableFaceCulling(false);

			shader->BindUniformBuffers();
			_mesh->Draw(_renderer, i);
		
			if (_materials[i]->DisableCulling())
				Engine::GetRenderer()->EnableFaceCulling(true);
		}
	}

	_mesh->Unbind();

	_renderer->EnableDepthTest(false);
}

void StaticMeshComponent::Update(double deltaTime) noexcept
{
	ObjectComponent::Update(deltaTime);
	/*if (!_loaded)
		return;*/
}

void StaticMeshComponent::Unload()
{
	if(!_loaded)
		return;
	
	ObjectComponent::Unload();

	for(Material *mat : _materials)
		ResourceManager::UnloadResource(mat->GetResourceInfo()->id, ResourceType::RES_MATERIAL);

	_materials.clear();
	_materialIds.clear();

	if (_mesh)
	{
		if (_mesh->GetResourceInfo() != nullptr)
			ResourceManager::UnloadResourceByName(_meshId.c_str(),
			_mesh->GetResourceInfo()->meshType == MeshType::Static ?
			ResourceType::RES_STATIC_MESH : ResourceType::RES_SKELETAL_MESH);
		else
			delete _mesh;

		_mesh = nullptr;
	}

	delete _matrixUbo;
	
	_loaded = false;
}

StaticMeshComponent::~StaticMeshComponent()
{
	Unload();
}
