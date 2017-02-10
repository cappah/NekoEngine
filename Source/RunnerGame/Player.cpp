/* DungeonGame
 *
 * Player.cpp
 * Author: Cristian Lambru
 *
 * Player class
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

#include <Engine/Engine.h>
#include <Engine/EventManager.h>

#include "Player.h"
#include "BatEnemy.h"
#include "TarantulaEnemy.h"
#include "RunningPlayerState.h"
#include "DyingPlayerState.h"

using namespace glm;
using namespace std;

REGISTER_OBJECT_CLASS(Player);

Player::Player(ObjectInitializer *initializer) :
	Object(initializer),
	_currentPlayerState(nullptr)
{

}

int Player::Load()
{
	int ret{ Object::Load() };
	if (ret != ENGINE_OK) return ret;

	_currentPlayerState = new RunningPlayerState(this);
	_moveDirection = vec3(0, 0, 1);

	return ENGINE_OK;
}

void Player::OnHit(Object *other, glm::vec3 &position)
{
	if ((_currentPlayerState->GetStateType() == PlayerStateType::STATE_DYING) ||
		(_currentPlayerState->GetStateType() == PlayerStateType::STATE_DEAD))
		return;
	
	_currentPlayerState->OnHit(other);
	
	if(dynamic_cast<BatEnemy *>(other))
	{
		NSLog(@"bat");
		if (_currentPlayerState->GetStateType() != PlayerStateType::STATE_CROUCHING)
			Kill();
		other->Destroy();
	}
	
	if(dynamic_cast<TarantulaEnemy *>(other))
	{
		NSLog(@"tarantula");
		if (_currentPlayerState->GetStateType() != PlayerStateType::STATE_JUMPING)
			Kill();
		other->Destroy();
	}
}

void Player::Update(double deltaTime) noexcept
{
	Object::Update(deltaTime);
	_currentPlayerState->Update(deltaTime);
}

bool Player::Unload() noexcept
{
	if (!Object::Unload())
		return false;

	return true;
}

void Player::Kill() noexcept
{
	SetState(new DyingPlayerState(this));
}

vec3 Player::GetForwardDirection() const noexcept
{
	return _moveDirection;
}

void Player::SetState(PlayerState* playerState)
{
	if (_currentPlayerState)
		delete _currentPlayerState;

	_currentPlayerState = playerState;
}

PlayerState* Player::GetState() const
{
	return _currentPlayerState;
}

Player::~Player()
{
	//
}
