/* NekoEngine
 *
 * StaticMeshComponent.h
 * Author: Alexandru Naiman
 *
 * StaticMeshComponent class definition 
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

#include <Engine/Engine.h>
#include <Renderer/Renderer.h>
#include <Renderer/Material.h>
#include <Renderer/Drawable.h>
#include <Renderer/StaticMesh.h>
#include <Scene/ObjectComponent.h>

#define SM_GENERATED	"generated"

class StaticMeshComponent : public ObjectComponent
{
public:
	ENGINE_API StaticMeshComponent(ComponentInitializer *initializer);

	ENGINE_API NString &GetMeshID() noexcept { return _meshId; }
	ENGINE_API StaticMesh *GetMesh() noexcept { return _mesh; }
	ENGINE_API virtual size_t GetVertexCount() const noexcept override { return _mesh->GetVertexCount(); }
	ENGINE_API virtual size_t GetTriangleCount() const noexcept override { return _mesh->GetTriangleCount(); }
	ENGINE_API virtual NArray<Drawable> *GetDrawables() noexcept override { return &_drawables; }
	ENGINE_API virtual const glm::vec4 &GetColor() const noexcept { return _color; }

	ENGINE_API virtual void SetPosition(glm::vec3 &position) noexcept override;
	ENGINE_API virtual void SetRotation(glm::vec3 &rotation) noexcept override;
	ENGINE_API virtual void SetScale(glm::vec3 &scale) noexcept override;

	ENGINE_API virtual int Load() override;
	ENGINE_API int LoadStatic(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices, bool createGroup = false) { return _mesh->LoadStatic(vertices, indices, createGroup); }
	ENGINE_API int LoadDynamic(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices, bool createGroup = false) { return _mesh->LoadDynamic(vertices, indices, createGroup); }
	ENGINE_API virtual bool Upload(Buffer *buffer = nullptr) override;
	ENGINE_API virtual int CreateBuffer(bool dynamic) { return _mesh->CreateBuffer(dynamic); }
	ENGINE_API virtual void Update(double deltaTime) noexcept override;
	ENGINE_API virtual void UpdatePosition() noexcept override;

	ENGINE_API virtual bool InitDrawables() override;
	ENGINE_API virtual bool RebuildCommandBuffers() override;

	ENGINE_API virtual void AddGroup(MeshGroup &group, Material *mat) { _mesh->AddGroup(group); _materials.Add(mat); }
	ENGINE_API virtual void ResetGroups() { _mesh->ResetGroups(); }

	ENGINE_API virtual bool Unload() override;
	
	ENGINE_API virtual ~StaticMeshComponent() { };

	virtual VkDeviceSize GetRequiredMemorySize() const noexcept override { return _mesh->GetRequiredMemorySize(); }
	
	virtual void UpdateData(VkCommandBuffer commandBuffer) noexcept override;
	ENGINE_API virtual void DrawShadow(VkCommandBuffer commandBuffer, uint32_t shadowId) const noexcept override;

	void SetUniformBuffer(Buffer *buffer) { _ubo = buffer; }

protected:
	NString _meshId;
	StaticMesh *_mesh;
	bool _blend, _updateModelMatrix;
	std::vector<int> _materialIds;
	NArray<Material *> _materials;
	NArray<Drawable> _drawables;
	ObjectData _objectData;
	glm::mat4 _translationMatrix, _scaleMatrix;
	glm::quat _rotationQuaternion;
	glm::vec4 _color;	

	NArray<VkCommandBuffer> _sceneDrawBuffers, _depthDrawBuffers;
	VkDescriptorSet _descriptorSet;
	VkDescriptorPool _descriptorPool;

	Buffer *_ubo;

	void _SortGroups();
	void _UpdateModelMatrix();
};
