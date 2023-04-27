#pragma once
#include "Common.h"
#include <unordered_map>
#include <map>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
namespace tgen::gen
{
using GrowthDataId = uint32;
struct TreeGrowthData
{
	bool grow = true;

	float apicalControl = 0.5f;
	float vigorMultiplier = 2.0f;
	float baseLength = .02f;

	float baseRadius = 0.0005f;
	float radiusN = 2.5f;
	float branchCurviness = 0.5f;
	//lateral
	float lateralAngle = glm::radians(45.0f);

	vec3 tropism = vec3(0.0f, 1.0f, 0.0f);
	//default, optimal, tropism
	vec2 directionWeights = vec2(0.1f, 0.1f);

	//shedding
	bool shouldShed = true;
	float shedMultiplier = 0.3f;
	float shedExp = 1.5f;

	//leaf params
	int32 leafMaxChildCount = 5;
	int32 leafMinOrder = 4;
	float leafDensity = 60.0f;
	float leafSizeMultiplier = 0.4f;
};
struct TreeWorldGrowthData
{
	std::map<GrowthDataId, TreeGrowthData> presets;
	std::unordered_map<ivec3, GrowthDataId> colorToPresetMap;
	//shadows
	float fullExposure = 2.5f;
	int32 pyramidHeight = 6;
	float a = 0.8f;
	//b > 1
	float b = 1.5f;
};
}
