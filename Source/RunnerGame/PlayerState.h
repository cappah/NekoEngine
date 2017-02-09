/* NekoEngine
*
* PlayerState.h
* Author: Cristian Lambru
*
* PlayerState class definition
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

#pragma once

#include "RunnerGame.h"
#include <Scene/Object.h>

enum PlayerStateType
{
	STATE_IDLE = 0,
	STATE_RUNNING,
	STATE_JUMPING,
	STATE_CROUCHING,
	STATE_TURN_RIGHT,
	STATE_TURN_LEFT,
	STATE_DEAD
};

class Player;

class PlayerState
{
protected:
	Player* _player;
	PlayerStateType _stateType;

public:
	PlayerState(Player* player);
	virtual ~PlayerState();

	virtual void OnHit (Object* other);

	virtual void Update(double deltaTime) = 0;
	virtual PlayerStateType GetStateType () const;
};