/* NekoEngine
 *
 * StaticMeshComponent.h
 * Author: Alexandru Naiman
 *
 * StaticMeshComponent class definition 
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

#include <Engine/Engine.h>
#include <Engine/Material.h>
#include <Engine/StaticMesh.h>
#include <Scene/ObjectComponent.h>

typedef struct MATRIX_BLOCK
{
	glm::mat4 ModelViewProjection;
	glm::mat4 Model;
	glm::mat4 View;
	glm::mat4 Normal;
} MatrixBlock;

class StaticMeshComponent : public ObjectComponent
{
public:
	ENGINE_API StaticMeshComponent(ComponentInitializer *initializer);

	ENGINE_API StaticMesh *GetMesh() noexcept { return _mesh; }
	ENGINE_API virtual size_t GetVertexCount() noexcept override { return _mesh->GetVertexCount(); }
	ENGINE_API virtual size_t GetTriangleCount() noexcept override { return _mesh->GetTriangleCount(); }

	ENGINE_API virtual void SetLocalPosition(glm::vec3& position) noexcept override;
	ENGINE_API virtual void SetLocalRotation(glm::vec3& rotation) noexcept override;
	ENGINE_API virtual void SetLocalScale(glm::vec3& scale) noexcept override;

	ENGINE_API virtual void UpdatePosition() noexcept override { ObjectComponent::UpdatePosition(); _mmNeedsUpdate = true; }

	ENGINE_API virtual int Load() override;
	ENGINE_API virtual int CreateArrayBuffer() override;
	
	ENGINE_API virtual void Draw(RShader *shader, class Camera *camera) noexcept override;
	ENGINE_API virtual void Update(double deltaTime) noexcept override;

	ENGINE_API virtual bool Unload() override;
	
	ENGINE_API virtual ~StaticMeshComponent() { };
	
protected:
	std::string _meshId;
	StaticMesh *_mesh;
	bool _blend;
	Renderer* _renderer;
	glm::mat4 _translationMatrix, _scaleMatrix, _rotationMatrix;
	std::vector<int> _materialIds;
	std::vector<Material*> _materials;
	MatrixBlock _matrixBlock;
	RBuffer *_matrixUbo;
	bool _mmNeedsUpdate;
};
