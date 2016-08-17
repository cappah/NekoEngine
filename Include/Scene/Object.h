/* NekoEngine
 *
 * Object.h
 * Author: Alexandru Naiman
 *
 * Object class definition 
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

#pragma once

#include <vector>

#include <glm/glm.hpp>

#include <Engine/Engine.h>
#include <Engine/StaticMesh.h>
#include <Engine/SkeletalMesh.h>
#include <Engine/Shader.h>
#include <Engine/Texture.h>
#include <Engine/Material.h>
#include <Scene/ObjectComponent.h>

#define OBJ_NO_MATERIAL	-1

enum class ForwardDirection : unsigned short
{
	PositiveZ = 0,
	NegativeZ = 1,
	PositiveX = 2,
	NegativeX = 3
};

typedef struct OBJECT_MATRIX_BLOCK
{
	glm::mat4 ModelViewProjection;
	glm::mat4 Model;
	glm::mat4 View;
} ObjectMatrixBlock;

typedef struct OBJECT_BLOCK
{
	glm::vec3 CameraPosition;
	float padding;
	glm::vec3 ObjectColor;
	float padding1;
} ObjectBlock;

typedef std::multimap<std::string, std::string> ArgumentMapType;
typedef std::pair<ArgumentMapType::iterator, ArgumentMapType::iterator> ArgumentMapRangeType;
	
class ObjectInitializer
{
public:
	ObjectInitializer() :
		id(8000),
		name("unnamed"),
		position(0.f),
		rotation(0.f),
		scale(1.f),
		color(0.f)
	{ }

	int id;
	std::string name;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::vec3 color;
	ArgumentMapType arguments;
};

class Object
{
public:
	ENGINE_API Object() noexcept;
	ENGINE_API Object(ObjectInitializer *initializer) noexcept;
	 
	ENGINE_API int GetId() noexcept { return _id; }
	ENGINE_API bool GetUpdateWhilePaused() noexcept { return _updateWhilePaused; }

	ENGINE_API void SetId(int id) noexcept { _id = id; }
	ENGINE_API void SetPosition(glm::vec3& position) noexcept;
	ENGINE_API void SetRotation(glm::vec3& rotation) noexcept;
	ENGINE_API void SetScale(glm::vec3& scale) noexcept;
	ENGINE_API void SetColor(glm::vec3& color) noexcept { _objectBlock.ObjectColor = color; }
	ENGINE_API void SetForwardDirection(ForwardDirection dir) noexcept;
	ENGINE_API void SetUpdateWhilePaused(bool update) { _updateWhilePaused = update; }

	ENGINE_API void LookAt(glm::vec3& point) noexcept;
	ENGINE_API void MoveForward(float distance) noexcept;
	ENGINE_API void MoveRight(float distance) noexcept;

	ENGINE_API size_t GetVertexCount() noexcept;
	ENGINE_API size_t GetTriangleCount() noexcept;
	ENGINE_API glm::vec3& GetPosition() noexcept { return _position; }
	ENGINE_API glm::vec3& GetRotation() noexcept { return _rotation; }
	ENGINE_API glm::vec3& GetScale() noexcept { return _scale; }
	ENGINE_API glm::vec3& GetColor() noexcept { return _objectBlock.ObjectColor; }
	ENGINE_API glm::mat4& GetModelMatrix() noexcept { return _modelMatrix; }

	ENGINE_API void BindUniformBuffer(RShader *shader) { shader->FSSetUniformBuffer(0, 0, sizeof(ObjectBlock), _objectUbo); }
    
	ENGINE_API virtual int Load();
	ENGINE_API virtual void Draw(RShader* shader) noexcept;
	ENGINE_API virtual void Update(double deltaTime) noexcept;
	ENGINE_API virtual bool Unload() noexcept;
	ENGINE_API virtual bool CanUnload() noexcept;

	ENGINE_API void AddComponent(const char *name, ObjectComponent *comp);
	ENGINE_API ObjectComponent *GetComponent(const char *name) { return _components[name]; }
	ENGINE_API bool RemoveComponent(const char *name, bool force = false);

	ENGINE_API void AddToScene();
	ENGINE_API void Destroy();

	ENGINE_API virtual ~Object() noexcept;

protected:
	int _id;
	Renderer* _renderer;
	glm::vec3 _position, _rotation, _scale;
	glm::vec3 _center, _forward, _right;
	ForwardDirection _objectForward;
	bool _loaded;	
	std::map<std::string, ObjectComponent*> _components;
	RBuffer *_objectUbo;
	ObjectBlock _objectBlock;
	glm::mat4 _translationMatrix, _scaleMatrix, _rotationMatrix, _modelMatrix;
	bool _updateWhilePaused;

	void _UpdateModelMatrix() noexcept { _modelMatrix = (_translationMatrix * _rotationMatrix) * _scaleMatrix; for(std::pair<std::string, ObjectComponent *> kvp : _components) kvp.second->UpdatePosition(); }
};

#if defined(_MSC_VER)
template ENGINE_API class NArray<Object>;
#endif