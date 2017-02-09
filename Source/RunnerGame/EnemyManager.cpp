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

#include "EnemyManager.h"
#include <Scene/Object.h>

using namespace std;
using namespace glm;

static queue<vector<Object *>> _layers{};
static const size_t _maxLayerCount{ 5 };
static vec3 _nextPatchPos{ 0.f, 0.f, 1000.f };
static queue<Object *> _patches;

size_t EnemyManager::_maxEnemyLayers (5);

void EnemyManager::CreateNewLayer()
{
	_layers.push(vector<Object *>());
}

void EnemyManager::AddEnemyInCurrentLayer(Object *patch)
{
	_layers.back().push_back(patch);
}

void EnemyManager::DestroyOldestLayer()
{
	for (auto item : _layers.front()) {
		item->Destroy ();
	}

	_layers.pop();
}

void EnemyManager::NewBatEnemy(vec3 position, vec3 rotation)
{
	Object* batEnemy = GetNewBatEnemy (position, rotation);

	CreateNewLayer ();
	AddEnemyInCurrentLayer (batEnemy);

	if (_layers.size () > _maxEnemyLayers)
		DestroyOldestLayer ();
}

void EnemyManager::NewTarantulaEnemy(vec3 position, vec3 rotation)
{
	Object* batEnemy = GetNewTarantulaEnemy(position, rotation);

	CreateNewLayer();
	AddEnemyInCurrentLayer(batEnemy);

	if (_layers.size() > _maxEnemyLayers)
		DestroyOldestLayer();
}

Object *EnemyManager::GetNewBatEnemy(vec3 &position, vec3 &rotation)
{
	ObjectInitializer initializer{};
	initializer.position = position;
	initializer.rotation = rotation;
	initializer.scale = vec3(100.f);

	Object *enemy = Engine::NewObject("BatEnemy", &initializer);
	if (!enemy) { DIE("Out of resources"); }
	if (enemy->Load() != ENGINE_OK) { DIE("Patch load failed"); }
	enemy->AddToScene();

	return enemy;
}

Object *EnemyManager::GetNewTarantulaEnemy(vec3 &position, vec3 &rotation)
{
	ObjectInitializer initializer{};
	initializer.position = position;
	initializer.rotation = rotation;

	Object *patch{ Engine::NewObject("TarantulaEnemy", &initializer) };
	if (!patch) { DIE("Out of resources"); }
	patch->AddToScene();
	return patch;
}