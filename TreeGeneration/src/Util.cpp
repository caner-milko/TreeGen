#include "Util.hpp"
namespace util {

	uint32 util::hash(uint32 a) {
		a = (a ^ 61) ^ (a >> 16);
		a = a + (a << 3);
		a = a ^ (a >> 4);
		a = a * 0x27d4eb2d;
		a = a ^ (a >> 15);
		return a;
	}

	double util::IntNoise2D(int x, int y)
	{
		int n = x + y * 57;
		n = (n << 13) ^ n;
		return (double)(1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
	}


	//https://stackoverflow.com/questions/2659257/perturb-vector-by-some-angle
	vec3 util::randomPerturbateVector(vec3 original, float angle, int seed)
	{
		auto ox = vec3(1.0f, 0.0f, 0.0f);
		vec3 random = ox;
		if (glm::abs(dot(original, random)) > .999f)
		{
			random = vec3(0.0f, 1.0f, 0.0f);
		}

		vec3 cross = normalize(glm::cross(original, random));

		float s = IntNoise2D(seed) * 0.5f + 0.5f;
		float r = IntNoise2D(seed, 1) * 0.5f + 0.5f;

		float h = glm::cos(angle);

		float phi = 2.0f * glm::pi<float>() * s;

		float z = h + (1.0f - h) * r;

		float sinT = glm::sqrt(1.0f - z * z);

		float x = glm::cos(phi) * sinT;
		float y = glm::sin(phi) * sinT;

		return glm::normalize(random * x + cross * y + original * z);
	}
	static bool util::endsWith(const std::string& str, const std::string& suffix)
	{
		return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
	}

	static bool util::startsWith(const std::string& str, const std::string& prefix)
	{
		return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
	}

	static bool util::endsWith(const std::string& str, const char* suffix, uint32_t suffixLen)
	{
		return str.size() >= suffixLen && 0 == str.compare(str.size() - suffixLen, suffixLen, suffix, suffixLen);
	}

	static bool util::endsWith(const std::string& str, const char* suffix)
	{
		return endsWith(str, suffix, (uint32_t)std::string::traits_type::length(suffix));
	}

	static bool util::startsWith(const std::string& str, const char* prefix, uint32_t prefixLen)
	{
		return str.size() >= prefixLen && 0 == str.compare(0, prefixLen, prefix, prefixLen);
	}

	static bool util::startsWith(const std::string& str, const char* prefix)
	{
		return startsWith(str, prefix, (uint32_t)std::string::traits_type::length(prefix));
	}
}