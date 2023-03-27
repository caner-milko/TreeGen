#include "TreeRendererManager.h"
#include "util/Util.h"
#include "generation/TreeWorld.h"
#include "Rendering.h"
#include "Renderer.h"
namespace tgen::graphics
{
using namespace gl;
using namespace gen;

TreeRendererManager::TreeRendererResources TreeRendererManager::resources = {};
AnimatedTreeRendererManager::AnimatedTreeRendererResources AnimatedTreeRendererManager::resources = {};
StaticTreeRendererManager::StaticTreeRendererResources StaticTreeRendererManager::resources = {};

void TreeRendererManager::renderTrees(RendererSpan renderers,
	const DrawView& view, const DrawScene& scene, bool branchs, bool leaves) const
{
	if (branchs)
	{
		renderBranchs(renderers, view, scene);
	}
	if (leaves)
	{
		renderLeaves(renderers, view, scene);
	}
}

void StaticTreeRendererManager::renderBranchs(RendererSpan renderers,
	const DrawView& view, const DrawScene& scene) const
{
	static GraphicsPipeline BranchRenderPipeline = []() -> GraphicsPipeline
	{
		GraphicsPipeline pipeline("Branchs", *resources.branchShader);
		pipeline.vertexInputState = TreeRendererManager::resources.cubeMesh->inputState;
		return pipeline;
	}();
	Cmd::ScopedGraphicsPipeline _(BranchRenderPipeline);
	Cmd::util::BindMesh(*TreeRendererManager::resources.cubeMesh);


	glBindTextureUnit(Cmd::GetShader().getTextureIndex("treeMaterial.colorTexture"), TreeRendererManager::resources.material.colorTexture->getHandle());
	glBindTextureUnit(Cmd::GetShader().getTextureIndex("treeMaterial.normalTexture"), TreeRendererManager::resources.material.normalTexture->getHandle());
	glBindTextureUnit(Cmd::GetShader().getTextureIndex("shadowMap"), scene.light.shadowMap->getHandle());
	vec3 camPos = view.camera.cameraPosition;
	vec3 camDir = view.camera.getCameraDirection();

	Cmd::SetUniform("treeColor", vec3(166.0f / 255.0f, 123.0f / 255.0f, 81.0f / 255.0f));
	Cmd::BindUBO(0, *TreeRendererManager::resources.camUBO, 0, TreeRendererManager::resources.camUBO->getRawSize());
	Cmd::BindUBO(1, *TreeRendererManager::resources.lightUBO, 0, TreeRendererManager::resources.lightUBO->getRawSize());
	for (auto renderer : renderers)
	{
		if (renderer->getBranchCount() == 0)
			continue;
		Cmd::BindSSBO(0, renderer->getBranchSSBO(), 0, renderer->getBranchSSBO().getRawSize());
		Cmd::Draw(TreeRendererManager::resources.cubeMesh->vbo.getSize(), renderer->getBranchCount(), 0, 0);
	}
}

void StaticTreeRendererManager::renderLeaves(RendererSpan renderers,
	const DrawView& view, const DrawScene& scene) const
{
	const GraphicsPipeline pipeline = []() ->GraphicsPipeline
	{
		GraphicsPipeline pipeline("Leaves", *resources.leafShader);
		pipeline.rasterizationState.cullMode = CullMode::NONE;
		pipeline.vertexInputState = TreeRendererManager::resources.leafMesh->inputState;
		return pipeline;
	}();
	Cmd::ScopedGraphicsPipeline _(pipeline);
	Cmd::util::BindMesh(*TreeRendererManager::resources.leafMesh);

	Cmd::BindUBO(0, *TreeRendererManager::resources.camUBO, 0, TreeRendererManager::resources.camUBO->getRawSize());
	Cmd::BindUBO(1, *TreeRendererManager::resources.lightUBO, 0, TreeRendererManager::resources.lightUBO->getRawSize());

	glBindTextureUnit(resources.leafShader->getTextureIndex("leafTex"), TreeRendererManager::resources.leafTexture->getHandle());
	for (auto& renderer : renderers)
	{
		if (renderer->getLeafSSBO().getRawSize() == 0)
			continue;
		Cmd::BindSSBO(0, renderer->getLeafSSBO(), 0, renderer->getLeafSSBO().getRawSize());
		Cmd::DrawIndexed(TreeRendererManager::resources.leafMesh->ebo.getSize(), renderer->getLeafSSBO().getSize());
	}

}

void TreeRendererManager::renderTreeShadows(RendererSpan renderers, const DrawView& view,
	bool renderBranches, bool renderLeaves) const
{
	if (renderBranches)
		renderBranchShadows(renderers, view);
	if (renderLeaves)
		renderLeafShadows(renderers, view);
}

void StaticTreeRendererManager::renderBranchShadows(RendererSpan renderers, const DrawView& view) const
{
	static GraphicsPipeline BranchRenderPipeline = []() -> GraphicsPipeline
	{
		GraphicsPipeline pipeline("Branch Shadows", *resources.branchShadowShader);
		pipeline.vertexInputState = TreeRendererManager::resources.cubeMesh->inputState;
		pipeline.rasterizationState.cullMode = CullMode::FRONT;
		return pipeline;
	}();
	Cmd::ScopedGraphicsPipeline _(BranchRenderPipeline);
	Cmd::util::BindMesh(*TreeRendererManager::resources.cubeMesh);

	Cmd::BindUBO(0, *TreeRendererManager::resources.camUBO, 0, TreeRendererManager::resources.camUBO->getRawSize());
	Cmd::BindUBO(1, *TreeRendererManager::resources.lightUBO, 0, TreeRendererManager::resources.lightUBO->getRawSize());

	for (auto renderer : renderers)
	{
		if (renderer->getBranchSSBO().getRawSize() == 0)
			continue;
		Cmd::BindSSBO(0, renderer->getBranchSSBO(), 0, renderer->getBranchSSBO().getRawSize());
		Cmd::Draw(TreeRendererManager::resources.cubeMesh->vbo.getSize(), renderer->getBranchCount());
	}
}

void StaticTreeRendererManager::renderLeafShadows(RendererSpan renderers, const DrawView& view) const
{
	const GraphicsPipeline pipeline = []() ->GraphicsPipeline
	{
		GraphicsPipeline pipeline("Leaf Shadows", *resources.leavesShadowShader);
		pipeline.rasterizationState.cullMode = CullMode::NONE;
		pipeline.vertexInputState = TreeRendererManager::resources.leafMesh->inputState;
		return pipeline;
	}();
	Cmd::ScopedGraphicsPipeline _(pipeline);
	Cmd::util::BindMesh(*TreeRendererManager::resources.leafShadowMesh);

	Cmd::BindUBO(0, *TreeRendererManager::resources.camUBO, 0, TreeRendererManager::resources.camUBO->getRawSize());
	Cmd::BindUBO(1, *TreeRendererManager::resources.lightUBO, 0, TreeRendererManager::resources.lightUBO->getRawSize());

	glBindTextureUnit(resources.leafShader->getTextureIndex("leafTex"), TreeRendererManager::resources.leafTexture->getHandle());

	for (auto renderer : renderers)
	{
		if (renderer->getLeafSSBO().getRawSize() == 0)
			continue;
		Cmd::BindSSBO(0, renderer->getLeafSSBO(), 0, renderer->getLeafSSBO().getRawSize());
		Cmd::DrawIndexed(TreeRendererManager::resources.leafMesh->ebo.getSize(), renderer->getLeafSSBO().getSize());
	}
}

AnimatedTreeRendererManager::AnimatedTreeRendererManager(TreeWorld& world)
{
	onGrowAfterSubscriber = world.OnAfterWorldGrow.subscribe(this, "AnimatedTreeRendererManager",
		[this](const EventData& eventData)
		{
			lastGrowth = std::chrono::steady_clock::now();
		});
}

void AnimatedTreeRendererManager::renderBranchShadows(RendererSpan renderers, const DrawView& view) const
{
}
void AnimatedTreeRendererManager::renderLeafShadows(RendererSpan renderers, const DrawView& view) const
{
}
void AnimatedTreeRendererManager::renderBranchs(RendererSpan renderers, const DrawView& view, const DrawScene& scene) const
{
	static GraphicsPipeline BranchRenderPipeline = []() -> GraphicsPipeline
	{
		GraphicsPipeline pipeline("Branchs", *resources.animatedBranchShader);
		pipeline.vertexInputState = TreeRendererManager::resources.cubeMesh->inputState;
		return pipeline;
	}();
	Cmd::ScopedGraphicsPipeline _(BranchRenderPipeline);
	Cmd::util::BindMesh(*TreeRendererManager::resources.cubeMesh);


	glBindTextureUnit(Cmd::GetShader().getTextureIndex("treeMaterial.colorTexture"), TreeRendererManager::resources.material.colorTexture->getHandle());
	glBindTextureUnit(Cmd::GetShader().getTextureIndex("treeMaterial.normalTexture"), TreeRendererManager::resources.material.normalTexture->getHandle());
	glBindTextureUnit(Cmd::GetShader().getTextureIndex("shadowMap"), scene.light.shadowMap->getHandle());
	vec3 camPos = view.camera.cameraPosition;
	vec3 camDir = view.camera.getCameraDirection();

	std::chrono::duration<float> diff = std::chrono::steady_clock::now() - lastGrowth;
	float timeSinceLastGrowth = glm::clamp(diff.count() * resources.animationSpeed, 0.0f, 1.0f);
	Cmd::SetUniform("animationT", timeSinceLastGrowth);

	Cmd::SetUniform("treeColor", vec3(166.0f / 255.0f, 123.0f / 255.0f, 81.0f / 255.0f));
	Cmd::BindUBO(0, *TreeRendererManager::resources.camUBO, 0, TreeRendererManager::resources.camUBO->getRawSize());
	Cmd::BindUBO(1, *TreeRendererManager::resources.lightUBO, 0, TreeRendererManager::resources.lightUBO->getRawSize());
	for (auto renderer : renderers)
	{
		if (renderer->getBranchCount() == 0)
			continue;
		Cmd::BindSSBO(0, renderer->getBranchSSBO(), 0, renderer->getBranchSSBO().getRawSize());
		Cmd::Draw(TreeRendererManager::resources.cubeMesh->vbo.getSize(), renderer->getBranchCount(), 0, 0);
	}
}
void AnimatedTreeRendererManager::renderLeaves(RendererSpan renderers, const DrawView& view, const DrawScene& scene) const
{
}
}