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

static ObjectInitializer _objDefaultInitializer =
{
	7000,
	"unnamed",
	{ 0.f, 0.f, 0.f },
	{ 0.f, 0.f, 0.f },
	{ 1.f, 1.f, 1.f },
	{ 0.f, 0.f, 0.f }
};

ENGINE_REGISTER_OBJECT_CLASS(Object)

Object::Object(ObjectInitializer *initializer) noexcept
{
	if(!initializer)
	{
		Logger::Log(OBJ_MODULE, LOG_WARNING, "No initializer supplied, using default values");
		
		initializer = &_objDefaultInitializer;
		++_objDefaultInitializer.id;
	}
	
	std::vector<Texture *> _Textures;
	std::vector<int> _TextureIds;
	std::vector<TextureParams> _TextureParams;
	_id = -1;
	_translationMatrix = mat4();
	_rotationMatrix = mat4();
	_scaleMatrix = mat4();
	_modelMatrix = mat4();
	_loaded = false;
	
	_renderer = Engine::GetRenderer();

	memset(&_objectBlock, 0x0, sizeof(_objectBlock));

	_objectUbo = nullptr;
	
	SetForwardDirection(ForwardDirection::PositiveZ);
	SetPosition(initializer->position);
	SetRotation(initializer->rotation);
	SetScale(initializer->scale);
	SetColor(initializer->color);
	
	_id = initializer->id;
	
	ArgumentMapType::iterator it = initializer->arguments.find("type");

	if (it == initializer->arguments.end())
		return;

	const char *type = it->second.c_str();
	if(!type)
		return;
	
	size_t len = strlen(type);
	
	if(!strncmp(type, "posz", len))
		SetForwardDirection(ForwardDirection::PositiveZ);
	else if(!strncmp(type, "negz", len))
		SetForwardDirection(ForwardDirection::NegativeZ);
	else if(!strncmp(type, "posx", len))
		SetForwardDirection(ForwardDirection::PositiveX);
	else if(!strncmp(type, "negx", len))
		SetForwardDirection(ForwardDirection::NegativeX);
	else
		SetForwardDirection(ForwardDirection::PositiveZ);
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
	if((_objectUbo = _renderer->CreateBuffer(BufferType::Uniform, true, false)) == nullptr)
	{
		Unload();
		return ENGINE_OUT_OF_RESOURCES;
	}
	_objectUbo->SetStorage(sizeof(ObjectBlock), &_objectBlock);
	
	for(pair<string, ObjectComponent*> kvp : _components)
	{
		int ret = kvp.second->InitializeComponent();
		if(ret != ENGINE_OK)
			return ret;
	}

	_loaded = true;

	return ENGINE_OK;
}

void Object::Draw(RShader* shader) noexcept
{
	if (!_loaded)
		return;

	_objectUbo->UpdateData(0, sizeof(vec3), &SceneManager::GetActiveScene()->GetSceneCamera()->GetPosition().x);

	for (pair<string, ObjectComponent*> kvp : _components)
		kvp.second->Draw(shader);
}

void Object::Update(double deltaTime) noexcept
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

	delete _objectUbo;
	
	for(pair<string, ObjectComponent*> kvp : _components)
		kvp.second->Unload();
	
	_loaded = false;
}

void Object::AddComponent(const char *name, ObjectComponent *comp)
{
	_components.insert({ name, comp });
}

Object::~Object() noexcept
{
	Unload();
}
