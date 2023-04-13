#include "TreeGenerator.h"
namespace tgen::gen
{
Tree& TreeGenerator::createTree(TreeWorld& world, vec3 position, GrowthDataId growthDataId)
{
	return world.createTree(position, growthDataId);
}

std::vector<vec2> TreeGenerator::spreadSeeds(Tree& world)
{
	return std::vector<vec2>();
}

void TreeGenerator::growTree(Tree& tree)
{
	if (!tree.getGrowthData().grow)
		return;
	tree.OnBeforeGrow.dispatch({});
	tree.age++;
	tree.startGrow();
	//tree.world.calculateShadows();

	tree.accumulateLight();
	tree.distributeVigor();


	tree.addNewShoots();
	tree.shedBranchs();

	tree.calculateChildCount();

	tree.endGrow();
}

void TreeGenerator::iterateWorld(TreeWorld& world, int count, bool updateRenderers)
{
	for (int i = 0; i < count; i++)
	{
		for (auto& tree : world.getTrees())
		{
			growTree(*tree);
		}
		world.age++;
	}
	if (updateRenderers)
	{
		for (auto& tree : world.getTrees())
		{
			if (!tree->getGrowthData().grow)
				return;
			tree->OnAfterGrow.dispatch({});
		}
	}
	world.OnAfterWorldGrow.dispatch(EventData{});
}
}