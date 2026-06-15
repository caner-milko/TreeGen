#pragma once
#include <vector>
#include <memory>
#include "TreeWorld.h"
namespace tgen::gen
{
class TreeGenerator
{
public:
	Tree& createTree(TreeWorld& world, vec2 position, GrowthDataId growthDataId = 0);
	std::vector<vec2> spreadSeeds(Tree& tree);
	bool shouldDie(Tree& tree);
	void iterateWorld(TreeWorld& world, int count = 1, bool updateRenderers = true);
	void growTree(Tree& tree);
};
}