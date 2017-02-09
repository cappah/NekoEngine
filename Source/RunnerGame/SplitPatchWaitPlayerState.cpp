/* RunnerGame
*
* SplitPatchWaitPlayerState.cpp
* Author: Cristian Lambru
*
* SplitPatchWaitPlayerState class
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

#include "SplitPatchWaitPlayerState.h"

#include "TurnLeftPlayerState.h"
#include "TurnRightPlayerState.h"

#include <Engine/Input.h>
#include <Engine/Keycodes.h>

SplitPatchWaitPlayerState::SplitPatchWaitPlayerState(Player* player) :
	PlayerState(player)
{
	_stateType = PlayerStateType::STATE_SPLIT_PATCH_WAITING;

	_speed = 300.f;
	_duration = .75f; // seconds
	_durationAvailableForInput = .5f; // seconds

	_timeElapsed = 0;
	_chosenDirection = 0; // none
}

SplitPatchWaitPlayerState::~SplitPatchWaitPlayerState()
{

}

void SplitPatchWaitPlayerState::Update(double deltaTime)
{
	bool right = false;

#ifndef NE_DEVICE_MOBILE
	right = Input::GetButtonDown(NE_KEY_D);
#else
	// ios
#endif

	// Only first chose is counted, the others are thrown away
	if (right && _chosenDirection == 0 && _timeElapsed < _durationAvailableForInput)
		_chosenDirection = 1;

	bool left = false;

#ifndef NE_DEVICE_MOBILE
	left = Input::GetButtonDown(NE_KEY_A);
#else
	// ios
#endif

	// Only first chose is counted, the others are thrown away
	if (left && _chosenDirection == 0 && _timeElapsed < _durationAvailableForInput)
		_chosenDirection = 2;

	vec3 velocity = _player->GetForwardDirection() * _speed * (float)deltaTime;
	vec3 newPosition = _player->GetPosition() + velocity;

	_player->SetPosition(newPosition);

	_timeElapsed += deltaTime;

	if (_timeElapsed > _durationAvailableForInput && _chosenDirection != 0)
	{
		if (_chosenDirection == 1)
			_player->SetState(new TurnRightPlayerState(_player));
		else
			_player->SetState (new TurnLeftPlayerState(_player));
	}

	if (_timeElapsed > _duration)
		_player->Kill();
}