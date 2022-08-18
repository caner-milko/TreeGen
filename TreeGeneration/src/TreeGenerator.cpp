#include "TreeGenerator.h"

Tree& TreeGenerator::createTree(TreeWorld& world, vec3 position)
{
	return world.createTree(position);
}

void TreeGenerator::growTree(Tree& tree)
{
	tree.world.calculateShadows();

	tree.accumulateLight();
	tree.root->vigor = tree.root->light;
	tree.distributeVigor();

	tree.addNewShoots();
}