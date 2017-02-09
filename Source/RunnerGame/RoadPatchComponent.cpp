/* NekoEngine
*
* RoadPatchComponent.cpp
* Author: Cristian Lambru
*
* RoadPatchComponent class definition
*
* -----------------------------------------------------------------------------
*
* Copyright (c) 2015-2017, NekoEngine
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

#include <Scene/Object.h>
#include <Platform/Platform.h>

#include "Player.h"
#include "PatchManager.h"
#include "EnemyFactory.h"
#include "RoadPatchComponent.h"
#include "EnemyManager.h"

using namespace glm;

REGISTER_COMPONENT_CLASS(RoadPatchComponent);

static uint64_t rpid = 0;

RoadPatchComponent::RoadPatchComponent(ComponentInitializer *initializer) :
	ObjectComponent(initializer),
	_hit(false)
{
}

int RoadPatchComponent::Load()
{
	int ret{ ObjectComponent::Load() };
	if (ret != ENGINE_OK) return ret;

	return ENGINE_OK;
}

void RoadPatchComponent::Update(double deltaTime) noexcept
{
	ObjectComponent::Update(deltaTime);
}

bool RoadPatchComponent::Unload()
{
	if (!ObjectComponent::Unload())
		return false;

	return true;
}

void RoadPatchComponent::OnHit(Object *other, glm::vec3 &position)
{
	Player *p{ dynamic_cast<Player *>(other) };
	if (_hit || !p) return;
	_hit = true;

	PatchManager::NewPatch();	

	if (Platform::Rand() % 2 == 0) // 50% chance of an enemy
		return;

	vec3 enemyPosition = _parent->GetPosition();
	vec3 enemyRotation = _parent->GetRotation();

	if (Platform::Rand() % 2 == 1) // 1 for bat 
		EnemyManager::NewBatEnemy(enemyPosition, enemyRotation);
	else
		EnemyManager::NewTarantulaEnemy(enemyPosition, enemyRotation);
}

RoadPatchComponent::~RoadPatchComponent()
{
	//
}