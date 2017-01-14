/* NekoEngine
 *
 * SkeletalMesh.h
 * Author: Alexandru Naiman
 *
 * SkeletalMesh class definition
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

#include <string>
#include <vector>
#include <unordered_map>

#include <Engine/Engine.h>
#include <Animation/Skeleton.h>
#include <Renderer/StaticMesh.h>

class SkeletalMesh : public StaticMesh
{
public:
	ENGINE_API SkeletalMesh(MeshResource *res) noexcept;
	
	ENGINE_API const std::vector<SkeletalVertex> &GetVertices() const noexcept { return _vertices; }

	ENGINE_API Skeleton *CreateSkeleton();
	
	ENGINE_API virtual int Load() override;
	ENGINE_API int LoadStatic(std::vector<SkeletalVertex> &vertices, std::vector<uint32_t> &indices, bool createGroup = true, bool calculateTangents = true, bool createBounds = true);
	ENGINE_API int LoadDynamic(std::vector<SkeletalVertex> & vertices, std::vector<uint32_t> &indices, bool createGroup = true, bool calculateTangents = true, bool createBounds = true);
	ENGINE_API virtual void CreateBounds() override;

	ENGINE_API virtual ~SkeletalMesh() noexcept;

	VkDeviceSize GetRequiredMemorySize();

	bool Upload(Buffer *buffer = nullptr);
	bool BuildDrawables(NArray<Material *> &materials, VkDescriptorSet descriptorSet, NArray<Drawable> &drawables, bool buildDepth = true, bool buildBounds = true);
	VkDescriptorSet CreateDescriptorSet(VkDescriptorPool pool, Buffer *uniform, Buffer *boneBuffer);
	virtual void DrawShadow(VkCommandBuffer commandBuffer, uint32_t shadowId, VkDescriptorSet descriptorSet) noexcept override;

private:
	std::vector<SkeletalVertex> _vertices;
	glm::dmat4 _globalInverseTransform;
	std::vector<Bone> _bones;
	std::vector<TransformNode> _nodes;

	void _CalculateTangents();
	void _BuildBounds(uint32_t group, NBounds &bounds);
};

#if defined(_MSC_VER)
template class ENGINE_API NArray<SkeletalMesh *>;
#endif