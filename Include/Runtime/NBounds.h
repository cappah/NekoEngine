/* NekoEngine
 *
 * NBounds.h
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

#define NBOX_MIN	7
#define NBOX_MAX	2

struct NBoundingSphere
{
public:
	NBoundingSphere() :
		_center(glm::vec3(0.f)),
		_radius(0.f)
	{ }

	NBoundingSphere(glm::vec3 center, float radius) :
		_center(center),
		_radius(radius)
	{ }

	const glm::vec3 &GetCenter() const { return _center; }
	float GetRadius() const { return _radius; }

	void SetCenter(glm::vec3 center) { _center = center; }
	void SetRadius(float radius) { _radius = radius; }

	void Transform(glm::mat4 &matrix)
	{
		glm::vec3 edge = _center + glm::vec3(1.f, 0.f, 0.f) * _radius;
		_center = matrix * glm::vec4(_center, 1.f);
		edge = matrix * glm::vec4(edge, 1.f);
		_radius = glm::distance(_center, edge);
	}

private:
	glm::vec3 _center;
	float _radius;
};

class NBoundingBox
{
public:
	NBoundingBox() :
		_center(glm::vec3(0.f)),
		_half(glm::vec3(0.f))
	{ }

	NBoundingBox(glm::vec3 min, glm::vec3 max)
	{
		InitWithMinMax(min, max);
	}

	const glm::vec3 *GetVertices() const { return _corners; }
	const glm::vec3 &GetMin() const { return _corners[NBOX_MIN]; }
	const glm::vec3 &GetMax() const { return _corners[NBOX_MAX]; }
	const glm::vec3 &GetCenter() const { return _center; }
	const glm::vec3 &GetExtents() const { return _extents; }
	const glm::vec3 &GetHalf() const { return _half; }

	void InitWithMinMax(glm::vec3 &min, glm::vec3 &max)
	{
		_corners[0] = glm::vec3(min.x, max.y, min.z);
		_corners[1] = glm::vec3(max.x, max.y, min.z);
		_corners[2] = max;
		_corners[3] = glm::vec3(min.x, max.y, max.z);
		_corners[4] = glm::vec3(min.x, min.y, max.z);
		_corners[5] = glm::vec3(max.x, min.y, max.z);
		_corners[6] = glm::vec3(max.x, min.y, min.z);
		_corners[7] = min;

		_center = glm::vec3((float)(min.x + max.x) / 2.f, (float)(min.y + max.y) / 2.f, (float)(min.z + max.z) / 2.f);
		// _center = glm::vec3((max.x + min.x) * .5f, (max.y + min.y) * .5f, (max.z + min.z) * .5f);
		_half = abs((max - min) * .5f);
	}

	void CreateSphere(NBoundingSphere &sphere)
	{
		sphere.SetCenter(_center);
		sphere.SetRadius(sqrtf(powf(_corners[2].x, 2.f) + powf(_corners[2].y, 2.f) + powf(_corners[2].z, 2.f)) / 2.f);
	}

	void Transform(glm::mat4 &matrix)
	{
		/*glm::vec3 min, max;

		for (uint8_t i = 0; i < 8; ++i)
		{
			_corners[i] = matrix * glm::vec4(_corners[i], 1.f);
			
			if (_corners[i].x < min.x) min.x = _corners[i].x;
			else if (_corners[i].x > max.x) max.x = _corners[i].x;

			if (_corners[i].y < min.y) min.y = _corners[i].y;
			else if (_corners[i].y > max.x) max.y = _corners[i].y;

			if (_corners[i].z < min.z) min.z = _corners[i].z;
			else if (_corners[i].z > max.z) max.z = _corners[i].z;
		}

		InitWithMinMax(min, max);*/
	}

	double SquaredDistanceToPoint(const glm::vec3 &pt) const
	{
		std::function<double(const double, const double, const double)> calcDistance{
			[&](const double p, const double min, const double max) -> double
		{
			double ret{ 0.0 };

			if (p < min)
				ret = min - p;
			else if (p > max)
				ret = p - max;

			return ret * ret;
		} };

		return calcDistance(pt.x, _corners[NBOX_MIN].x, _corners[NBOX_MAX].x) 
			+ calcDistance(pt.y, _corners[NBOX_MIN].y, _corners[NBOX_MAX].y)
			+ calcDistance(pt.z, _corners[NBOX_MIN].z, _corners[NBOX_MAX].z);
	}

private:
	glm::vec3 _corners[8];
	glm::vec3 _center, _half, _extents;

	void _SetCorners()
	{

	}
};

