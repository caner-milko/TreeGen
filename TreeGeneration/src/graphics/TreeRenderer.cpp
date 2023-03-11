#include "TreeRenderer.h"
#include "util/Util.h"
#include "generation/TreeWorld.h"
#include "Rendering.h"
#include "Renderer.h"
namespace tgen::graphics {
using namespace gl;
using namespace gen;
TreeRenderer::TreeRendererResources TreeRenderer::resources = {};
TreeRenderer::TreeRenderer(rb<Tree> tree) : tree(tree)
{
	onGrowSubsriber = tree->OnGrow.subscribe(this, "Renderer", [this](const Tree::TreeEventData& eventData)
		{
			updateRenderer();
		});
	OnDestroySubscriber = tree->OnDestroy.subscribe(this, "Renderer", [this](const Tree::TreeEventData& eventData)
		{
			this->clear();
		});
}

void TreeRenderer::clear()
{
	tree = nullptr;
	onGrowSubsriber->event = nullptr;
	OnDestroySubscriber->event = nullptr;
}

void TreeRenderer::renderTrees(std::span<rb<TreeRenderer>> renderers,
	const DrawView& view, const DrawScene& scene, bool branchs, bool leaves)
{
	if (branchs && resources.branchShader) {
		renderBranchs(renderers, view, scene);
	}
	if (leaves && resources.leafShader) {
		renderLeaves(renderers, view, scene);
	}
}

void TreeRenderer::renderBranchs(std::span<rb<TreeRenderer>> renderers,
	const DrawView& view, const DrawScene& scene)
{
	static GraphicsPipeline BranchRenderPipeline = []() -> GraphicsPipeline {
		GraphicsPipeline pipeline("Branchs", *resources.branchShader);
		pipeline.vertexInputState = resources.cubeMesh->inputState;
		return pipeline;
	}();
	Cmd::ScopedGraphicsPipeline _(BranchRenderPipeline);
	Cmd::util::BindMesh(*resources.cubeMesh);

	glBindTextureUnit(Cmd::GetShader().getTextureIndex("treeMaterial.colorTexture"), resources.material.colorTexture->getHandle());
	glBindTextureUnit(Cmd::GetShader().getTextureIndex("treeMaterial.normalTexture"), resources.material.normalTexture->getHandle());
	glBindTextureUnit(Cmd::GetShader().getTextureIndex("shadowMap"), scene.light.shadowMap->getHandle());
	vec3 camPos = view.camera.cameraPosition;
	vec3 camDir = view.camera.getCameraDirection();

	Cmd::SetUniform("treeColor", vec3(166.0f / 255.0f, 123.0f / 255.0f, 81.0f / 255.0f));
	Cmd::BindUBO(0, *resources.camUBO, 0, resources.camUBO->getRawSize());
	Cmd::BindUBO(1, *resources.lightUBO, 0, resources.lightUBO->getRawSize());
	for (auto renderer : renderers) {
		if (renderer->branchSSBO.getRawSize() == 0)
			continue;
		Cmd::BindSSBO(0, renderer->branchSSBO, 0, renderer->branchSSBO.getRawSize());
		Cmd::Draw(resources.cubeMesh->vbo.getSize(), renderer->branchSSBO.getSize(), 0, 0);
	}
}

void TreeRenderer::renderLeaves(std::span<rb<TreeRenderer>> renderers,
	const DrawView& view, const DrawScene& scene)
{
	const GraphicsPipeline pipeline = []() ->GraphicsPipeline {
		GraphicsPipeline pipeline("Leaves", *TreeRenderer::resources.leafShader);
		pipeline.rasterizationState.cullMode = CullMode::NONE;
		pipeline.vertexInputState = TreeRenderer::resources.leafMesh->inputState;
		return pipeline;
	}();
	Cmd::ScopedGraphicsPipeline _(pipeline);
	Cmd::util::BindMesh(*resources.leafMesh);

	Cmd::BindUBO(0, *resources.camUBO, 0, resources.camUBO->getRawSize());
	Cmd::BindUBO(1, *resources.lightUBO, 0, resources.lightUBO->getRawSize());

	glBindTextureUnit(resources.leafShader->getTextureIndex("leafTex"), resources.leafTexture->getHandle());
	for (auto& renderer : renderers) {
		if (renderer->leafSSBO.getRawSize() == 0)
			continue;
		Cmd::BindSSBO(0, renderer->leafSSBO, 0, renderer->leafSSBO.getRawSize());
		Cmd::Draw(resources.leafMesh->vbo.getSize(), renderer->leafSSBO.getSize());
	}

}

void TreeRenderer::renderTreeShadows(std::span<rb<TreeRenderer>> renderers, const DrawView& view, bool renderBranches, bool renderLeaves) {
	if (renderBranches)
		renderBranchShadows(renderers, view);
	if (renderLeaves)
		renderLeafShadows(renderers, view);
}

void TreeRenderer::renderBranchShadows(std::span<rb<TreeRenderer>> renderers, const DrawView& view)
{
	static GraphicsPipeline BranchRenderPipeline = []() -> GraphicsPipeline {
		GraphicsPipeline pipeline("Branch Shadows", *resources.branchShadowShader);
		pipeline.vertexInputState = resources.cubeMesh->inputState;
		pipeline.rasterizationState.cullMode = CullMode::FRONT;
		return pipeline;
	}();
	Cmd::ScopedGraphicsPipeline _(BranchRenderPipeline);
	Cmd::util::BindMesh(*resources.cubeMesh);

	Cmd::BindUBO(0, *resources.camUBO, 0, resources.camUBO->getRawSize());
	Cmd::BindUBO(1, *resources.lightUBO, 0, resources.lightUBO->getRawSize());

	for (auto renderer : renderers) {
		if (renderer->branchSSBO.getRawSize() == 0)
			continue;
		Cmd::BindSSBO(0, renderer->branchSSBO, 0, renderer->branchSSBO.getRawSize());
		Cmd::Draw(resources.cubeMesh->vbo.getSize(), renderer->branchSSBO.getSize());
	}
}

void TreeRenderer::renderLeafShadows(std::span<rb<TreeRenderer>> renderers, const DrawView& view)
{
	const GraphicsPipeline pipeline = []() ->GraphicsPipeline {
		GraphicsPipeline pipeline("Leaf Shadows", *TreeRenderer::resources.leavesShadowShader);
		pipeline.rasterizationState.cullMode = CullMode::NONE;
		pipeline.vertexInputState = TreeRenderer::resources.leafMesh->inputState;
		return pipeline;
	}();
	Cmd::ScopedGraphicsPipeline _(pipeline);
	Cmd::util::BindMesh(*resources.leafMesh);

	Cmd::BindUBO(0, *resources.camUBO, 0, resources.camUBO->getRawSize());
	Cmd::BindUBO(1, *resources.lightUBO, 0, resources.lightUBO->getRawSize());

	glBindTextureUnit(resources.leafShader->getTextureIndex("leafTex"), resources.leafTexture->getHandle());

	for (auto renderer : renderers) {
		if (renderer->leafSSBO.getRawSize() == 0)
			continue;
		Cmd::BindSSBO(0, renderer->leafSSBO, 0, renderer->leafSSBO.getRawSize());
		Cmd::Draw(resources.leafMesh->vbo.getSize(), renderer->leafSSBO.getSize());
	}
}

void TreeRenderer::renderVigor(const DrawView& view)
{
	static GraphicsPipeline pipeline = []() -> GraphicsPipeline {
		GraphicsPipeline pipeline("Bud Point", *resources.budPointShader);
		pipeline.rasterizationState.programPointSize = true;
		pipeline.inputAssemblyState.topology = PrimitiveTopology::POINT_LIST;
		return pipeline;
	}();
	Cmd::ScopedGraphicsPipeline _(pipeline);
	Cmd::SetUniform("VP", view.VP);
	Cmd::util::BindMesh(*resources.pointMesh);
	Cmd::BindSSBO(0, budSSBO, 0, budSSBO.getRawSize());

	Cmd::DrawIndexed(1, budSSBO.getSize());
}

void TreeRenderer::renderOptimalDirection(const DrawView& view) {
	static GraphicsPipeline pipeline = []() -> GraphicsPipeline {
		GraphicsPipeline pipeline("Bud Point", *resources.coloredLineShader);
		pipeline.rasterizationState.lineWidth = 2.0f;
		pipeline.inputAssemblyState.topology = PrimitiveTopology::LINE_LIST;
		return pipeline;
	}();
	Cmd::ScopedGraphicsPipeline _(pipeline);
	Cmd::SetUniform("VP", view.VP);
	Cmd::util::BindMesh(*resources.lineMesh);
	Cmd::BindSSBO(0, coloredLineSSBO, 0, coloredLineSSBO.getRawSize());
	Cmd::DrawIndexed(2, coloredLineSSBO.getSize());
}

void TreeRenderer::updateRenderer()
{
	if (tree->age <= 0)
		return;
	const std::vector<Branch>& branchs = tree->getBranchs();
	std::vector<BranchShaderData> branchData;

	branchData.reserve(branchs.size());

	for (auto& branch : branchs) {

		uint32 colorSelected = branch.from.order;
		vec3 color = vec3(util::IntNoise2D(colorSelected), util::IntNoise2D(colorSelected, 1), util::IntNoise2D(colorSelected, 2)) * 0.5f + 0.5f;


		branchData.push_back(branch.asShaderData(color));
	}
	branchSSBO.init(std::span<BranchShaderData>(branchData));

	std::vector<mat4> models;

	for (auto& branch : branchs) {
		for (auto& leaf : branch.leaves) {
			models.emplace_back(leaf.model);
		}
	}
	leafSSBO.init(std::span<mat4>(models));

	//tree.world.calculateShadows();

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

uint32 TreeRenderer::getLeafCount() const {
	return leafSSBO.getSize();
}

uint32 TreeRenderer::getBranchCount() const {
	return branchSSBO.getSize();
}

uint32 TreeRenderer::getBudCount() const {
	return budSSBO.getSize();
}

}