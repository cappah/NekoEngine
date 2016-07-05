/* Neko Engine
 *
 * Light.cpp
 * Author: Alexandru Naiman
 *
 * Light implementation
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

#define ENGINE_INTERNAL

#include <Scene/Light.h>

#define LIGHT_MODULE	"Light"

ENGINE_REGISTER_OBJECT_CLASS(Light);

using namespace glm;

Light::Light(ObjectInitializer *initializer)
	: Object(initializer)
{
	if(!initializer)
	{
		Logger::Log(LIGHT_MODULE, LOG_WARNING, "No initializer supplied, using default values");
		
		_intensity = 1.f;
		_attenuation = vec2(0.f);
		_direction = vec3(0.f);
		_type = LightType::Directional;
		_castShadows = false;
		
		return;
	}
	
	ArgumentMapType::iterator it;
	const char *ptr = nullptr;

	if (((it = initializer->arguments.find("intensity")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_intensity = (float)atof(ptr);
	else
		_intensity = 1.f;
	
	if (((it = initializer->arguments.find("attenuation")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		EngineUtils::ReadFloatArray(ptr, 2, &_attenuation.x);
	else
		_attenuation = vec2(0.f);
	
	if (((it = initializer->arguments.find("direction")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		EngineUtils::ReadFloatArray(ptr, 3, &_direction.x);
	else
		_direction = vec3(0.f);

	if (((it = initializer->arguments.find("shadows")) != initializer->arguments.end()) && ((ptr = it->second.c_str()) != nullptr))
		_castShadows = true;
	else
		_castShadows = false;
	
	if (((it = initializer->arguments.find("type")) == initializer->arguments.end()) || ((ptr = it->second.c_str()) == nullptr))
	{
		_type = LightType::Directional;
		return;
	}
	
	size_t len = strlen(ptr);
	
	if(!strncmp(ptr, "directional", len))
		_type = LightType::Directional;
	else if(!strncmp(ptr, "point", len))
		_type = LightType::Point;
	else if(!strncmp(ptr, "spot", len))
		_type = LightType::Spot;
	else
		_type = LightType::Directional;
}
