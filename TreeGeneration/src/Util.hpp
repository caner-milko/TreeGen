#include "./Definitions.h"
#include <glm/gtc/constants.hpp>
#include <random>
#include <iostream>
namespace util {
	//https://stackoverflow.com/questions/2659257/perturb-vector-by-some-angle
	vec3 randomPerturbateVector(vec3 original, float angle)
	{
		auto ox = vec3(1.0f, 0.0f, 0.0f);
		vec3 random = ox;
		if (glm::abs(dot(original, random)) > .999f)
		{
			random = vec3(0.0f, 1.0f, 0.0f);
		}
		vec3 cross = normalize(glm::cross(original, random));
		float s = (float)rand() / RAND_MAX;
		float r = (float)rand() / RAND_MAX;
		float h = glm::cos(angle);
		float phi = 2.0f * glm::pi<float>() * s;
		float z = h + (1.0f - h) * r;
		float sinT = glm::sqrt(1.0f - z * z);
		float x = glm::cos(phi) * sinT;
		float y = glm::sin(phi) * sinT;
		return glm::normalize(random * x + cross * y + original * z);
	}
	static bool endsWith(const std::string& str, const std::string& suffix)
	{
		return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
	}

	static bool startsWith(const std::string& str, const std::string& prefix)
	{
		return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
	}

	static bool endsWith(const std::string& str, const char* suffix, uint32_t suffixLen)
	{
		return str.size() >= suffixLen && 0 == str.compare(str.size() - suffixLen, suffixLen, suffix, suffixLen);
	}

	static bool endsWith(const std::string& str, const char* suffix)
	{
		return endsWith(str, suffix, (uint32_t)std::string::traits_type::length(suffix));
	}

	static bool startsWith(const std::string& str, const char* prefix, uint32_t prefixLen)
	{
		return str.size() >= prefixLen && 0 == str.compare(0, prefixLen, prefix, prefixLen);
	}

	static bool startsWith(const std::string& str, const char* prefix)
	{
		return startsWith(str, prefix, (uint32_t)std::string::traits_type::length(prefix));
	}
}