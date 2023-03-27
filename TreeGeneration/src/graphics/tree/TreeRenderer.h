#pragma once
#include "generation/Tree.h"
#include "DrawView.h"
#include "opengl/types/Buffer.h"
#include "opengl/types/VertexArray.h"
#include "opengl/types/Shader.h"
#include "opengl/types/Texture.h"
#include "DrawScene.h"
#include "Mesh.h"
namespace tgen::graphics
{

struct TreeMaterial
{
	rc<gl::Texture> colorTexture{}, normalTexture{};
};

class TreeRenderer
{
	struct BudPoint
	{
		vec4 pos;
		vec4 color;
	};
	struct ColoredLine
	{
		vec4 pos1;
		vec4 pos2;
		vec4 color;
	};
public:


	TreeRenderer(rb<gen::Tree> tree);
	virtual ~TreeRenderer() = default;
	virtual void clear();
	DELETE_COPY_CONSTRUCTORS(TreeRenderer);
	void renderVigor(const DrawView& view);
	void renderOptimalDirection(const DrawView& view);
	virtual void updateRenderer();

	uint32 getLeafCount() const
	{
		return leafSSBO.getSize();
	}
	virtual const gl::Buffer<gl::BufferType::SSBO>& getBranchSSBO() const = 0;
	template<typename T>
	const gl::SSBO<T>& getBranchSSBOTyped() const
	{
		return reinterpret_cast<const gl::SSBO<T>&>(getBranchSSBO());
	}
	virtual uint32 getBranchCount() const = 0;
	uint32 getBudCount() const
	{
		return budSSBO.getSize();
	}

	const gl::SSBO<mat4>& getLeafSSBO() const { return leafSSBO; }
	const gl::SSBO<ColoredLine>& getColoredLineSSBO() const { return coloredLineSSBO; }
	const gl::SSBO<BudPoint>& getBudSSBO() const { return budSSBO; }

protected:
	rb<gen::Tree> tree;
	std::unique_ptr<EventSubscriber<gen::Tree::TreeEventData>> onGrowAfterSubscriber{}, OnDestroySubscriber{};
	gl::SSBO<BudPoint> budSSBO{};
	gl::SSBO<ColoredLine> coloredLineSSBO{};
	gl::SSBO<mat4> leafSSBO{};
};
}