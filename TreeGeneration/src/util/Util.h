#pragma once
#include "Common.h"
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <vector>
namespace tgen::util {
uint32 hash(uint32 a);

//returns double between -1.0 and 1.0
double IntNoise2D(int x, int y = 0, double min = -1.0, double max = 1.0);


//https://stackoverflow.com/questions/2659257/perturb-vector-by-some-angle
vec3 randomPerturbateVector(vec3 original, float angle, int seed);
bool endsWith(std::string_view str, std::string_view suffix);

bool startsWith(std::string_view str, std::string_view prefix);
std::vector<glm::vec2> DistributePoints(int seed, int points, glm::vec4 bounds);

}