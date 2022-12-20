#pragma once
#include <vector>
#include <memory>
#include "./TreeWorld.h"
class TreeGenerator
{
public:
	Tree* createTree(TreeWorld& world, vec3 position, TreeGrowthData growthData = {});
	void growTree(Tree& tree);
};
