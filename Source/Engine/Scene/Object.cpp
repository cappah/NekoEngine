/* NekoEngine
 *
 * Object.cpp
 * Author: Alexandru Naiman
 *
 * Object class implementation 
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

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

#include <Engine/Engine.h>
#include <Engine/EventManager.h>
#include <Engine/ResourceManager.h>
#include <Engine/EventManager.h>
#include <Scene/Object.h>
#include <Scene/SceneManager.h>
#include <Scene/CameraManager.h>
#include <System/Logger.h>

#define OBJ_MODULE	"Object"

using namespace std;
using namespace glm;

static ObjectInitializer _objDefaultInitializer;
static uint32_t _nextObjectId{ 0 };

ENGINE_REGISTER_OBJECT_CLASS(Object)

ObjectInitializer::ObjectInitializer() :
	id(_nextObjectId),
	name("unnamed"),
	position(0.f),
	rotation(0.f),
	scale(1.f)
{
	name = *NString::StringWithFormat(20, "unnamed_%d", _nextObjectId++);
}

Object::Object(ObjectInitializer *initializer) noexcept
{
	if(!initializer)
	{
		Logger::Log(OBJ_MODULE, LOG_WARNING, "No initializer supplied, using default values");
		
		initializer = &_objDefaultInitializer;
		++_objDefaultInitializer.id;
	}
	
	_id = -1;
	_rotationQuaternion = quat();
	_translationMatrix = mat4();
	_scaleMatrix = mat4();
	_loaded = false;
	_updateWhilePaused = false;
	_noCull = false;
	_buffer = nullptr;
	_updateModelMatrix = false;
	_haveMesh = false;
	_visible = true;

	SetForwardDirection(ForwardDirection::PositiveZ);
	SetPosition(initializer->position);
	SetRotation(initializer->rotation);
	SetScale(initializer->scale);

	_id = initializer->id;
	_name = initializer->name;

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
	_updateModelMatrix = true;

	EventManager::Broadcast(NE_EVT_OBJ_MOVED, this);
}

void Object::SetRotation(vec3 &rotation) noexcept
{
	_rotation = rotation;
	_rotationQuaternion = rotate(quat(), radians(rotation));

	SetForwardDirection(_objectForward);

	_updateModelMatrix = true;
}

void Object::SetScale(vec3 &newScale) noexcept
{
	_scale = newScale;
	_scaleMatrix = scale(mat4(), newScale);
	_updateModelMatrix = true;
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

	mat4 rotationMatrix = mat4_cast(_rotationQuaternion);

	vec4 fwd = vec4(_forward, 1.f) * rotationMatrix;
	vec4 right = vec4(_right, 1.f) * rotationMatrix;

	_forward = vec3(fwd.x, fwd.y, fwd.z);
	_right = vec3(right.x, right.y, right.z);
}

void Object::SetBounds(const NBounds &bounds) noexcept
{
	_bounds = bounds;
	_bounds.Transform(_modelMatrix, &_transformedBounds);
}

void Object::LookAt(const vec3 &point) noexcept
{
	vec3 fwd = normalize(point - _position);
	vec3 dirFwd = vec3(0.f, 0.f, 1.f);
	float dotFwd = dot(dirFwd, fwd);
	float angle = 0.f;
	vec3 axis;

/*	if (abs(dotFwd - (-1.f)) < 0.000001f)
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
	{*/
		angle = acosf(dotFwd);
		axis = normalize(cross(dirFwd, fwd));
//	}
	
	vec3 rotation{ vec3(radians(axis.x * angle), axis.y * angle, radians(axis.y * angle)) };
	vec3 degRotation{ degrees(rotation) };
	SetRotation(degRotation);
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

size_t Object::GetVertexCount() noexcept
{
	size_t ret = 0;
	for(pair<string, ObjectComponent*> kvp : _components)
		ret += kvp.second->GetVertexCount();
	return ret;
}

