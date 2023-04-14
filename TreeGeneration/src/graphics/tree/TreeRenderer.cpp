#include "TreeRenderer.h"
#include "util/Util.h"
#include "generation/TreeWorld.h"
#include "Rendering.h"
#include "Renderer.h"
#include "TreeRendererManager.h"
namespace tgen::graphics
{
using namespace gl;
using namespace gen;
TreeRenderer::TreeRenderer(rb<Tree> tree) : treeId(tree->id), world(tree->world)
{
	onGrowAfterSubscriber = tree->OnAfterGrow.subscribe(this, "Renderer", [this](const Tree::TreeEventData& eventData)
		{
			updateRenderer();
		});
	OnDestroySubscriber = tree->OnDestroy.subscribe(this, "Renderer", [this](const Tree::TreeEventData& eventData)
		{
			clear();
		});
}

void TreeRenderer::clear()
{
	treeId = -1;
	world = nullptr;
	onGrowAfterSubscriber->event = nullptr;
	OnDestroySubscriber->event = nullptr;
}


void TreeRenderer::renderVigor(const DrawView& view)
{
	static GraphicsPipeline pipeline = []() -> GraphicsPipeline
	{
		GraphicsPipeline pipeline("Bud Point", *TreeRendererManager::resources.budPointShader);
		pipeline.rasterizationState.programPointSize = true;
		pipeline.inputAssemblyState.topology = PrimitiveTopology::POINT_LIST;
		return pipeline;
	}();
	Cmd::ScopedGraphicsPipeline _(pipeline);
	Cmd::SetUniform("VP", view.VP);
	Cmd::util::BindMesh(*TreeRendererManager::resources.pointMesh);
	Cmd::BindSSBO(0, budSSBO, 0, budSSBO.getRawSize());

	Cmd::DrawIndexed(1, budSSBO.getSize());
}

void TreeRenderer::renderOptimalDirection(const DrawView& view)
{
	static GraphicsPipeline pipeline = []() -> GraphicsPipeline
	{
		GraphicsPipeline pipeline("Bud Point", *TreeRendererManager::resources.coloredLineShader);
		pipeline.rasterizationState.lineWidth = 2.0f;
		pipeline.inputAssemblyState.topology = PrimitiveTopology::LINE_LIST;
		return pipeline;
	}();
	Cmd::ScopedGraphicsPipeline _(pipeline);
	Cmd::SetUniform("VP", view.VP);
	Cmd::util::BindMesh(*TreeRendererManager::resources.lineMesh);
	Cmd::BindSSBO(0, coloredLineSSBO, 0, coloredLineSSBO.getRawSize());
	Cmd::DrawIndexed(2, coloredLineSSBO.getSize());
}

void TreeRenderer::updateRenderer()
{
	auto* tree = getTree();
	if (tree->age <= 0)
		return;

	tree->accumulateLight();
	tree->root->vigor = tree->root->light;
	tree->distributeVigor();

	const auto& nodes = tree->AsNodeVector(true);
	std::vector<BudPoint> points;
	std::vector<ColoredLine> lines;
	for (auto& node : nodes)
	{
		if (node.nodeStatus == TreeNode::ALIVE)
			continue;
		float vigor = node.vigor;

		vec4 color = vigor >= 1.0f ? vec4(0.0f, 1.0f, 0.0f, vigor) : vec4(1.0f, 0.0f, 0.0f, vigor);
		if (node.nodeStatus == TreeNode::DEAD)
			color = vec4(0.0f, 0.0f, 0.0f, vigor);

		points.emplace_back(vec4(node.startPos, 0.0f), color);

		vec3 dir = 0.1f * tree->world->getOptimalDirection(node.startPos);
		lines.emplace_back(vec4(node.startPos, 0.0f), vec4(node.startPos + dir, 0.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f));
	}
	budSSBO.init(std::span<BudPoint>(points));
	coloredLineSSBO.init(std::span<ColoredLine>(lines));
}
}