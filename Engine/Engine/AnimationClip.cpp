/* Neko Engine
 *
 * AnimationClip.cpp
 * Author: Alexandru Naiman
 *
 * AnimationClip class implementation 
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

#define ENGINE_INTERNAL

#include <glm/glm.hpp>

#include <Engine/AnimationClip.h>

#define AC_MODULE	"AnimationClip"

using namespace std;
using namespace glm;

AnimationClip::AnimationClip(AnimationClipResource *res) noexcept :
	_duration(0.f)
{
}

int AnimationClip::Load()
{
	string path("/");
	path.append(GetResourceInfo()->filePath);

/*	if (AssetLoader::LoadNFG(path, _vertices, _indices, _groupOffset, _groupCount) != ENGINE_OK)
	{
		Logger::Log(MESH_MODULE, LOG_CRITICAL, "Failed to load mesh id=%s", _resourceInfo->name.c_str());
		return ENGINE_FAIL;
	}

	_indexCount = _indices.size();
	_vertexCount = _vertices.size();
	_triangleCount = _indexCount / 3;

	_CalculateTangents();*/

	Logger::Log(AC_MODULE, LOG_DEBUG, "Loaded animation clip id %s from %s", _resourceInfo->name.c_str(), path.c_str());

	return ENGINE_OK;
}

void AnimationClip::Release() noexcept
{
}

AnimationClip::~AnimationClip() noexcept
{
	Release();
}