size_t Object::GetTriangleCount() noexcept
{
	size_t ret = 0;
	for(pair<string, ObjectComponent*> kvp : _components)
		ret += kvp.second->GetTriangleCount();
	return ret;
}

int Object::Load()
{
	for(pair<string, ObjectComponent*> kvp : _components)
	{
		int ret = kvp.second->InitializeComponent();
		if(ret != ENGINE_OK)
			return ret;
	}

	_UpdateModelMatrix();

	_loaded = true;

	return ENGINE_OK;
}

int Object::CreateBuffers()
{
	int ret = ENGINE_OK;

	for (pair<string, ObjectComponent*> kvp : _components)
		ret = kvp.second->CreateBuffers();

	return ret;
}

void Object::FixedUpdate() noexcept
{
	if (!_loaded)
		return;

	/*for (pair<string, ObjectComponent*> kvp : _components)
		if (kvp.second->IsEnabled()) kvp.second->Update(deltaTime);*/
}

void Object::Update(double deltaTime) noexcept
{
	if (!_loaded)
		return;
	
	for (pair<string, ObjectComponent*> kvp : _components)
		if (kvp.second->IsEnabled()) kvp.second->Update(deltaTime);
}

bool Object::Unload() noexcept
{
	if(!_loaded)
		return false;

	delete _buffer;
	
	for(pair<string, ObjectComponent*> kvp : _components)
	{
		kvp.second->Unload();
		delete kvp.second;
	}
	_components.clear();

	_loaded = false;

	return true;
}

bool Object::CanUnload() noexcept
{
	bool ret = true;
	for (pair<string, ObjectComponent *> kvp : _components)
		ret = ret && kvp.second->CanUnload();
	return ret;
}

void Object::OnHit(Object *other, glm::vec3 &position)
{
	if (!_loaded)
		return;

	for (pair<string, ObjectComponent*> kvp : _components)
		if (kvp.second->IsEnabled()) kvp.second->OnHit(other, position);
}

void Object::AddComponent(const char *name, ObjectComponent *comp)
{
	_components.insert({ name, comp });
}

bool Object::RemoveComponent(const char *name, bool force)
{
	ObjectComponent *comp = _components[name];

	if (comp->CanUnload() || force)
	{
		_components.erase(name);
		delete comp;
		return true;
	}

	return false;
}

void Object::AddToScene()
{
	Scene *s = SceneManager::GetLoadingScene();
	if (!s)
		s = SceneManager::GetActiveScene();

	s->AddObject(this);
}

void Object::Destroy()
{
	Scene *s = SceneManager::GetActiveScene();
	if (!s)
		return;
	s->RemoveObject(this);
}

bool Object::BuildCommandBuffers()
{
	for (std::pair<std::string, ObjectComponent *> kvp : _components)
		if (!kvp.second->InitDrawables())
			return false;

	_haveMesh = true;
	_UpdateModelMatrix();

	return true;
}

bool Object::RebuildCommandBuffers()
{
	for (std::pair<std::string, ObjectComponent *> kvp : _components)
		if (!kvp.second->RebuildCommandBuffers())
			return false;
	return true;
}

VkDeviceSize Object::GetRequiredMemorySize()
{
	VkDeviceSize size = 0;
	for (std::pair<std::string, ObjectComponent *> kvp : _components)
		size += kvp.second->GetRequiredMemorySize();
	return size;
}

void Object::DrawShadow(VkCommandBuffer commandBuffer, uint32_t shadowId) const noexcept
{
	for (std::pair<std::string, ObjectComponent *> kvp : _components)
		kvp.second->DrawShadow(commandBuffer, shadowId);
}

void Object::UpdateData(VkCommandBuffer commandBuffer) noexcept
{
	if (_updateModelMatrix)
		_UpdateModelMatrix();

	for (pair<string, ObjectComponent*> kvp : _components)
		kvp.second->UpdateData(commandBuffer);
}

Object::~Object() noexcept
{
	Unload();
}
