#include "TreeRenderer.h"
#include "../Util.hpp"
#include "../TreeWorld.h"
TreeRenderer::TreeRenderer(Tree& tree, const TreeRendererResources& resources) : tree(tree), resources(resources)
{
	branchSSBO.init();
	leafSSBO.init();
	budSSBO.init();
	coloredLineSSBO.init();
}



void TreeRenderer::renderTree(DrawView view, bool branchs, bool leaves, Scene scene)
{
	if (branchs && resources.branchShader) {
		renderBranchs(view, scene);
	}
	if (leaves && resources.leafShader) {
		renderLeaves(view, scene);
	}
}

void TreeRenderer::renderBranchs(DrawView view, Scene scene)
{
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();

	glEnable(GL_CULL_FACE);

	glCullFace(GL_BACK);

	glFrontFace(GL_CCW);

	glDepthFunc(GL_LESS);

	const sp<Shader>& branchShader = resources.branchShader;
	const sp<Texture>& barkTexture = resources.barkTexture;
	const sp<GLVertexArray>& cubeVAO = resources.cubeVAO;

	branchShader->bind();
	cubeVAO->bind();

	branchShader->setUniform("VP", vp);

	barkTexture->bindTo(branchShader->getTextureIndex("barkTexture"));
	scene.shadowMap->bindTo(branchShader->getTextureIndex("shadowMap"));
	branchShader->setUniform("lightVP", scene.LightVP);
	vec3 camPos = view.camera.getCameraPosition();
	vec3 camDir = view.camera.getCameraDirection();
	branchShader->setUniform("camPos", camPos);
	branchShader->setUniform("viewDir", camDir);
	branchShader->setUniform("ambientColor", scene.ambientCol);
	branchShader->setUniform("lightColor", scene.lightColor);
	branchShader->setUniform("lightDir", scene.lightDir);
	branchShader->setUniform("treeColor", vec3(166.0f / 255.0f, 123.0f / 255.0f, 81.0f / 255.0f));
	branchShader->setUniform("farPlane", view.camera.getFarPlane());
	branchShader->setUniform("nearPlane", view.camera.getNearPlane());
	branchSSBO.bindBase(0);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 36, branchCount);
}

void TreeRenderer::renderBranchsShadow(Scene scene)
{
	glEnable(GL_CULL_FACE);

	glCullFace(GL_FRONT);

	glFrontFace(GL_CCW);

	glDepthFunc(GL_LESS);

	const sp<Shader>& branchShadowShader = resources.branchShadowShader;
	const sp<GLVertexArray>& cubeVAO = resources.cubeVAO;

	branchShadowShader->bind();
	cubeVAO->bind();

	branchShadowShader->setUniform("VP", scene.LightVP);

	vec3 camPos = scene.lightPos;
	vec3 camDir = scene.lightDir;
	branchShadowShader->setUniform("camPos", camPos);
	branchShadowShader->setUniform("viewDir", camDir);
	branchShadowShader->setUniform("farPlane", scene.lightFarPlane);
	branchShadowShader->setUniform("nearPlane", scene.lightNearPlane);
	branchSSBO.bindBase(0);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 36, branchCount);
}

void TreeRenderer::renderLeaves(DrawView view, Scene scene)
{
	glDisable(GL_CULL_FACE);
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();

	resources.quadVAO->bind();
	resources.leafShader->bind();

	resources.leafShader->setUniform("camPos", view.camera.getCameraPosition());
	resources.leafShader->setUniform("ambientColor", scene.ambientCol);
	resources.leafShader->setUniform("lightColor", scene.lightColor);
	resources.leafShader->setUniform("lightDir", scene.lightDir);

	resources.leafShader->setUniform("VP", vp);
	glBindTextureUnit(resources.leafShader->getTextureIndex("leafTex"), resources.leafTexture->getHandle());
	leafSSBO.bindBase(0);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, leafCount);
}

void TreeRenderer::renderLeavesShadow(Scene scene)
{
	glDisable(GL_CULL_FACE);

	resources.quadVAO->bind();
	resources.leavesShadowShader->bind();


	resources.leavesShadowShader->setUniform("VP", scene.LightVP);
	glBindTextureUnit(resources.leafShader->getTextureIndex("leafTex"), resources.leafTexture->getHandle());
	leafSSBO.bindBase(0);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, leafCount);
}

uint32 TreeRenderer::getLeafCount() const
{
	return leafCount;
}

uint32 TreeRenderer::getBranchCount() const
{
	return branchCount;
}

uint32 TreeRenderer::getBudCount() const
{
	return budCount;
}

void TreeRenderer::renderVigor(DrawView view)
{
	glEnable(GL_PROGRAM_POINT_SIZE);
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();
	resources.budPointShader->setUniform("VP", vp);
	resources.pointVAO->bind();
	budSSBO.bindBase(0);

	glDrawArraysInstanced(GL_POINTS, 0, 1, budCount);

	glDisable(GL_PROGRAM_POINT_SIZE);
}

void TreeRenderer::renderOptimalDirection(DrawView view) {
	glLineWidth(2.0f);
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();
	resources.coloredLineShader->setUniform("VP", vp);
	resources.lineVAO->bind();
	coloredLineSSBO.bindBase(0);
	glDrawElementsInstanced(GL_LINES, 2, GL_UNSIGNED_INT, 0, budCount);
}



void TreeRenderer::updateRenderer()
{
	const std::vector<Branch>& branchs = tree.getBranchs();
	std::vector<BranchShaderData> branchData;

	branchData.reserve(branchs.size());

	for (auto& branch : branchs) {

		uint32 colorSelected = branch.from.order;
		vec3 color = vec3(util::IntNoise2D(colorSelected), util::IntNoise2D(colorSelected, 1), util::IntNoise2D(colorSelected, 2)) * 0.5f + 0.5f;


		branchData.push_back(branch.asShaderData(color));
	}
	branchSSBO.bufferData(sizeof(BranchShaderData) * branchData.size(), branchData.data(), GL_STATIC_DRAW);
	branchCount = branchData.size();

	std::vector<mat4> models;

	for (auto& branch : branchs) {
		for (auto& leaf : branch.leaves) {
			models.emplace_back(leaf.model);
		}
	}
	leafSSBO.bufferData(sizeof(mat4) * models.size(), models.data(), GL_STATIC_DRAW);
	leafCount = models.size();


	//tree.world.calculateShadows();

	tree.accumulateLight();
	tree.root->vigor = tree.root->light;
	tree.distributeVigor();

	const auto& nodes = tree.AsNodeVector(true);
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

		vec3 dir = 0.1f * tree.world.getOptimalDirection(node.startPos);
		lines.emplace_back(vec4(node.startPos, 0.0f), vec4(node.startPos + dir, 0.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f));
	}
	budSSBO.bufferData(sizeof(BudPoint) * points.size(), points.data(), GL_STATIC_DRAW);
	coloredLineSSBO.bufferData(sizeof(ColoredLine) * lines.size(), lines.data(), GL_STATIC_DRAW);
	budCount = points.size();




}

TreeRenderer::~TreeRenderer()
{
	branchSSBO.destroy();
	leafSSBO.destroy();
	coloredLineSSBO.destroy();
	budSSBO.destroy();
}
