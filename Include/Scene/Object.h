/* NekoEngine
 *
 * Object.h
 * Author: Alexandru Naiman
 *
 * Object class definition 
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

#pragma once

#include <vector>

#include <Engine/Engine.h>
#include <Runtime/NBounds.h>
#include <Renderer/Renderer.h>
#include <Renderer/Drawable.h>
#include <Renderer/StaticMesh.h>
#include <Renderer/Material.h>
#include <Scene/ObjectComponent.h>

#define OBJ_NO_MATERIAL	-1

enum class ForwardDirection : unsigned short
{
	PositiveZ = 0,
	NegativeZ = 1,
	PositiveX = 2,
	NegativeX = 3
};

typedef std::multimap<std::string, std::string> ArgumentMapType;
typedef std::pair<ArgumentMapType::iterator, ArgumentMapType::iterator> ArgumentMapRangeType;
	
class ObjectInitializer
{
public:
	ENGINE_API ObjectInitializer();

	uint32_t id;
	std::string name;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	ArgumentMapType arguments;
};

class Object
{
public:
	ENGINE_API Object() noexcept;
	ENGINE_API Object(ObjectInitializer *initializer) noexcept;

	ENGINE_API uint32_t GetId() const noexcept { return _id; }
	ENGINE_API NString &GetName() noexcept { return _name; }
	ENGINE_API bool GetNoCull() const { return _noCull; }
	ENGINE_API bool GetUpdateWhilePaused() const noexcept { return _updateWhilePaused; }
	ENGINE_API NArray<Drawable> *GetDrawables() const noexcept
	{
		for (std::pair<std::string, ObjectComponent *> kvp : _components)
			if (kvp.second->GetDrawables()) return kvp.second->GetDrawables();
		return nullptr;
	}

	ENGINE_API void SetId(int id) noexcept { _id = id; }
	ENGINE_API void SetPosition(glm::vec3& position) noexcept;
	ENGINE_API void SetRotation(glm::vec3& rotation) noexcept;
	ENGINE_API void SetScale(glm::vec3& scale) noexcept;
	ENGINE_API void SetForwardDirection(ForwardDirection dir) noexcept;
	ENGINE_API void SetNoCull(bool noCull) { _noCull = noCull; }
	ENGINE_API void SetUpdateWhilePaused(bool update) { _updateWhilePaused = update; }
	ENGINE_API void SetBounds(const NBounds &bounds) noexcept;

	ENGINE_API void LookAt(const glm::vec3 &point) noexcept;
	ENGINE_API void MoveForward(float distance) noexcept;
	ENGINE_API void MoveRight(float distance) noexcept;

	ENGINE_API size_t GetVertexCount() noexcept;
	ENGINE_API size_t GetTriangleCount() noexcept;
	ENGINE_API glm::vec3 &GetPosition() noexcept { return _position; }
	ENGINE_API glm::quat &GetRotation() noexcept { return _rotationQuaternion; }
	ENGINE_API glm::vec3 &GetRotationAngles() noexcept { return _rotation; }
	ENGINE_API glm::vec3 &GetScale() noexcept { return _scale; }
	ENGINE_API glm::mat4 &GetModelMatrix() noexcept { return _modelMatrix; }
	ENGINE_API const NBounds &GetBounds() const noexcept { return _bounds; }
	ENGINE_API const NBounds &GetTransformedBounds() const noexcept { return _transformedBounds; }

	ENGINE_API bool IsEnabled() const noexcept { return _enabled; }
	ENGINE_API void SetEnabled(bool enable) noexcept { _enabled = enable; for (std::pair<std::string, ObjectComponent *> kvp : _components) kvp.second->Enable(enable); }
	ENGINE_API bool IsVisible() const noexcept { return _visible; }
	ENGINE_API void SetVisible(bool visible) noexcept { _visible = visible; for (std::pair<std::string, ObjectComponent *> kvp : _components) kvp.second->SetVisible(visible); }

	ENGINE_API virtual int Load();
	ENGINE_API virtual int CreateBuffers();
	ENGINE_API virtual void FixedUpdate() noexcept;
	ENGINE_API virtual void Update(double deltaTime) noexcept;
	ENGINE_API virtual bool Unload() noexcept;
	ENGINE_API virtual bool CanUnload() noexcept;

	ENGINE_API virtual void OnHit(Object *other, glm::vec3 &position);

	ENGINE_API virtual bool BuildCommandBuffers();
	ENGINE_API virtual bool RebuildCommandBuffers();

	ENGINE_API void AddComponent(const char *name, ObjectComponent *comp);
	ENGINE_API ObjectComponent *GetComponent(const char *name)
	{
		if (_components.find(name) == _components.end())
			return nullptr;

		return _components[name];
	}

	template<typename T>
	std::vector<T *> GetComponentsOfType()
	{
		std::vector<T *> vec;

		for (std::pair<std::string, ObjectComponent *> kvp : _components)
		{
			T *c = dynamic_cast<T *>(kvp.second);
			if (c)
				vec.push_back(c);
		}

		return vec;
	}
	ENGINE_API bool RemoveComponent(const char *name, bool force = false);

	ENGINE_API void AddToScene();
	ENGINE_API void Destroy();

	ENGINE_API virtual ~Object() noexcept;

	VkDeviceSize GetRequiredMemorySize();

	ENGINE_API virtual void DrawShadow(VkCommandBuffer commandBuffer, uint32_t shadowId) const noexcept;
	ENGINE_API virtual void UpdateData(VkCommandBuffer commandBuffer) noexcept;

protected:
	int32_t _id;
	NString _name;
	glm::vec3 _position, _rotation, _scale;
	glm::vec3 _center, _forward, _right;
	glm::quat _rotationQuaternion;
	ForwardDirection _objectForward;
	bool _loaded, _visible;
	std::map<std::string, ObjectComponent*> _components;
	glm::mat4 _translationMatrix, _scaleMatrix, _modelMatrix;
	bool _updateWhilePaused, _noCull, _updateModelMatrix, _haveMesh, _enabled;
	Buffer *_buffer;
	NBounds _bounds, _transformedBounds;

	void _UpdateModelMatrix()
	{
		if (_haveMesh)
			_modelMatrix = (_translationMatrix * mat4_cast(_rotationQuaternion)) * _scaleMatrix;

		for (std::pair<std::string, ObjectComponent *> kvp : _components)
			kvp.second->UpdatePosition();

		_updateModelMatrix = false;

		if (!_bounds.IsValid())
			return;

		_bounds.Transform(_modelMatrix, &_transformedBounds);
	}

	inline void _UpdateTransformedBounds() noexcept
	{
		if (!_bounds.IsValid()) return;
		_bounds.Transform(_modelMatrix, &_transformedBounds);
	}
};

#if defined(_MSC_VER)
template ENGINE_API class NArray<Object>;
#endif
