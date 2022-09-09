#include "./Renderer.h"
#include <queue>
#include <iostream>
#include "../Util.hpp"
void Renderer::init()
{
	glEnable(GL_DEPTH_TEST);
	lineVAO.init();
	lineVAO.bind();
	const float lineData[] = { 0.0f, 1.0f };
	lineVAO.attachBuffer(GLVertexArray::BufferType::ARRAY, 2 * sizeof(float), GLVertexArray::DrawMode::STATIC, lineData);
	lineVAO.enableAttribute(0, 2, sizeof(float), nullptr);

	pointVAO.init();
	pointVAO.bind();
	const float pointData[] = { 0.0f };
	pointVAO.attachBuffer(GLVertexArray::BufferType::ARRAY, sizeof(float), GLVertexArray::DrawMode::STATIC, pointData);
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
			uint32 colorSelected = sel->createdAt;
			shader->setUniform("color", vec3(util::IntNoise2D(colorSelected), util::IntNoise2D(colorSelected, 1), util::IntNoise2D(colorSelected, 2)) * 0.5f + 0.5f);
			glDrawArrays(GL_LINES, 0, 2);
		}
	}
}

void Renderer::renderShadowPoints(DrawView view, Shader* shader, const std::vector<std::tuple<vec3, float>>& points)
{
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();
	shader->bind();
	pointVAO.bind();
	shader->setUniform("VP", vp);

	for (auto& [pos, shadow] : points) {
		shader->setUniform("pos", pos);
		glPointSize(shadow);
		shader->setUniform("shadow", shadow);
		glDrawArrays(GL_POINTS, 0, 1);
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