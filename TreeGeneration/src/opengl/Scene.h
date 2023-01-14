#pragma once
#include "../Definitions.h"
struct Scene {
	vec3 lightDir = glm::normalize(vec3(0.4, -0.6, -0.4));
	vec3 ambientCol = 0.2f * vec3(0.2f, 0.2f, 0.15f);
	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
};