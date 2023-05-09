#pragma once
#include "Common.h"
#include <vector>
namespace tgen::graphics {
struct CubicBezier
{
	vec3 A, B, C, D;
	CubicBezier(vec3 A, vec3 B, vec3 C, vec3 D) : A(A), B(B), C(C), D(D) {}
	vec3 calculate(float t)
	{
		float mint = 1.f - t;
		return mint * mint * mint * A + 3 * mint * mint * t * B + 3 * mint * t * t * C + t * t * t * D;
	}
};

struct CameraPath
{
	std::vector<CubicBezier> beziers;
	vec3 start;
	bool repeat = true;

	CameraPath(vec3 start) : start(start)
	{

	}

	vec3 calculate(float t)
	{
		if (beziers.size() == 0)
			return vec3();
		int floored = glm::floor(t);
		t = t - floored;
		if (repeat)
		{
			floored = floored % beziers.size();
		}
		floored = glm::max(0, glm::min(floored, (int)beziers.size() - 1));
		auto& selected = beziers[floored];
		return selected.calculate(t);
	}

	void addNew(vec3 B, vec3 C, vec3 D)
	{
		vec3 A = start;
		if (beziers.size() > 0)
			A = beziers[beziers.size() - 1].D;
		beziers.push_back(CubicBezier(A, B, C, D));
	}

};

}