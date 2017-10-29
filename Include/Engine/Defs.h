/* NekoEngine
 *
 * Defs.h
 * Author: Alexandru Naiman
 *
 * Engine definitions
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

#include <Platform/PlatformDetect.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef NE_PLATFORM_WINDOWS
	#ifdef ENGINE_INTERNAL
		#define ENGINE_API	__declspec(dllexport)
	#else
		#define ENGINE_API	__declspec(dllimport)
	#endif
#else
	#define ENGINE_API
#endif

#define NE_PATH_SIZE			1024

// Return codes

#define ENGINE_OK							0
#define ENGINE_FAIL							-1

// Initialization error codes

#define ENGINE_PIPELINE_INIT_FAIL				-1000
#define ENGINE_INSTANCE_CREATE_FAIL				-1001
#define ENGINE_DEVICE_CREATE_FAIL				-1002
#define ENGINE_SURFACE_CREATE_FAIL				-1003
#define ENGINE_SWAPCHAIN_CREATE_FAIL			-1004
#define ENGINE_CMDPOOL_CREATE_FAIL				-1005
#define ENGINE_FRAMEUBFFER_CREATE_FAIL			-1006
#define ENGINE_CMDBUFFER_CREATE_FAIL			-1007
#define ENGINE_DEBUG_INIT_FAIL					-1008
#define ENGINE_SEMAPHORE_CREATE_FAIL			-1009
#define ENGINE_DESCRIPTOR_SET_CREATE_FAIL		-1010
#define ENGINE_DESCRIPTOR_POOL_CREATE_FAIL		-1011
#define ENGINE_DESCRIPTOR_SET_LYT_CREATE_FAIL	-1012
#define ENGINE_PIPELINE_LYT_CREATE_FAIL			-1013
#define ENGINE_PIPELINE_CREATE_FAIL				-1014
#define ENGINE_LOAD_SHADER_FAIL					-1015
#define ENGINE_RENDERPASS_CREATE_FAIL			-1016

#define ENGINE_CMDBUFFER_BEGIN_FAIL				-2000
#define ENGINE_CMDBUFFER_RECORD_FAIL			-2001

#define ENGINE_NO_SCENE							-7
#define ENGINE_NOT_FOUND						-8
#define ENGINE_IN_USE							-9
#define ENGINE_INVALID_RES						-10
#define ENGINE_IO_FAIL							-11
#define ENGINE_NO_CAMERA						-12
#define ENGINE_INVALID_ARGS						-13
#define ENGINE_NOT_LOADED						-14
#define ENGINE_MEM_FAIL							-15
#define ENGINE_LOAD_GS_FAIL						-16
#define ENGINE_OUT_OF_RESOURCES					-17
#define ENGINE_DB_INIT_FAIL						-18

#define ENGINE_INVALID_HEADER					-3000
