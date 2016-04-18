/* Neko Engine
 *
 * Object.cpp
 * Author: Alexandru Naiman
 *
 * Object class implementation 
 *
 * ----------------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman <alexandru dot naiman at icloud dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ALEXANDRU NAIMAN
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define ENGINE_INTERNAL

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Engine/Engine.h>
#include <Engine/ResourceManager.h>
#include <Engine/SceneManager.h>
#include <Scene/Object.h>

#define OBJ_MODULE	"Object"

using namespace std;
using namespace glm;

ENGINE_REGISTER_OBJECT_CLASS(Object)

Object::Object() noexcept
{
	_mesh = nullptr;
	std::vector<Texture *> _Textures;
	std::vector<int> _TextureIds;
	std::vector<TextureParams> _TextureParams;
	_id = -1;
	_modelId = -1;
	_translationMatrix = mat4();
	_rotationMatrix = mat4();
	_scaleMatrix = mat4();
	_blend = false;
	_loaded = false;
	SetForwardDirection(ForwardDirection::PositiveZ);
	_renderer = Engine::GetRenderer();

	memset(&_objectBlock, 0x0, sizeof(_objectBlock));
	memset(&_matrixBlock, 0x0, sizeof(_matrixBlock));

	_objectUbo = _matrixUbo = nullptr;
}

void Object::SetPosition(vec3 &position) noexcept
{
	_position = position;
	_translationMatrix = translate(mat4(), _position);
	_UpdateModelMatrix();
}

void Object::SetRotation(vec3 &rotation) noexcept
{
	_rotation = rotation;

	mat4 rotXMatrix = rotate(mat4(), DEG2RAD(_rotation.x), vec3(1.f, 0.f, 0.f));
	mat4 rotYMatrix = rotate(mat4(), DEG2RAD(_rotation.y), vec3(0.f, 1.f, 0.f));
	mat4 rotZMatrix = rotate(mat4(), DEG2RAD(_rotation.z), vec3(0.f, 0.f, 1.f));

	_rotationMatrix = rotZMatrix * rotXMatrix * rotYMatrix;
	SetForwardDirection(_objectForward);

	_UpdateModelMatrix();
}

void Object::SetScale(vec3 &newScale) noexcept
{
	_scale = newScale;
	_scaleMatrix = scale(mat4(), newScale);
	_UpdateModelMatrix();
}

void Object::SetForwardDirection(ForwardDirection dir) noexcept
{
	switch (dir)
	{
	case ForwardDirection::PositiveZ:
	{
		_forward = vec3(0.f, 0.f, 1.f);
		_right = vec3(-1.f, 0.f, 0.f);
	}
	break;
	case ForwardDirection::NegativeZ:
	{
		_forward = vec3(0.f, 0.f, -1.f);
		_right = vec3(1.f, 0.f, 0.f);
	}
	break;
	case ForwardDirection::PositiveX:
	{
		_forward = vec3(1.f, 0.f, 0.f);
		_right = vec3(0.f, 0.f, 1.f);
	}
	break;
	case ForwardDirection::NegativeX:
	{
		_forward = vec3(-1.f, 0.f, 0.f);
		_right = vec3(0.f, 0.f, -1.f);
	}
	break;
	}
	
	_objectForward = dir;

	vec4 fwd = vec4(_forward, 1.f) * _rotationMatrix;
	vec4 right = vec4(_right, 1.f) * _rotationMatrix;

	_forward = vec3(fwd.x, fwd.y, fwd.z);
	_right = vec3(right.x, right.y, right.z);
}

void Object::LookAt(vec3 &point) noexcept
{
	vec3 fwd = normalize(point - _position);
	vec3 dirFwd = vec3(0.f, 0.f, 1.f);
	float dotFwd = dot(dirFwd, fwd);
	float angle = 0.f;
	vec3 axis;

	if (abs(dotFwd - (-1.f)) < 0.000001f)
	{
		axis = vec3(0.f, 1.f, 0.f);
		angle = (float)M_PI;
	}
	else if (abs(dotFwd - 1.f) < 0.000001f)
	{
		axis = vec3(0.f, 0.f, 0.f);
		angle = 0.f;
	}
	else
	{
		angle = acosf(dotFwd);
		axis = normalize(cross(dirFwd, fwd));
	}

	vec3 rotation = vec3(RAD2DEG(angle * axis.x), RAD2DEG(angle * axis.y), RAD2DEG(angle * axis.z));
	SetRotation(rotation);
}

void Object::MoveForward(float distance) noexcept
{
	_position += _forward * distance;
	SetPosition(_position);
}

void Object::MoveRight(float distance) noexcept
{
	_position += _right * distance;
	SetPosition(_position);
}

int Object::Load()
{
	bool noMaterial = false;

	for (int id : _materialIds)
	{
		if (id == OBJ_NO_MATERIAL)
		{
			noMaterial = true;
			break;
		}

		Material* mat = (Material*)ResourceManager::GetResource(id, ResourceType::RES_MATERIAL);

		if (mat == nullptr)
		{
			Unload();
		    Logger::Log(OBJ_MODULE, LOG_CRITICAL, "Failed to load material id %d for object id %d", id, _id);
			return ENGINE_INVALID_RES;
		}

		_blend |= mat->EnableBlend();
		_materials.push_back(mat);
	}

	if(!_mesh)
		_mesh = (Mesh*)ResourceManager::GetResource(_modelId, ResourceType::RES_MESH);

	if (!_mesh)
	{
		Logger::Log(OBJ_MODULE, LOG_CRITICAL, "Failed to load mesh for object id %d", _id);
		return ENGINE_INVALID_RES;
	}

	if (!noMaterial && (_materials.size() != _mesh->GetGroupCount()))
	{
		Logger::Log(OBJ_MODULE, LOG_CRITICAL, "Failed to load mesh for object id=%d. The mesh requires %d materials, but only %d are set", _id, _mesh->GetGroupCount(), _materials.size());
		return ENGINE_INVALID_RES;
	}

	_objectUbo = _renderer->CreateBuffer(BufferType::Uniform, true, false);
	_objectUbo->SetStorage(sizeof(ObjectBlock), &_objectBlock);

	_matrixUbo = _renderer->CreateBuffer(BufferType::Uniform, true, false);
	_matrixUbo->SetStorage(sizeof(ObjectMatrixBlock), nullptr);

	_loaded = true;

	return ENGINE_OK;
}

void Object::PreDraw(RShader* shader, size_t i) noexcept
{
	_materials[i]->Enable(shader);

	/*if (_materials[i]->EnableBlend())
	{
		GL_CHECK(glEnable(GL_BLEND));
		GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}*/

	if (_materials[i]->DisableCulling())
		Engine::GetRenderer()->EnableFaceCulling(false);
}

