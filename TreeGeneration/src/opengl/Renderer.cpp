#include "./Renderer.h"
#include <queue>
#include <iostream>
#include "../Util.hpp"
void Renderer::init()
{
	glEnable(GL_DEPTH_TEST);

	quadVAO.init();
	quadVAO.bind();
	const uint32 indices[] = {
			0, 1, 2,
			1, 3, 2,

			3, 4, 2,
			3, 5, 4,

			1, 0, 6,
			1, 6, 7
	};
	quadVAO.attachBuffer(GLVertexArray::BufferType::ELEMENT, sizeof(indices), GLVertexArray::DrawMode::STATIC, indices);

	lineVAO.init();
	lineVAO.bind();
	const float lineData[] = { 0.0f, 1.0f };
	lineVAO.attachBuffer(GLVertexArray::BufferType::ARRAY, sizeof(lineData), GLVertexArray::DrawMode::STATIC, lineData);
	lineVAO.enableAttribute(0, 2, sizeof(float), nullptr);

	pointVAO.init();
	pointVAO.bind();
	const float pointData[] = { 0.0f };
	pointVAO.attachBuffer(GLVertexArray::BufferType::ARRAY, sizeof(pointData), GLVertexArray::DrawMode::STATIC, pointData);
	pointVAO.enableAttribute(0, 1, sizeof(float), nullptr);

	glLineWidth(3.0f);

}

