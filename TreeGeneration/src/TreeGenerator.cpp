#include "TreeGenerator.h"

Tree* TreeGenerator::createTree(TreeWorld& world, vec3 position, TreeGrowthData growthData)
{
	return world.createTree(position, growthData);
}

void TreeGenerator::growTree(Tree& tree)
{
	tree.age++;
	tree.startGrow();
	tree.world.calculateShadows();

	tree.accumulateLight();
	tree.distributeVigor();


	tree.addNewShoots();
	if (tree.growthData.shouldShed) {
		tree.shedBranchs();
	}

	tree.calculateChildCount();

	tree.endGrow();


}