/* NekoEngine
 *
 * ObjectComponent.h
 * Author: Alexandru Naiman
 *
 * ObjectComponent class definition 
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

#include <map>
#include <glm/glm.hpp>

#include <Engine/Engine.h>

typedef std::multimap<std::string, std::string> ArgumentMapType;
typedef std::pair<ArgumentMapType::iterator, ArgumentMapType::iterator> ArgumentMapRangeType;

class ComponentInitializer
{
public:
	Object *parent;
	ArgumentMapType arguments;
};

class ENGINE_API ObjectComponent
{
public:
	ObjectComponent(ComponentInitializer *initializer);

	virtual class Object *GetParent() noexcept { return _parent; }
	virtual glm::vec3 &GetLocalPosition() noexcept { return _localPosition; }
	virtual glm::vec3 &GetLocalRotation() noexcept { return _localRotation; }
	virtual glm::vec3 &GetLocalScale() noexcept { return _localScale; }
	virtual glm::vec3 &GetPosition() noexcept { return _position; }
	virtual glm::vec3 &GetRotation() noexcept { return _rotation; }
	virtual glm::vec3 &GetScale() noexcept { return _scale; }
	virtual size_t GetVertexCount() noexcept { return 0; }
	virtual size_t GetTriangleCount() noexcept { return 0; }
	
	virtual void SetParent(class Object *obj) { _parent = obj; }
	virtual void SetLocalPosition(glm::vec3& position) noexcept;
	virtual void SetLocalRotation(glm::vec3& rotation) noexcept;
	virtual void SetLocalScale(glm::vec3& scale) noexcept;

	virtual void UpdatePosition() noexcept;

	virtual int Load() { _loaded = true; return ENGINE_OK; }
	virtual int CreateArrayBuffer() { return ENGINE_OK; }
	virtual int InitializeComponent() { return ENGINE_OK; }
	
	virtual void Draw(RShader *shader, class Camera *camera) noexcept { (void)shader; (void)camera; }
	virtual void Update(double deltaTime) noexcept { (void)deltaTime; }
	
	virtual bool Unload();
	virtual bool CanUnload() { return true; }

	virtual void OnHit(Object *other, glm::vec3 &position) { (void)other; (void)position; }

	virtual ~ObjectComponent() { Unload(); }

protected:
	class Object* _parent;
	glm::vec3 _localPosition, _localRotation, _localScale;
	glm::vec3 _position, _rotation, _scale;
	bool _loaded;
};

#if defined(_MSC_VER)
template class ENGINE_API NArray<ObjectComponent*>;
#endif
