#include "PreviewWorld.h"

tgen::gen::PreviewWorld::PreviewWorld(TreeWorld& realWorld)
	: TreeWorld(realWorld.getWorldInfo(), realWorld.getWorldGrowthData()), realWorld(realWorld)
{
	ResetToRealWorld();
}

void tgen::gen::PreviewWorld::ResetToRealWorld()
{
	terrain = realWorld.terrain;
	age = realWorld.age;
	if (info != realWorld.getWorldInfo())
	{
		info = realWorld.getWorldInfo();
	}
	SetWorldGrowthData(realWorld.getWorldGrowthData());
	trees.clear();
	shadowGrid = realWorld.getShadowGrid();

	for (auto& tree : realWorld.getTrees())
	{
		auto& newTree = trees.emplace_back(std::make_unique<Tree>(*tree));
		newTree->world = this;
	}
}
