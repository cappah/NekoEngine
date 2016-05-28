/* Neko Engine
 *
 * Object.h
 * Author: Alexandru Naiman
 *
 * Object class definition 
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

#define OBJ_NO_MATERIAL	-1

class Object
{
public:
	ENGINE_API Object() noexcept;
	 
	ENGINE_API int GetId() noexcept { return _id; }

	ENGINE_API void SetId(int id) noexcept { _id = id; }
	ENGINE_API void SetPosition(glm::vec3& position) noexcept;
	ENGINE_API void SetRotation(glm::vec3& rotation) noexcept;
	ENGINE_API void SetScale(glm::vec3& scale) noexcept;
	ENGINE_API void SetColor(glm::vec3& color) noexcept { _objectBlock.ObjectColor = color; }
	ENGINE_API void SetForwardDirection(ForwardDirection dir) noexcept;

	ENGINE_API void LookAt(glm::vec3& point) noexcept;
	ENGINE_API void MoveForward(float distance) noexcept;
	ENGINE_API void MoveRight(float distance) noexcept;

	ENGINE_API size_t GetVertexCount() noexcept { return 0; }
	ENGINE_API size_t GetTriangleCount() noexcept { return 0; }
	ENGINE_API glm::vec3& GetPosition() noexcept { return _position; }
	ENGINE_API glm::mat4& GetModelMatrix() noexcept { return _modelMatrix; }

    ENGINE_API void BindUniformBuffer(RShader *shader) { shader->FSSetUniformBuffer(0, 0, sizeof(ObjectBlock), _objectUbo); }
    
	ENGINE_API virtual int Load();
	ENGINE_API virtual void Draw(RShader* shader) noexcept;
	ENGINE_API virtual void Update(double deltaTime) noexcept;
	ENGINE_API void Unload() noexcept;

	ENGINE_API void AddComponent(const char *name, ObjectComponent *comp);
	ENGINE_API ObjectComponent *GetComponent(const char *name) { return _components[name]; }

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

	void _UpdateModelMatrix() noexcept { _modelMatrix = (_translationMatrix * _rotationMatrix) * _scaleMatrix; }
};

