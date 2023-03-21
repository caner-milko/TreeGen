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
	bool debug = false;
	static struct TreeRendererResources
	{
		rc<CompleteMesh<Vertex, gl::IndexType::UNSIGNED_INT>> leafShadowMesh = nullptr;
		rc<CompleteMesh<Vertex, gl::IndexType::UNSIGNED_INT>> leafMesh = nullptr;
		rb<const ArrayMesh<vec3>> cubeMesh = nullptr;
		rb<const IndexedMesh<gl::IndexType::UNSIGNED_INT>> pointMesh = nullptr,
			lineMesh = nullptr;

		rc<gl::Shader> branchShader = nullptr,
			leafShader = nullptr,
			budPointShader = nullptr,
			coloredLineShader = nullptr,
			branchShadowShader = nullptr,
			leavesShadowShader = nullptr;

		rc<gl::Texture> leafTexture = nullptr;
		TreeMaterial material{};

		rb<const gl::UBO<CameraUniform>> camUBO = nullptr;
		rb<const gl::UBO<DirLightUniform>> lightUBO = nullptr;
	} resources;

	TreeRenderer(rb<gen::Tree> tree);
	void clear();
	DELETE_COPY_CONSTRUCTORS(TreeRenderer)
		void renderVigor(const DrawView& view);
	void renderOptimalDirection(const DrawView& view);
	void updateRenderer();
	uint32 getLeafCount() const;
	uint32 getBranchCount() const;
	uint32 getBudCount() const;


	static void renderBranchShadows(std::span<rb<TreeRenderer>> renderers, const DrawView& view);
	static void renderLeafShadows(std::span<rb<TreeRenderer>> renderers, const DrawView& view);
	static void renderTreeShadows(std::span<rb<TreeRenderer>> renderers, const DrawView& view, bool renderBranches, bool renderLeaves);

	static void renderBranchs(std::span<rb<TreeRenderer>> renderers, const DrawView& view, const DrawScene& scene);
	static void renderLeaves(std::span<rb<TreeRenderer>> renderers, const DrawView& view, const DrawScene& scene);
	static void renderTrees(std::span<rb<TreeRenderer>> renderers, const DrawView& view, const DrawScene& scene, bool renderBranchs, bool renderLeaves);
private:
	rb<gen::Tree> tree;
	std::unique_ptr<EventSubscriber<gen::Tree::TreeEventData>> onGrowSubsriber{}, OnDestroySubscriber{};
	gl::SSBO<gen::BranchShaderData> branchSSBO{};
	gl::SSBO<mat4> leafSSBO{};
	gl::SSBO<BudPoint> budSSBO{};
	gl::SSBO<ColoredLine> coloredLineSSBO{};
};
}