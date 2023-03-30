#pragma once
#include "TreeRenderer.h"
#include "AnimatedBranch.h"
namespace tgen::graphics
{
class AnimatedTreeRenderer : public TreeRenderer
{
public:
	AnimatedTreeRenderer(rb<gen::Tree> tree) : TreeRenderer(tree)
	{
		onGrowBeforeSubscriber = tree->OnBeforeGrow.subscribe(this, "Renderer", [this](const gen::Tree::TreeEventData& eventData)
			{
				recordOldBranchs();
			});
	};
	void CreateLeafSSBO(float animationT);
	void recordOldBranchs();
	~AnimatedTreeRenderer() override = default;
	void updateRenderer() override;
	const gl::Buffer<gl::BufferType::SSBO>& getBranchSSBO() const override { return branchSSBO; }
	uint32 getBranchCount() const override { return branchSSBO.getSize(); }

private:
	std::vector<gen::Branch> branchs;
	std::vector<gen::AnimatedBranch> animatedBranchs;
	std::unordered_map<uint32, gen::Branch> lastRecorded;
	std::unique_ptr<EventSubscriber<gen::Tree::TreeEventData>> onGrowBeforeSubscriber{};
	gl::SSBO<gen::AnimatedBranchShaderData> branchSSBO{};
};

}