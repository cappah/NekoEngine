/* RunnerGame
 *
 * PatchManager.cpp
 * Author: Alexandru Naiman
 *
 * PatchManager class
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

#include "PatchManager.h"
#include <Scene/Object.h>

using namespace std;
using namespace glm;

static queue<vector<Object *>> _layers{};
static const size_t _maxLayerCount{ 5 };
static vec3 _nextPatchPos{ 0.f, 0.f, 1000.f };
static queue<Object *> _patches;

void PatchManager::CreateNewLayer()
{
	_layers.push(vector<Object *>());
}

void PatchManager::AddPatchInCurrentLayer(Object *patch)
{
	_layers.back().push_back(patch);

	//TODO: Instantiate
}

void PatchManager::DestroyOldestLayer()
{
	for (auto item : _layers.front())
	{
		//TODO: Destroy patch in oldest layer
	}

	_layers.pop();
}

void PatchManager::NewPatch()
{
	vec3 r{ 0.f, 90.f, 0.f };

	_patches.push(NewRoadPatch(_nextPatchPos, r));
	_nextPatchPos.z += 200.f;

	if (_patches.size() > 7)
	{
		_patches.front()->Destroy();
		_patches.pop();
	}

	//AddPatchInCurrentLayer(NewRoadPatch(_nextPatchPos, r));

/*	if (Platform::Rand() % 5 < 4)
		patch = PatchFactory::GetRoadPatch(vec3(0), quat());
	else
		patch = PatchFactory::GetSplitPatch(vec3(0), quat());*/
}

Object *PatchManager::NewRoadPatch(vec3 &position, vec3 &rotation)
{
	ObjectInitializer initializer{};
	initializer.position = position;
	initializer.rotation = rotation;
	initializer.scale = vec3(100.f);

	Object *patch{ Engine::NewObject("RoadPatch", &initializer) };
	if (!patch) { DIE("Out of resources"); }
	if (patch->Load() != ENGINE_OK) { DIE("Patch load failed"); }
	patch->AddToScene();

	return patch;
}

Object *PatchManager::NewSplitPatch(vec3 &position, vec3 &rotation)
{
	ObjectInitializer initializer{};
	initializer.position = position;
	initializer.rotation = rotation;

	Object *patch{ Engine::NewObject("SplitPatch", &initializer) };
	if (!patch) { DIE("Out of resources"); }
	patch->AddToScene();
	return patch;
}