/* NekoEngine
 *
 * Profiler.h
 * Author: Alexandru Naiman
 *
 * Engine profiler
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

#include <vector>

#include <Engine/Engine.h>
#include <GUI/GUIManager.h>
#include <Profiler/Profiler.h>

using namespace std;
using namespace glm;

struct ProfilerMarker
{
	const char *name;
	double time;
	vec3 color;
};

struct ProfilerRegion
{
	const char *name;
	double start;
	double end;
	vec3 color;
	vector<ProfilerMarker> markers;
};

static vector<ProfilerRegion> _regions{};
static ProfilerRegion *_activeRegion{ nullptr };

void Profiler::BeginRegion(const char *name, vec3 color)
{
	size_t len{ strlen(name) };

	for (ProfilerRegion &region : _regions)
		if (!strncmp(region.name, name, len))
			return;

	_regions.push_back({ name, Engine::GetTime(), 0.0, color });
	_activeRegion = &_regions.back();
}

void Profiler::InsertMarker(const char *name, vec3 color)
{
	if(_activeRegion)
		_activeRegion->markers.push_back({ name, Engine::GetTime(), color });
}

void Profiler::EndRegion()
{
	if (!_activeRegion)
			return;

	_activeRegion->end = Engine::GetTime();
	_activeRegion = nullptr;
}

void Profiler::Draw()
{
	float y{ 0.f };
	float yIncrement{ (float)GUIManager::GetCharacterHeight() };

	for (ProfilerRegion &region : _regions)
	{
		double duration{ region.end - region.start };
		double lastMarker{ region.start };

		GUIManager::DrawString(vec2(400.f, y), region.color, "%s %f ms", region.name, duration * 1000.0);
		y += yIncrement;

		for (ProfilerMarker &marker : region.markers)
		{
			double markerDuration{ marker.time - lastMarker };
			lastMarker = marker.time;

			GUIManager::DrawString(vec2(420.f, y), marker.color, "%s %f ms", marker.name, markerDuration * 1000.0);
			y += yIncrement;
		}
	}

	_regions.clear();
}