void Renderer::startFrame()
{
	glClearColor(0.0f, 0.3f, 0.2f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::endFrame()
{
}

void Renderer::renderTree(DrawView view, Shader* shader, const TreeNode* root)
{
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();

	std::queue< const TreeNode*> queue({ root });
	shader->bind();
	lineVAO.bind();
	shader->setUniform("VP", vp);

	while (!queue.empty()) {
		const TreeNode* sel = queue.front();
		queue.pop();
		if (!sel->bud) {
			queue.push(sel->mainChild);
			queue.push(sel->lateralChild);
			shader->setUniform("pos1", sel->startPos);
			shader->setUniform("pos2", sel->endPos());
			uint32 colorSelected = sel->order;
			shader->setUniform("color", vec3(util::IntNoise2D(colorSelected), util::IntNoise2D(colorSelected, 1), util::IntNoise2D(colorSelected, 2)) * 0.5f + 0.5f);
			glDrawArrays(GL_LINES, 0, 2);
		}
	}
}

struct Branch {
	vec3 start = vec3(0.0);
	vec3 end = vec3(0.0);
	float lowRadius = -1.0;
	float highRadius = -1.0;
	vec3 camProjected = vec3(0.0);
	vec3 camCross = vec3(0.0);
	TreeNode* selectedChild = nullptr;
};

Branch nodeToBranch(const TreeNode& node, const vec3& camPos) {
	if (node.bud) {
		return Branch();
	}
	Branch branch;
	branch.start = node.startPos;
	branch.end = node.endPos();
	vec3 dif = camPos - branch.start;
	vec3 dirToCam = glm::normalize(dif);
	vec3 dir = node.direction;
	float dot = glm::dot(dir, dirToCam);

	branch.camProjected = glm::normalize(dirToCam - dot * dir);
	branch.camCross = glm::normalize(glm::cross(dir, branch.camProjected));
	branch.lowRadius = node.radius;
	branch.selectedChild = node.mainChild;
	if (node.mainChild->radius < node.lateralChild->radius) {
		branch.selectedChild = node.lateralChild;
	}
	branch.highRadius = branch.selectedChild->radius;
	branch.lowRadius = glm::max(branch.lowRadius, 0.01f);
	branch.highRadius = glm::max(branch.highRadius, 0.01f);
	return branch;
}

void Renderer::renderTree2(DrawView view, Shader* shader, const std::vector<TreeNode>& nodes)
{
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();

	shader->bind();
	quadVAO.bind();

	shader->setUniform("VP", vp);
	vec3 camPos = view.camera.getCameraPosition();
	vec3 camDir = view.camera.getCameraDirection();
	shader->setUniform("camPos", camPos);
	shader->setUniform("viewDir", camDir);
	shader->setUniform("ambientColor", vec3(0.3f, 0.3f, 0.3f));
	shader->setUniform("lightColor", vec3(1.0f, 1.0f, 1.0f));
	shader->setUniform("lightDir", glm::normalize(vec3(0.3, 0.6, 0.2)));
	shader->setUniform("treeColor", vec3(166.0f / 255.0f, 123.0f / 255.0f, 81.0f / 255.0f));
	for (auto& node : nodes) {
		Branch mainBranch = nodeToBranch(node, camPos);

		shader->setUniform("branchMain.start", mainBranch.start);
		shader->setUniform("branchMain.end", mainBranch.end);

		shader->setUniform("branchMain.camProjected", mainBranch.camProjected);
		shader->setUniform("branchMain.camCross", mainBranch.camCross);

		shader->setUniform("branchMain.lowRadius", mainBranch.lowRadius);
		shader->setUniform("branchMain.highRadius", mainBranch.highRadius);

		Branch childBranch = nodeToBranch(*mainBranch.selectedChild, camPos);

		shader->setUniform("branchNext.start", childBranch.start);
		shader->setUniform("branchNext.end", childBranch.end);

		shader->setUniform("branchNext.camProjected", childBranch.camProjected);
		shader->setUniform("branchNext.camCross", childBranch.camCross);

		shader->setUniform("branchNext.lowRadius", childBranch.lowRadius);
		shader->setUniform("branchNext.highRadius", childBranch.highRadius);

		if (node.order == 0) {
			shader->setUniform("branchBefore.lowRadius", -1.0f);
		}
		else {
			Branch parentBranch = nodeToBranch(*node.parent, camPos);


			shader->setUniform("branchBefore.start", parentBranch.start);
			shader->setUniform("branchBefore.end", parentBranch.end);

			shader->setUniform("branchBefore.camProjected", parentBranch.camProjected);
			shader->setUniform("branchBefore.camCross", parentBranch.camCross);

			shader->setUniform("branchBefore.lowRadius", parentBranch.lowRadius);
			shader->setUniform("branchBefore.highRadius", parentBranch.highRadius);
		}

		uint32 colorSelected = node.order;
		shader->setUniform("branchMain.color", vec3(util::IntNoise2D(colorSelected), util::IntNoise2D(colorSelected, 1), util::IntNoise2D(colorSelected, 2)) * 0.5f + 0.5f);
		glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
	}
}




void RenderShadowPoint(Shader* shader, const vec3& pos, float shadow) {
	shader->setUniform("pos", pos);
	glPointSize(shadow * 2.0f);
	shader->setUniform("shadow", shadow);
	glDrawArrays(GL_POINTS, 0, 1);
}
void Renderer::renderShadowPoints(DrawView view, Shader* shader, const std::vector<std::tuple<vec3, float>>& points)
{
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();
	shader->bind();
	pointVAO.bind();
	shader->setUniform("VP", vp);

	for (auto& [pos, shadow] : points) {
		RenderShadowPoint(shader, pos, shadow);
	}
}

void Renderer::renderShadowsOnBuds(DrawView view, Shader* shader, const TreeWorld& world, const std::vector<TreeNode>& nodes)
{
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();
	shader->bind();
	pointVAO.bind();
	shader->setUniform("VP", vp);
	for (auto& node : nodes) {
		if (node.bud) {

			ShadowCell cell = world.getCellAt(world.coordinateToCell(node.startPos));
			RenderShadowPoint(shader, node.startPos, cell.shadow);
		}
	}
}







/*void Renderer::render(const DrawView view, DrawCall* drawCalls, uint32 count)
{
	glClearColor(0.0f, 0.3f, 0.2f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();
	for (uint32 i = 0; i < count; i++) {
		DrawCall& call = drawCalls[i];
		RenderMaterial& curMaterial = call.material;
		if (&curMaterial != m_LastMaterial) {
			changeToMaterial(curMaterial);
		}
		Shader& cur = *m_LastShader;
		cur.setUniform("MVP", call.transform.getModelMatrix());
		glBindVertexArray(call.vao);
		glDrawElements(GL_TRIANGLES, call.indexCount, GL_UNSIGNED_INT, 0);
	}
}

void Renderer::changeToMaterial(RenderMaterial& material)
{
	m_LastMaterial = &material;
	if (material.m_Shader != m_LastShader) {
		material.m_Shader->bind();
		m_LastShader = material.m_Shader;
	}
}
*/