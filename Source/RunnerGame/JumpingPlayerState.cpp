/* RunnerGame
*
* JumpingPlayerState.cpp
* Author: Cristian Lambru
*
* JumpingPlayerState class
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

#include "JumpingPlayerState.h"
#include "RunningPlayerState.h"
#include "Player.h"

JumpingPlayerState::JumpingPlayerState(Player* player) :
	PlayerState (player)
{
	_stateType = PlayerStateType::STATE_JUMPING;

	_jumpForce = 100.0;	
	_gravity = 300.0;		
	_speed = 300;

	_originalPlayerAltitude = _player->GetPosition ().y;

	_jumpVelocity = player->GetForwardDirection () * _speed +
		vec3 (0, 1, 0) * _jumpForce;
}

JumpingPlayerState::~JumpingPlayerState()
{

}

void JumpingPlayerState::Update(double deltaTime)
{
	const vec3 gravityDirection = vec3 (0, -1, 0) * _gravity;

	_jumpVelocity = _jumpVelocity + gravityDirection * (float) deltaTime;

	vec3 newPosition = _player->GetPosition () + _jumpVelocity * (float) deltaTime;
	_player->SetPosition (newPosition);

	if (newPosition.y < _originalPlayerAltitude) {
		newPosition.y = _originalPlayerAltitude;
		_player->SetPosition (newPosition);

		_player->SetState (new RunningPlayerState (_player));
	}
}