class NBounds
{
public:
	NBounds()
	{
		_haveSphere = _haveBox = false;
	}

	void Init(glm::vec3 center, glm::vec3 min, glm::vec3 max, float radius)
	{
		_sphere.SetRadius(radius);
		_sphere.SetCenter(center);
		_box.InitWithMinMax(min, max);
		_haveSphere = true;
		_haveBox = true;
	}

	void InitSphere(glm::vec3 center, float radius)
	{
		_sphere.SetCenter(center);
		_sphere.SetRadius(radius);
		_haveSphere = true;
	}

	void InitBox(glm::vec3 min, glm::vec3 max)
	{
		_box.InitWithMinMax(min, max);
		_box.CreateSphere(_sphere);
		_haveSphere = true;
		_haveBox = true;
	}

	const glm::vec3 &GetCenter() const
	{
		if (!_haveBox)
			return _sphere.GetCenter();
		return _box.GetCenter();
	}

	const bool IsValid() const { return _haveSphere || _haveBox; }

	const bool HaveSphere() const { return _haveSphere; }
	const NBoundingSphere &GetSphere() const { return _sphere; }

	const bool HaveBox() const { return _haveBox; }
	const NBoundingBox &GetBox() const { return _box; }

	void SetCenter(glm::vec3 &center)
	{
		_sphere.SetCenter(center);
	}

	bool Contains(const NBounds &bounds) const
	{
		const glm::vec3 &center = GetCenter();
		const glm::vec3 &otherCenter = bounds.GetCenter();

		if (_haveBox && bounds._haveBox)
		{
			bool x = fabs(center.x - otherCenter.x) <= (_box.GetHalf().x + bounds._box.GetHalf().x);
			bool y = fabs(center.y - otherCenter.y) <= (_box.GetHalf().y + bounds._box.GetHalf().y);
			bool z = fabs(center.z - otherCenter.z) <= (_box.GetHalf().z + bounds._box.GetHalf().z);

			return x && y && z;
		}
		else if (_haveBox && bounds._haveSphere)
			return _box.SquaredDistanceToPoint(bounds._sphere.GetCenter()) <= (bounds._sphere.GetRadius() * bounds._sphere.GetRadius());
		else if (_haveSphere && bounds._haveBox)
			return bounds._box.SquaredDistanceToPoint(_sphere.GetCenter()) <= (_sphere.GetRadius() * _sphere.GetRadius());		
		else if (_haveSphere && bounds._haveSphere)
			return glm::distance(_sphere.GetCenter(), bounds._sphere.GetCenter()) + bounds._sphere.GetRadius() < _sphere.GetRadius();

		return false;
	}

	bool Intersects(const NBounds &bounds) const
	{
		const glm::vec3 &center = GetCenter();
		const glm::vec3 &otherCenter = bounds.GetCenter();

		if (_haveBox && bounds._haveBox)
		{
			bool x = fabs(center.x - otherCenter.x) <= (_box.GetHalf().x + bounds._box.GetHalf().x);
			bool y = fabs(center.y - otherCenter.y) <= (_box.GetHalf().y + bounds._box.GetHalf().y);
			bool z = fabs(center.z - otherCenter.z) <= (_box.GetHalf().z + bounds._box.GetHalf().z);

			return x && y && z;
		}
		else if (_haveBox && bounds._haveSphere)
			return _box.SquaredDistanceToPoint(bounds._sphere.GetCenter()) <= (bounds._sphere.GetRadius() * bounds._sphere.GetRadius());
		else if (_haveSphere && bounds._haveBox)
			return bounds._box.SquaredDistanceToPoint(_sphere.GetCenter()) <= (_sphere.GetRadius() * _sphere.GetRadius());		
		else if (_haveSphere && bounds._haveSphere)
			return glm::distance(_sphere.GetCenter(), bounds._sphere.GetCenter()) < (_sphere.GetRadius() + bounds._sphere.GetRadius());

		return false;
	}

	void Transform(glm::mat4 &matrix, NBounds *out = nullptr)
	{
		if (!out)
			out = this;
		else
		{
			if (_haveSphere)
				out->_sphere = _sphere;
			out->_haveSphere = _haveSphere;

			if (_haveBox)
				out->_box = _box;
			out->_haveBox = _haveBox;
		}

		out->_sphere.Transform(matrix);
		out->_box.Transform(matrix);
	}

private:
	NBoundingBox _box;
	NBoundingSphere _sphere;
	bool _haveSphere, _haveBox;
};
