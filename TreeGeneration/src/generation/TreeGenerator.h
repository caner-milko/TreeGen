#pragma once
#include <vector>
#include <memory>
#include "TreeWorld.h"
namespace tgen::gen {
class TreeGenerator
{
public:
	Tree& createTree(TreeWorld& world, vec3 position, TreeGrowthData growthData = {});
	void iterateWorld(TreeWorld& world, int count = 1, bool updateRenderers = true);
	void growTree(Tree& tree);
};
}