void Object::Draw(RShader* shader) noexcept
{
	if (!_loaded)
		return;

	Renderer* r = Engine::GetRenderer();

	r->EnableDepthTest(true);

	_mesh->Bind();

	if (!_materials.size()) // used only for lighting pass
		r->DrawElementsBaseVertex(PolygonMode::Triangles, (int32_t)_mesh->GetIndexCount(0), ElementType::UnsignedInt, (void *)(_mesh->GetIboOffset(0) * sizeof(unsigned int)), (uint32_t)_mesh->GetVboOffset());
	else
	{
		Camera *cam = SceneManager::GetActiveScene()->GetSceneCamera();

		_matrixBlock.View = cam->GetView();
		_matrixBlock.ModelViewProjection = (cam->GetProjectionMatrix() * cam->GetView()) * _matrixBlock.Model;
		
		shader->VSSetUniformBuffer(0, 0, sizeof(ObjectMatrixBlock), _matrixUbo);
		_matrixUbo->UpdateData(0, sizeof(ObjectMatrixBlock), &_matrixBlock);
		
		shader->FSSetUniformBuffer(0, 0, sizeof(ObjectBlock), _objectUbo);
		_objectUbo->UpdateData(0, sizeof(vec3), &cam->GetPosition().x);

		for (size_t i = 0; i < _materials.size(); i++)
		{
			PreDraw(shader, i);

			shader->BindUniformBuffers();
			r->DrawElementsBaseVertex(PolygonMode::Triangles, (int32_t)_mesh->GetIndexCount(i), ElementType::UnsignedInt, (void *)(_mesh->GetIboOffset(i) * sizeof(unsigned int)), (uint32_t)_mesh->GetVboOffset());
		
			PostDraw(i);
		}
	}

	_mesh->Unbind();

	r->EnableDepthTest(false);
}

void Object::PostDraw(size_t i) noexcept
{
	/*if (_materials[i]->EnableBlend())
	{ GL_CHECK(glDisable(GL_BLEND)); }*/

	if (_materials[i]->DisableCulling())
		Engine::GetRenderer()->EnableFaceCulling(true);
}

void Object::Update(float deltaTime) noexcept
{
	if (!_loaded)
		return;

	for (pair<string, ObjectComponent*> kvp : _components)
		kvp.second->Update(deltaTime);
}

void Object::Unload() noexcept
{
	if(!_loaded)
		return;

	for(Material *mat : _materials)
		ResourceManager::UnloadResource(mat->GetResourceInfo()->id, ResourceType::RES_MATERIAL);

	_materials.clear();
	_materialIds.clear();

	if (_mesh != nullptr)
	{
		if (_mesh->GetResourceInfo() != nullptr)
			ResourceManager::UnloadResource(_mesh->GetResourceInfo()->id, ResourceType::RES_MESH);
		else
			delete _mesh;

		_mesh = nullptr;
	}

	delete _objectUbo;
	delete _matrixUbo;
	
	_loaded = false;
}

Object::~Object() noexcept
{
	Unload();
}
