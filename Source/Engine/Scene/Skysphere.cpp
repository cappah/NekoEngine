/* NekoEngine
 *
 * Skysphere.cpp
 * Author: Alexandru Naiman
 *
 * Skysphere implementation 
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

#include <Engine/Engine.h>
#include <Engine/CameraManager.h>
#include <Scene/Skysphere.h>

using namespace glm;
using namespace std;

ENGINE_REGISTER_OBJECT_CLASS(Skysphere)

void Skysphere::UpdateData(VkCommandBuffer commandBuffer) noexcept
{
	if (_buffer)
	{
		Camera *cam = CameraManager::GetActiveCamera();
		cam->EnableSkybox(true);
		_objectData.ModelViewProjection = cam->GetProjectionMatrix() * (cam->GetView() * _objectData.Model);
		_buffer->UpdateData((uint8_t *)&_objectData, 0, sizeof(_objectData), commandBuffer);
		cam->EnableSkybox(false);
	}

	for (pair<string, ObjectComponent*> kvp : _components)
		kvp.second->UpdateData(commandBuffer);
}