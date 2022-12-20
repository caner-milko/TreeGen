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
		if (glm::abs(dot(original, ox)) > .999f)
		{
			ox = vec3(0.0f, 0.0f, 1.0f);
		}

		vec3 cross = normalize(glm::cross(original, ox));

		float s = IntNoise2D(seed) * 0.5f + 0.5f;
		float r = IntNoise2D(seed, 1) * 0.5f + 0.5f;

		float h = glm::cos(angle);

		float phi = 2.0f * glm::pi<float>() * s;

		float z = h + (1.0f - h) * r;

		float sinT = glm::sqrt(1.0f - z * z);

		float x = glm::cos(phi) * sinT;
		float y = glm::sin(phi) * sinT;

		return glm::normalize(ox * x + cross * y + original * z);
	}
	bool util::endsWith(std::string_view str, std::string_view suffix)
	{
		return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
	}

	bool util::startsWith(std::string_view str, std::string_view prefix)
	{
		return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
	}
}