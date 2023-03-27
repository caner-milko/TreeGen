#pragma once
#include "TreeRenderer.h"
#include "StaticTreeRenderer.h"
#include "AnimatedTreeRenderer.h"
#include "TreeWorld.h"
#include <chrono>
namespace tgen::graphics
{
class TreeRendererManager
{
public:
	using RendererSpan = std::span<rb<TreeRenderer>>;
	static struct TreeRendererResources
	{
		rc<CompleteMesh<Vertex, gl::IndexType::UNSIGNED_INT>> leafShadowMesh = nullptr;
		rc<CompleteMesh<Vertex, gl::IndexType::UNSIGNED_INT>> leafMesh = nullptr;
		rb<const ArrayMesh<vec3>> cubeMesh = nullptr;
		rb<const IndexedMesh<gl::IndexType::UNSIGNED_INT>> pointMesh = nullptr,
			lineMesh = nullptr;

		rc<gl::Shader> budPointShader = nullptr,
			coloredLineShader = nullptr;

		rc<gl::Texture> leafTexture = nullptr;
		TreeMaterial material{};

		rb<const gl::UBO<CameraUniform>> camUBO = nullptr;
		rb<const gl::UBO<DirLightUniform>> lightUBO = nullptr;
	} resources;
	virtual void renderBranchShadows(RendererSpan renderers, const DrawView& view) const = 0;
	virtual void renderLeafShadows(RendererSpan renderers, const DrawView& view) const = 0;
	virtual void renderTreeShadows(RendererSpan renderers, const DrawView& view,
		bool renderBranches, bool renderLeaves) const;

	virtual void renderBranchs(RendererSpan renderers, const DrawView& view, const DrawScene& scene) const = 0;
	virtual void renderLeaves(RendererSpan renderers, const DrawView& view, const DrawScene& scene) const = 0;
	virtual void renderTrees(RendererSpan renderers, const DrawView& view, const DrawScene& scene,
		bool renderBranchs, bool renderLeaves) const;

	bool debug = false;
};
class StaticTreeRendererManager : public TreeRendererManager
{
public:
	static struct StaticTreeRendererResources
	{
		rc<gl::Shader> branchShader = nullptr,
			leafShader = nullptr,
			branchShadowShader = nullptr,
			leavesShadowShader = nullptr;
	} resources;

	virtual void renderBranchShadows(RendererSpan renderers, const DrawView& view) const override;
	virtual void renderLeafShadows(RendererSpan renderers, const DrawView& view) const override;

	virtual void renderBranchs(RendererSpan renderers, const DrawView& view, const DrawScene& scene) const override;
	virtual void renderLeaves(RendererSpan renderers, const DrawView& view, const DrawScene& scene) const override;
};

class AnimatedTreeRendererManager : public TreeRendererManager
{
public:
	static struct AnimatedTreeRendererResources
	{
		rc<gl::Shader> animatedBranchShader = nullptr,
			animatedLeafShader = nullptr;
		float animationSpeed = 0.2f;
	} resources;

	AnimatedTreeRendererManager(gen::TreeWorld& world);

	virtual void renderBranchShadows(RendererSpan renderers, const DrawView& view) const override;
	virtual void renderLeafShadows(RendererSpan renderers, const DrawView& view) const override;

	virtual void renderBranchs(RendererSpan renderers, const DrawView& view, const DrawScene& scene) const override;
	virtual void renderLeaves(RendererSpan renderers, const DrawView& view, const DrawScene& scene) const override;

	std::chrono::steady_clock::time_point lastGrowth = std::chrono::steady_clock::now();
	std::unique_ptr<EventSubscriber<EventData>> onGrowAfterSubscriber{};
};
}