#include "AnimatedTreeRenderer.h"
#include "util/Util.h"
#include "generation/TreeWorld.h"
#include "Rendering.h"
#include "Renderer.h"
namespace tgen::graphics
{
using namespace gl;
using namespace gen;
void AnimatedTreeRenderer::CreateLeafSSBO(float animationT)
{
	auto* tree = getTree();
	std::vector<mat4> models;
	int i = 0;
	for (auto& branch : branchs)
	{
		auto& animatedBranch = animatedBranchs[i];
		for (auto& leaf : branch->leaves)
		{
			models.emplace_back(leaf.animatedLeaf(&animatedBranch, tree->age, animationT));
		}
		i++;
	}
	leafSSBO.init(std::span<mat4>(models));
}
void AnimatedTreeRenderer::recordOldBranchs()
{
	auto* tree = getTree();
	lastRecorded.clear();
	const auto& branchs = tree->getBranchs();
	for (auto& branch : branchs)
	{
		lastRecorded.try_emplace(branch->from->id, Branch(*branch, false));
	}
}

void AnimatedTreeRenderer::updateRenderer()
{
	auto* tree = getTree();
	if (tree->age <= 0)
		return;
	branchs = tree->getBranchs();
	std::vector<AnimatedBranchShaderData> branchData;

	animatedBranchs.clear();
	animatedBranchs.reserve(branchs.size());
	branchData.reserve(branchs.size());

	for (auto branchPtr : branchs)
	{
		auto& branch = *branchPtr;
		uint32 colorSelected = branch.from->order;
		vec3 color = vec3(util::IntNoise2D(colorSelected), util::IntNoise2D(colorSelected, 1), util::IntNoise2D(colorSelected, 2)) * 0.5f + 0.5f;
		if (auto it = lastRecorded.find(branch.from->id); it != lastRecorded.end())
		{
			auto& br = animatedBranchs.emplace_back(vec2(0.0, 0.0), branch, it->second);
			branchData.push_back(br.asShaderData(color));
		}
		else
		{
			uint32 newBranchCount = 0;
			TreeNode* curNode = branch.from->mainChild;
			while (curNode)
			{
				newBranchCount++;
				curNode = curNode->mainChild;
			}
			uint32 newOrder = 0;
			uint32 age = branch.from->createdAt;
			curNode = branch.from->parent;
			while (curNode)
			{
				if (curNode->createdAt != age)
					break;
				curNode = curNode->parent;
				newOrder++;
				newBranchCount++;
			}

			vec2 bounds = vec2(float(newOrder) / newBranchCount, float(newOrder + 1) / newBranchCount);


			auto& br = animatedBranchs.emplace_back(bounds, branch);
			branchData.push_back(br.asShaderData(color));

		}
	}
	branchSSBO.init(branchData);

	TreeRenderer::updateRenderer();
}
}