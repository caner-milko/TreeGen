#include "AnimatedTreeRenderer.h"
#include "util/Util.h"
#include "generation/TreeWorld.h"
#include "Rendering.h"
#include "Renderer.h"
namespace tgen::graphics
{
using namespace gl;
using namespace gen;
void AnimatedTreeRenderer::recordOldBranchs()
{
	lastRecorded.clear();
	const std::vector<Branch>& branchs = tree->getBranchs();
	for (auto& branch : branchs)
	{
		lastRecorded.try_emplace(branch.from->id, Branch(branch, false));
	}
}

void AnimatedTreeRenderer::updateRenderer()
{
	if (tree->age <= 0)
		return;
	const std::vector<Branch>& branchs = tree->getBranchs();
	std::vector<AnimatedBranchShaderData> branchData;

	branchData.reserve(branchs.size());

	for (auto& branch : branchs)
	{

		uint32 colorSelected = branch.from->order;
		vec3 color = vec3(util::IntNoise2D(colorSelected), util::IntNoise2D(colorSelected, 1), util::IntNoise2D(colorSelected, 2)) * 0.5f + 0.5f;
		if (auto it = lastRecorded.find(branch.from->id); it != lastRecorded.end())
			branchData.push_back(AnimatedBranch(vec2(0.0, 0.0), branch, it->second).asShaderData(color));
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

			branchData.push_back(AnimatedBranch(bounds, branch).asShaderData(color));

		}
	}
	branchSSBO.init(branchData);

	std::vector<mat4> models;

	for (auto& branch : branchs)
	{
		for (auto& leaf : branch.leaves)
		{
			models.emplace_back(leaf.model);
		}
	}
	leafSSBO.init(std::span<mat4>(models));
	TreeRenderer::updateRenderer();
}
}