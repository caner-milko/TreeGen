#pragma once
#include "TreeRenderer.h"
namespace tgen::graphics
{
class StaticTreeRenderer : public TreeRenderer
{
public:
	StaticTreeRenderer(rb<gen::Tree> tree) : TreeRenderer(tree) {};
	~StaticTreeRenderer() override = default;
	void updateRenderer() override;
	const gl::Buffer<gl::BufferType::SSBO>& getBranchSSBO() const override { return branchSSBO; }
	uint32 getBranchCount() const override { return branchSSBO.getSize(); }
private:
	gl::SSBO<gen::BranchShaderData> branchSSBO{};
};
}