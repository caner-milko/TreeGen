#include "TreeGenerator.h"
namespace tgen::gen
{
Tree& TreeGenerator::createTree(TreeWorld& world, vec3 position, TreeGrowthData growthData)
{
	return world.createTree(position, growthData);
}

void TreeGenerator::growTree(Tree& tree)
{
	if (!tree.growthData.grow)
		return;
	tree.OnBeforeGrow.dispatch({});
	tree.age++;
	tree.startGrow();
	//tree.world.calculateShadows();

	tree.accumulateLight();
	tree.distributeVigor();


	tree.addNewShoots();
	if (tree.growthData.shouldShed)
	{
		tree.shedBranchs();
	}

	tree.calculateChildCount();

	tree.endGrow();
	tree.OnAfterGrow.dispatch({});
}
void TreeGenerator::iterateWorld(TreeWorld& world, int count)
{
	for (int i = 0; i < count; i++)
	{
		for (auto& tree : world.getTrees())
		{
			growTree(*tree);
		}
		world.age++;
	}
	world.OnAfterWorldGrow.dispatch(EventData{});
}
}