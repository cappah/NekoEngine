/* Neko Engine
 *
 * Input.h
 * Author: Alexandru Naiman
 *
 * Input class definition
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

#include <vector>
#include <string>
#include <unordered_map>
#include <Engine/Keycodes.h>
#include <Platform/Platform.h>

#define NE_MOUSE_X		0
#define NE_MOUSE_Y		1
#define NE_JOY_LT		2
#define NE_JOY_RT		3
#define NE_JOY_TRIG		4

class Input
{
public:
	static int Initialize();

	ENGINE_API static void AddButtonMapping(std::string map, uint8_t button) { _buttonMap.insert(std::make_pair(map, button)); }
	ENGINE_API static void AddAxisMapping(std::string map, uint8_t axis) { _axisMap.insert(std::make_pair(map, axis)); }

	ENGINE_API static bool GetKeyUp(uint8_t key) noexcept;
	ENGINE_API static bool GetKeyDown(uint8_t key) noexcept;

	ENGINE_API static float GetAxis(uint8_t axis) noexcept { return _axis[axis]; }

	ENGINE_API static bool GetKeyUp(std::string &key) noexcept { return GetKeyUp(_buttonMap[key]); }
	ENGINE_API static bool GetKeyDown(std::string &key) noexcept { return GetKeyDown(_buttonMap[key]); }

	ENGINE_API static float GetAxis(std::string &axis) noexcept { return GetAxis(_axisMap[axis]); }

	static void Release();

#ifdef ENGINE_INTERNAL
	// Internal functions
	static void Key(int key, bool bIsPressed) noexcept;
	static void Update() noexcept;
#endif

private:
	static std::vector<uint8_t> _pressedKeys;
	static std::unordered_map<int, uint8_t> _keymap;
	static float _screenHalfWidth, _screenHalfHeight;
	static float _axis[5];
	static std::unordered_map<std::string, uint8_t> _buttonMap;
	static std::unordered_map<std::string, uint8_t> _axisMap;

	// Platform-specific functions
	static void _InitializeKeymap();
};