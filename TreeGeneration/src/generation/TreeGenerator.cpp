#include "TreeGenerator.h"
#include <algorithm>
namespace tgen::gen
{
Tree& TreeGenerator::createTree(TreeWorld& world, vec2 position, GrowthDataId growthDataId)
{
	return world.createTree(position, growthDataId);
}

std::vector<vec2> TreeGenerator::spreadSeeds(Tree& tree)
{
	return tree.spreadSeeds();
}

void TreeGenerator::growTree(Tree& tree)
{
	if (!tree.getGrowthData().grow || tree.age > 10)
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

bool TreeGenerator::shouldDie(Tree& tree)
{
	if (tree.age < 4)
		return false;
	return tree.root->nodeStatus != TreeNode::ALIVE;
}

void TreeGenerator::iterateWorld(TreeWorld& world, int count, bool updateRenderers)
{
	for (int i = 0; i < count; i++)
	{
		std::vector<std::pair<vec2, GrowthDataId>> seeds;
		std::vector<uint32> toDestroy;
		for (auto& tree : world.getTrees())
		{
			growTree(*tree);
			if (shouldDie(*tree))
			{
				toDestroy.push_back(tree->id);
				continue;
			}
			auto newSeeds = spreadSeeds(*tree);
			seeds.reserve(newSeeds.size());
			for (auto& vec : newSeeds)
				seeds.push_back({ vec, tree->growthDataId });
		}

		for (auto id : toDestroy)
			world.removeTree(id);

		for (auto& [seedLoc, seedGrowthId] : seeds)
			world.createTree(seedLoc, seedGrowthId);
		world.age++;
	}
	world.OnAfterWorldGrow.dispatch(EventData{});
	if (updateRenderers)
	{
		for (auto& tree : world.getTrees())
		{
			if (!tree->getGrowthData().grow)
				return;
			tree->OnAfterGrow.dispatch({});
		}
	}
}
}