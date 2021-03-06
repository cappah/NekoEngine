/* NekoEngine
 *
 * ObjectComponent.h
 * Author: Alexandru Naiman
 *
 * ObjectComponent class definition 
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

#include <map>

#include <Engine/Engine.h>
#include <Renderer/Drawable.h>

typedef std::multimap<std::string, std::string> ArgumentMapType;
typedef std::pair<ArgumentMapType::iterator, ArgumentMapType::iterator> ArgumentMapRangeType;

class ComponentInitializer
{
public:
	ComponentInitializer() :
		parent(nullptr),
		position(glm::vec3(0.f)),
		rotation(glm::vec3(0.f)),
		scale(glm::vec3(1.f))
	{ }

	Object *parent;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	ArgumentMapType arguments;
};

class ENGINE_API ObjectComponent
{
public:
	ObjectComponent(ComponentInitializer *initializer);

	virtual class Object *GetParent() noexcept { return _parent; }
	virtual size_t GetVertexCount() const noexcept { return 0; }
	virtual size_t GetTriangleCount() const noexcept { return 0; }
	virtual NArray<Drawable> *GetDrawables() noexcept { return nullptr; }
	
	virtual void SetParent(class Object *obj) { _parent = obj; }
	virtual void SetPosition(glm::vec3 &position) noexcept;
	virtual void SetRotation(glm::vec3 &rotation) noexcept;
	virtual void SetScale(glm::vec3 &scale) noexcept;

	virtual bool IsVisible() const noexcept { return _visible; }
	virtual void SetVisible(bool visible) noexcept { _visible = visible; }

	virtual void Enable(bool enable) { _enabled = enable; }
	virtual bool IsEnabled() const { return _enabled; }

	virtual void AttachToCamera(bool attach) { _attachedToCamera = attach; }
	virtual bool IsAttachedToCamera() const { return _attachedToCamera; }

	virtual int Load() { _loaded = true; return ENGINE_OK; }
	virtual int CreateBuffers() { return ENGINE_OK; }
	virtual int InitializeComponent();
	virtual bool Upload(Buffer *buffer) { (void)buffer; return true; }
	virtual void Update(double deltaTime) noexcept { (void)deltaTime; }
	virtual void UpdatePosition() noexcept { }
	
	virtual void OnHit(Object *other, glm::vec3 &position) { (void)other; (void)position; }

	virtual bool InitDrawables() { return true; }
	virtual bool RebuildCommandBuffers() { return true; }

	virtual bool Unload();
	virtual bool CanUnload() { return true; }

	virtual ~ObjectComponent() { Unload(); }

	virtual VkDeviceSize GetRequiredMemorySize() const noexcept { return 0; }
	virtual void UpdateData(VkCommandBuffer commandBuffer) noexcept { (void)commandBuffer; }
	virtual void DrawShadow(VkCommandBuffer commandBuffer, uint32_t shadowId) const noexcept { (void)commandBuffer; (void)shadowId; }

protected:
	class Object *_parent;
	bool _loaded, _enabled, _visible, _attachedToCamera;
	glm::vec3 _position, _rotation, _scale, _worldPosition, _worldRotation, _worldScale;

	VkCommandBuffer _cmdBuffer;
};

#if defined(_MSC_VER)
template class ENGINE_API NArray<ObjectComponent*>;
#endif
