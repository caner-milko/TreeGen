#include "PreviewWorld.h"

tgen::gen::PreviewWorld::PreviewWorld(TreeWorld& realWorld) 
	: TreeWorld(realWorld.getWorldInfo()), realWorld(realWorld)
{
	ResetToRealWorld();
}

void tgen::gen::PreviewWorld::ResetToRealWorld()
{
	age = realWorld.age;
	if (info != realWorld.getWorldInfo())
	{
		info = realWorld.getWorldInfo();
	}
	trees.clear();
	shadowGrid = realWorld.getShadowGrid();
	
	for (auto& tree : realWorld.getTrees())
	{
		auto& newTree = trees.emplace_back(std::make_unique<Tree>(*tree));
		newTree->world = this;
	}
}
