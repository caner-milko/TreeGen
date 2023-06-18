#include "TreeGenerator.h"
#include <algorithm>
#include <chrono>
#include <iostream>
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

struct Timer
{
	static std::unordered_map<std::string, float> timesForIteration;
	static std::unordered_map<std::string, float> totTimesForIteration;
	std::string name;
	std::chrono::system_clock::time_point begin;
	Timer(const std::string& name) : name(name), begin(std::chrono::system_clock::now())
	{
	}
	~Timer()
	{
		auto dif = std::chrono::duration<float, std::milli>(std::chrono::system_clock::now() - begin);
		timesForIteration[name] += dif.count();
	}
	static void reset()
	{
		std::string names[] = { "Accumulate","Distribute", "New shoots", "Shed", "Branch Radius", "End" };
		for (auto& name : names)
		{
			;// std::cout << name << " time: " << timesForIteration[name] << std::endl;
			totTimesForIteration[name] += timesForIteration[name];
		}
		for (auto& [key, val] : timesForIteration)
			if (key.starts_with("Iteration") || key == "Total")
			{
				std::cout << key << " time: " << val << std::endl;
				if (key == "Total")
					for (auto& name : names)
						;//std::cout << "Total " << name << " time: " << totTimesForIteration[name] << std::endl;
			}
		timesForIteration = {};
	}
};

decltype(Timer::timesForIteration) Timer::timesForIteration = {};
decltype(Timer::timesForIteration) Timer::totTimesForIteration = {};

void TreeGenerator::growTree(Tree& tree)
{
	if (!tree.getGrowthData().grow || tree.age > 20)
		return;
	/*if ((2 - tree.id) * tree.age * 2 > 12)
		return;*/
	tree.OnBeforeGrow.dispatch({});
	tree.age++;
	tree.startGrow();
	//tree.world.calculateShadows();
	{
		Timer _("Accumulate");
		tree.accumulateLight();
	}
	{
		Timer _("Distribute");
		tree.distributeVigor();
	}
	{
		Timer _("New shoots");
		tree.addNewShoots();
	}
	{
		Timer _("Shed");
		tree.shedBranchs();
	}
	{
		Timer _("Branch Radius");
		tree.calculateChildCount();
	}
	{
		Timer _("End");
		tree.endGrow();
	}
}

bool TreeGenerator::shouldDie(Tree& tree)
{
	return tree.root->nodeStatus == TreeNode::DEAD || (tree.root->nodeStatus == TreeNode::BUD && tree.age >= 4);
}

void TreeGenerator::iterateWorld(TreeWorld& world, int count, bool updateRenderers)
{
	std::cout << "\n\n" << std::endl;
	{
		Timer timer("Total");
		for (int i = 0; i < count; i++)
		{
			{
				Timer tm("Iteration" + std::to_string(i));
				std::vector<std::pair<vec2, GrowthDataId>> seeds;
				std::vector<uint32> toDestroy;
				int branchCount = 0;
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
					branchCount += tree->root->childCount;
				}

				for (auto id : toDestroy)
					world.removeTree(id);

				for (auto& [seedLoc, seedGrowthId] : seeds)
					world.createTree(seedLoc, seedGrowthId);
				world.age++;
				std::cout << "Branch count: " << branchCount << std::endl;
			}
			Timer::reset();
			std::cout << "\n\n" << std::endl;
		}
		std::chrono::system_clock::time_point begin = std::chrono::system_clock::now();

		world.OnAfterWorldGrow.dispatch(EventData{});
		if (updateRenderers)
		{
			for (auto& tree : world.getTrees())
			{
				if (!tree->getGrowthData().grow)
					continue;
				tree->OnAfterGrow.dispatch({});
			}
		}
		auto dif = std::chrono::duration<float, std::milli>(std::chrono::system_clock::now() - begin);
		std::cout << "Uploading time: " << dif << std::endl;
	}
	Timer::reset();
}
}