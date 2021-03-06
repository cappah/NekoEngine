/* NekoEngine
 *
 * SkeletalMeshComponent.h
 * Author: Alexandru Naiman
 *
 * SkeletalMeshComponent class definition 
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
#include <Renderer/SkeletalMesh.h>
#include <Scene/Components/StaticMeshComponent.h>

class SkeletalMeshComponent : public StaticMeshComponent
{
public:
	ENGINE_API SkeletalMeshComponent(ComponentInitializer *initializer);
	
	ENGINE_API SkeletalMesh *GetMesh() noexcept { return _mesh; }

	ENGINE_API virtual int Load() override;
	ENGINE_API int LoadStatic(std::vector<SkeletalVertex> &vertices, std::vector<uint32_t> &indices, bool createGroup = false) { return _mesh->LoadStatic(vertices, indices, createGroup); }
	ENGINE_API int LoadDynamic(std::vector<SkeletalVertex> &vertices, std::vector<uint32_t> &indices, bool createGroup = false) { return _mesh->LoadDynamic(vertices, indices, createGroup); }
	ENGINE_API virtual bool Upload(Buffer *buffer = nullptr) override;
	ENGINE_API virtual int InitializeComponent() override;

	ENGINE_API virtual void Update(double deltaTime) noexcept override;

	ENGINE_API virtual bool InitDrawables() override;
	ENGINE_API virtual bool RebuildCommandBuffers() override;

	ENGINE_API virtual bool Unload() override;

	ENGINE_API virtual ~SkeletalMeshComponent() { };

	virtual VkDeviceSize GetRequiredMemorySize() const noexcept override { return _mesh->GetRequiredMemorySize(); }
	
	virtual void UpdateData(VkCommandBuffer commandBuffer) noexcept override;
	ENGINE_API virtual void DrawShadow(VkCommandBuffer commandBuffer, uint32_t shadowId) const noexcept override;

protected:
	SkeletalMesh *_mesh;
	std::string _animatorId;
	class AnimatorComponent *_animator;
};
