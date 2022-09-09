#pragma once
#include "./Definitions.h"
#include <glm/gtc/constants.hpp>
#include <iostream>
namespace util {
	uint32 hash(uint32 a);

	//returns double between -1.0 and 1.0
	double IntNoise2D(int x, int y = 0);


	//https://stackoverflow.com/questions/2659257/perturb-vector-by-some-angle
	vec3 randomPerturbateVector(vec3 original, float angle, int seed);
	static bool endsWith(const std::string& str, const std::string& suffix);

	static bool startsWith(const std::string& str, const std::string& prefix);

	static bool endsWith(const std::string& str, const char* suffix, uint32_t suffixLen);

	static bool endsWith(const std::string& str, const char* suffix);

	static bool startsWith(const std::string& str, const char* prefix, uint32_t prefixLen);

	static bool startsWith(const std::string& str, const char* prefix);
}