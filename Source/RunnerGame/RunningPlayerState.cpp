/* RunnerGame
*
* RunningPlayerState.cpp
* Author: Cristian Lambru
*
* RunningPlayerState class
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

#include "RunningPlayerState.h"
#include "DemoAnimatorComponent.h"

#include "JumpingPlayerState.h"
#include "CrouchingPlayerState.h"

#include <Engine\Input.h>

RunningPlayerState::RunningPlayerState(Player* player) :
	PlayerState (player)
{
	_stateType = PlayerStateType::STATE_RUNNING;
}

RunningPlayerState::~RunningPlayerState()
{

}

void RunningPlayerState::Update(double deltaTime)
{
	bool jump = false;

#ifndef NE_DEVICE_MOBILE
	jump = Input::GetButtonDown(" ");
#else
	// ios
#endif

	if (jump) {
		_player->SetState(new JumpingPlayerState(_player));

		return;
	}

	bool crouch = false;

#ifndef NE_DEVICE_MOBILE
	jump = Input::GetButtonDown("s");
#else
	// ios
#endif

	if (crouch) {
		_player->SetState(new CrouchingPlayerState(_player));

		return;
	}


}