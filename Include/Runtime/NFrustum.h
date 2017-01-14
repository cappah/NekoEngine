/* NekoEngine
 *
 * NFrustum.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Runtime
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

#include <stdint.h>
#include <Engine/Defs.h>

#include <Runtime/NBounds.h>

struct NFrustumPlane
{
	glm::vec3 normal;
	float distance;

	inline float DistanceToPoint(const glm::vec3 &pt) const
	{
		return glm::dot(normal, pt) + distance;
	}

	inline void InitWithNormalAndPoint(const glm::vec3 &n, const glm::vec3 &pt)
	{
		normal = normalize(n);
		distance = -(normal.x * pt.x + normal.y * pt.y + normal.z * pt.z);
	}
};

class NFrustum
{
public:
	NFrustum()
	{
		for (int i = 0; i < 6; ++i)
			_frustumPlanes[i] = { glm::vec3(0.f), 0.f };
	}

	const NFrustumPlane &GetPlane(int plane) const { return _frustumPlanes[plane]; }

	void FromViewProjection(glm::mat4 &mat)
	{
		_frustumPlanes[0] = { glm::vec3(mat[0][3] + mat[0][0],
										mat[1][3] + mat[1][0],
										mat[2][3] + mat[2][0]),
							  mat[3][3] + mat[3][0] };

		_frustumPlanes[1] = { glm::vec3(mat[0][3] - mat[0][0],
										mat[1][3] - mat[1][0],
										mat[2][3] - mat[2][0]),
							  mat[3][3] - mat[3][0] };

		_frustumPlanes[2] = { glm::vec3(mat[0][3] - mat[0][1],
										mat[1][3] - mat[1][1],
										mat[2][3] - mat[2][1]),
							  mat[3][3] - mat[3][1] };

		_frustumPlanes[3] = { glm::vec3(mat[0][3] + mat[0][1],
										mat[1][3] + mat[1][1],
										mat[2][3] + mat[2][1]),
							  mat[3][3] + mat[3][1] };

		_frustumPlanes[4] = { glm::vec3(mat[0][3] + mat[0][2],
										mat[1][3] + mat[1][2],
										mat[2][3] + mat[2][2]),
							  mat[3][3] + mat[3][2] };

		_frustumPlanes[5] = { glm::vec3(mat[0][3] - mat[0][2],
										mat[1][3] - mat[1][2],
										mat[2][3] - mat[2][2]),
						      mat[3][3] - mat[3][2] };

		for (uint8_t i = 0; i < 6; ++i)
		{
			float len = glm::length(_frustumPlanes[i].normal);
			_frustumPlanes[i].normal /= len;
			_frustumPlanes[i].distance /= len;
		}
	}

	bool ContainsBounds(const NBounds &bounds) const
	{
		if (bounds.HaveSphere() && !_ContainsSphere(bounds.GetSphere()))
			return false;

		//if (bounds.HaveBox() && !_ContainsBox(bounds.GetBox()))
		//	return false;

		return true;
	}

private:
	NFrustumPlane _frustumPlanes[6];
	float _ratio, _angle, _near, _far, _tg;
	float _nHeight, _nWidth, _fHeight, _fWidth;

	inline bool _ContainsSphere(const NBoundingSphere &sphere) const
	{
		for (uint8_t i = 0; i < 6; ++i)
			if (_frustumPlanes[i].DistanceToPoint(sphere.GetCenter()) < -sphere.GetRadius())
				return false;

		return true;
	}

	inline bool _ContainsBox(const NBoundingBox &box) const
	{
		uint8_t out = 0;

		for (uint8_t i = 0; i < 6; ++i)
		{
			out += glm::dot(_frustumPlanes[i].normal, box.GetMin()) < 0.f ? 1 : 0;
			out += glm::dot(_frustumPlanes[i].normal, glm::vec3(box.GetMax().x, box.GetMin().y, box.GetMin().z)) < 0.f ? 1 : 0;
			out += glm::dot(_frustumPlanes[i].normal, glm::vec3(box.GetMin().x, box.GetMax().y, box.GetMin().z)) < 0.f ? 1 : 0;
			out += glm::dot(_frustumPlanes[i].normal, glm::vec3(box.GetMax().x, box.GetMax().y, box.GetMin().z)) < 0.f ? 1 : 0;
			out += glm::dot(_frustumPlanes[i].normal, glm::vec3(box.GetMin().x, box.GetMin().y, box.GetMax().z)) < 0.f ? 1 : 0;
			out += glm::dot(_frustumPlanes[i].normal, glm::vec3(box.GetMax().x, box.GetMin().y, box.GetMax().z)) < 0.f ? 1 : 0;
			out += glm::dot(_frustumPlanes[i].normal, glm::vec3(box.GetMin().x, box.GetMax().y, box.GetMax().z)) < 0.f ? 1 : 0;
			out += glm::dot(_frustumPlanes[i].normal, glm::vec3(box.GetMax().x, box.GetMax().y, box.GetMax().z)) < 0.f ? 1 : 0;

			if (out == 8)
				return false;
		}

		return true;
	}
};
