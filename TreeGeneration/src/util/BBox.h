#pragma once
#include "Common.h"
namespace tgen::util
{
struct BBox
{
	vec3 min, max;

	BBox(vec3 min, vec3 max) : min(glm::min(min, max)), max(glm::max(min, max))
	{
	}

	mat4 asModel() inline const {
		vec3 center = (max + min) / 2.0f;
		return glm::scale(glm::translate(glm::mat4(1.0f), center), (max - min));
	}


};
}