#pragma once
#include "Definitions.h"
struct BBox {
	vec3 min, max;

	//default length of a cube's any axis is 2
	mat4 asModel() inline const {
		vec3 center = (max + min) / 2.0f;
		return glm::scale(glm::translate(glm::mat4(1.0f), center), (max - min) / 2.0f);
	}


};