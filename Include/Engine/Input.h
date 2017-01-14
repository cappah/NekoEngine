/* NekoEngine
 *
 * Input.h
 * Author: Alexandru Naiman
 *
 * Input class definition
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

#pragma once

#include <math.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <Engine/Engine.h>
#include <Engine/Keycodes.h>
#include <Platform/Platform.h>

#define NE_MOUSE_X			0x00
#define NE_MOUSE_Y			0x01

#define NE_GPAD0_LX			0x02
#define NE_GPAD0_LY			0x03
#define NE_GPAD0_RX			0x04
#define NE_GPAD0_RY			0x05
#define NE_GPAD0_TL			0x06
#define NE_GPAD0_TR			0x07

#define NE_GPAD1_LX			0x12
#define NE_GPAD1_LY			0x13
#define NE_GPAD1_RX			0x14
#define NE_GPAD1_RY			0x15
#define NE_GPAD1_TL			0x16
#define NE_GPAD1_TR			0x17

#define NE_GPAD2_LX			0x22
#define NE_GPAD2_LY			0x23
#define NE_GPAD2_RX			0x24
#define NE_GPAD2_RY			0x25
#define NE_GPAD2_TL			0x26
#define NE_GPAD2_TR			0x27

#define NE_GPAD3_LX			0x32
#define NE_GPAD3_LY			0x33
#define NE_GPAD3_RX			0x34
#define NE_GPAD3_RY			0x35
#define NE_GPAD3_TL			0x36
#define NE_GPAD_TR			0x37

#define NE_VIRT_AXIS		0x40

#ifdef ENGINE_INTERNAL
#define NE_MAX_CONTROLLERS	4

typedef struct CTRL_STATE
{
	float left_x;
	float left_y;
	float left_trigger;
	float right_x;
	float right_y;
	float right_trigger;
} ControllerState;
#endif

struct VirtualAxis
{
	uint8_t min;
	uint8_t max;
	float val;
	bool active;
};

class Input
{
public:
	static int Initialize(bool enableMouseAxis);

	ENGINE_API static bool EnableMouseAxis(bool enable);

	ENGINE_API static void AddButtonMapping(std::string map, uint8_t button) { _buttonMap.insert(std::make_pair(map, button)); }
	ENGINE_API static void AddAxisMapping(std::string map, uint8_t axis) { _axisMap.insert(std::make_pair(map, axis)); }
	ENGINE_API static void AddVirtualAxis(uint8_t min, uint8_t max) { _virtualAxes.Add({ min, max, 0.f, false }); }
	ENGINE_API static void SetAxisSensivity(uint8_t axis, float sensivity);

	ENGINE_API static bool GetButton(uint8_t key) noexcept;
	ENGINE_API static bool GetButtonUp(uint8_t key) noexcept;
	ENGINE_API static bool GetButtonDown(uint8_t key) noexcept;

	ENGINE_API static float GetAxis(uint8_t axis) noexcept;

	ENGINE_API static bool GetButton(std::string key) noexcept { return GetButton(_buttonMap[key]); };
	ENGINE_API static bool GetButtonUp(std::string key) noexcept { return GetButtonUp(_buttonMap[key]); }
	ENGINE_API static bool GetButtonDown(std::string key) noexcept { return GetButtonDown(_buttonMap[key]); }

	ENGINE_API static float GetAxis(std::string axis) noexcept { return GetAxis(_axisMap[axis]); }

	ENGINE_API static int GetConnectedControllerCount() noexcept { return _connectedControllers; }

	ENGINE_API static bool PointerCaptured() noexcept { return _pointerCaptured; }

	ENGINE_API static char KeycodeToChar(uint8_t keyCode, bool shift) noexcept;

	static void Release();

#if defined(ENGINE_INTERNAL) || defined(EDITOR_INTERNAL)
	// Internal functions
	static void Key(int key, bool isPressed) noexcept;
	static void Update() noexcept;
	static void ClearKeyState() noexcept;

	static const std::vector<uint8_t> &GetKeyUpList() { return _keyUp; }
	static const std::vector<uint8_t> &GetKeyDownList() { return _keyDown; }
	static const std::vector<uint8_t> &GetPressedKeysList() { return _pressedKeys; }
#endif

	// Platform-specific functions (Implemented in PlatformInput.cpp)
	ENGINE_API static bool CapturePointer();
	ENGINE_API static void ReleasePointer();
	ENGINE_API static bool GetPointerPosition(long &x, long &y);
	ENGINE_API static bool SetPointerPosition(long x, long y);
	ENGINE_API static bool SetControllerVibration(int n, float left, float right);

private:
	static std::vector<uint8_t> _pressedKeys, _keyDown, _keyUp;
	static NArray<VirtualAxis> _virtualAxes;
	static std::unordered_map<int, uint8_t> _keymap;
	static float _screenHalfWidth, _screenHalfHeight;
	static float _mouseAxis[2];
	static float _sensivity[26];
	static std::unordered_map<std::string, uint8_t> _buttonMap;
	static std::unordered_map<std::string, uint8_t> _axisMap;
	static int _connectedControllers;
	static bool _enableMouseAxis;
	static bool _pointerCaptured;
	
#ifdef ENGINE_INTERNAL
	static ControllerState _controllerState[NE_MAX_CONTROLLERS];

	// Platform-specific internal functions
	static void _InitializeKeymap();
	static int _GetControllerCount();
	static bool _GetControllerState(int n, ControllerState *state);

	static inline void _deadzone(float &x, float &y, float deadzone)
	{
		float magnitude = sqrt(x*x + y*y);

		if (magnitude < deadzone)
		{
			x = 0;
			y = 0;
		}
		else
		{
			x = (x / magnitude) * ((magnitude - deadzone) / (1.f - deadzone));
			y = (y / magnitude) * ((magnitude - deadzone) / (1.f - deadzone));
		}
	}
#endif
};

#if defined(_MSC_VER)
template class ENGINE_API NArray<VirtualAxis>;
#endif
