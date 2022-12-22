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

vec3 lightDir = glm::normalize(vec3(0.4, -0.6, -0.4));
vec3 ambientCol = 0.1f * vec3(0.2f, 0.2f, 0.15f);
vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

void TreeRenderer::renderTree(DrawView view, bool branchs, bool leaves)
{
	if (branchs && resources.branchShader) {
		renderBranchs(view);
	}
	if (leaves && resources.leafShader) {
		renderLeaves(view);
	}
}

void TreeRenderer::renderBranchs(DrawView view)
{
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();

	glEnable(GL_CULL_FACE);

	glCullFace(GL_BACK);

	glFrontFace(GL_CCW);

	glDepthFunc(GL_LESS);

	Shader* branchShader = resources.branchShader;
	Texture* barkTexture = resources.barkTexture;
	GLVertexArray* cubeVAO = resources.cubeVAO;

	branchShader->bind();
	cubeVAO->bind();

	branchShader->setUniform("VP", vp);

	glBindTextureUnit(branchShader->getTextureIndex("barkTexture"), barkTexture->getHandle());

	vec3 camPos = view.camera.getCameraPosition();
	vec3 camDir = view.camera.getCameraDirection();
	branchShader->setUniform("camPos", camPos);
	branchShader->setUniform("viewDir", camDir);
	branchShader->setUniform("ambientColor", ambientCol);
	branchShader->setUniform("lightColor", lightColor);
	branchShader->setUniform("lightDir", lightDir);
	branchShader->setUniform("treeColor", vec3(166.0f / 255.0f, 123.0f / 255.0f, 81.0f / 255.0f));
	branchShader->setUniform("farPlane", view.camera.getFarPlane());
	branchShader->setUniform("nearPlane", view.camera.getNearPlane());
	branchSSBO.bindBase(0);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 36, branchCount);
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


	tree.world.calculateShadows();

	tree.accumulateLight();
	tree.root->vigor = tree.root->light;
	tree.distributeVigor();

	const auto& nodes = tree.AsNodeVector(true);
	std::vector<BudPoint> points;
	std::vector<ColoredLine> lines;
	for (auto& node : nodes)
	{
		if (node.nodeStatus == ALIVE)
			continue;
		float vigor = node.vigor;

		vec4 color = vigor >= 1.0f ? vec4(0.0f, 1.0f, 0.0f, vigor) : vec4(1.0f, 0.0f, 0.0f, vigor);
		if (node.nodeStatus == DEAD)
			color = vec4(0.0f, 0.0f, 0.0f, vigor);

		points.emplace_back(vec4(node.startPos, 0.0f), color);

		vec3 dir = 0.1f * tree.world.getOptimalDirection(node.startPos);
		lines.emplace_back(vec4(node.startPos, 0.0f), vec4(node.startPos + dir, 0.0f), vec4(0.0f, 0.0f, 1.0f, 0.0f));
	}
	budSSBO.bufferData(sizeof(BudPoint) * points.size(), points.data(), GL_STATIC_DRAW);
	coloredLineSSBO.bufferData(sizeof(ColoredLine) * lines.size(), lines.data(), GL_STATIC_DRAW);
	budCount = points.size();




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

void TreeRenderer::renderLeaves(DrawView view)
{
	glDisable(GL_CULL_FACE);
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();

	resources.quadVAO->bind();
	resources.leafShader->bind();
	resources.leafShader->setUniform("ambientColor", ambientCol);
	resources.leafShader->setUniform("lightColor", lightColor);
	resources.leafShader->setUniform("lightDir", lightDir);

	resources.leafShader->setUniform("VP", vp);
	glBindTextureUnit(resources.leafShader->getTextureIndex("leafTex"), resources.leafTexture->getHandle());
	leafSSBO.bindBase(0);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, leafCount);
}

TreeRenderer::~TreeRenderer()
{
	branchSSBO.destroy();
	leafSSBO.destroy();
	coloredLineSSBO.destroy();
	budSSBO.destroy();
}
