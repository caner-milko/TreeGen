#include "StaticTreeRenderer.h"
#include "Branch.h"
#include "util/Util.h"
#include "generation/TreeWorld.h"
#include "Rendering.h"
#include "Renderer.h"
namespace tgen::graphics
{
using namespace gl;
using namespace gen;
void StaticTreeRenderer::updateRenderer()
{
	if (tree->age <= 0)
		return;
	const std::vector<rb<Branch>>& branchs = tree->getBranchs();
	std::vector<BranchShaderData> branchData;

	branchData.reserve(branchs.size());

	for (auto& branch : branchs)
	{

		uint32 colorSelected = branch->from->order;
		vec3 color = vec3(util::IntNoise2D(colorSelected), util::IntNoise2D(colorSelected, 1), util::IntNoise2D(colorSelected, 2)) * 0.5f + 0.5f;


		branchData.push_back(branch->asShaderData(color));
	}
	branchSSBO.init(std::span<BranchShaderData>(branchData));

	std::vector<mat4> models;

	for (auto& branch : branchs)
	{
		for (auto& leaf : branch->leaves)
		{
			models.emplace_back(leaf.model);
		}
	}
	leafSSBO.init(std::span<mat4>(models));
	TreeRenderer::updateRenderer();
}
}
