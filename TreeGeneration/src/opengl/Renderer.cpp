#include "./Renderer.h"
#include <queue>
#include <iostream>
void Renderer::init()
{
	lineVAO.init();
	lineVAO.bind();
	const float data[] = { 0.0f, 1.0f };
	lineVAO.attachBuffer(GLVertexArray::BufferType::ARRAY, 2 * sizeof(float), GLVertexArray::DrawMode::STATIC, data);
	lineVAO.enableAttribute(0, 2, sizeof(float), nullptr);
}

void Renderer::renderTree(DrawView view, Shader* shader, const TreeNode* root)
{
	glClearColor(0.0f, 0.3f, 0.2f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();

	std::queue< const TreeNode*> queue({ root });
	shader->bind();
	lineVAO.bind();
	shader->setUniform("VP", vp);
	while (!queue.empty()) {
		const TreeNode* sel = queue.front();
		queue.pop();
		if (!sel->bud) {
			queue.push(sel->mainChild.get());
			queue.push(sel->lateralChild.get());
			shader->setUniform("pos1", sel->startPos);
			shader->setUniform("pos2", sel->endPos());
			glDrawArrays(GL_LINES, 0, 2);
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