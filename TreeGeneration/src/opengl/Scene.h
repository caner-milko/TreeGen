#pragma once
#include "../Definitions.h"
#include "Texture.h"
struct Scene {
	vec3 lightDir = glm::normalize(vec3(0.4, -0.6, -0.4));
	vec3 lightPos = 0.5f * vec3(-4.0f, 8.0f, 4.0f);
	vec3 ambientCol = 0.2f * vec3(0.2f, 0.2f, 0.15f);
	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
	mat4 LightVP;
	sp<Texture> shadowMap;
	vec4 ortho = vec4(-1.0f, 1.0f, -1.0f, 1.0f);
	float lightFarPlane = 10.0f;
	float lightNearPlane = 1.0f;
};

extern Scene Gscene;