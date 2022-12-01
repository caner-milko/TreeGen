#include "./Renderer.h"
#include <queue>
#include <iostream>
#include "../Util.hpp"
#include "../Branch.h"
void Renderer::init()
{
	glEnable(GL_DEPTH_TEST);

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	cubeVAO.init();
	cubeVAO.bind();
	cubeVAO.attachBuffer(GLVertexArray::BufferType::ARRAY, sizeof(skyboxVertices), GLVertexArray::DrawMode::STATIC, skyboxVertices);
	cubeVAO.enableAttribute(0, 3, 3 * sizeof(float), nullptr);

	float quadVertices[] = {
		// positions
		 -0.5f,  1.0f,
		 -0.5f,  0.0f,
		 0.5f,  0.0f,

		 0.5f,  0.0f,
		 0.5f,  1.0f,
		-0.5f,  1.0f,
	};
	leafQuadVAO.init();
	leafQuadVAO.bind();
	leafQuadVAO.attachBuffer(GLVertexArray::BufferType::ARRAY, sizeof(quadVertices), GLVertexArray::DrawMode::STATIC, quadVertices);

	leafQuadVAO.enableAttribute(0, 2, 2 * sizeof(float), nullptr);

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

	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void Renderer::endFrame()
{
	lastShader = nullptr;
}

void Renderer::renderPlane(DrawView view, Shader* shader, mat4 model)
{
	glDisable(GL_CULL_FACE);
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();
	leafQuadVAO.bind();
	shader->bind();
	shader->setUniform("VP", vp);
	shader->setUniform("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glEnable(GL_CULL_FACE);
}


/*void Renderer::renderTree(DrawView view, Shader* shader, const TreeNode* root)
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
}*/

void Renderer::renderTree2(DrawView view, Texture* barkTex, Shader* shader, const std::vector<Branch>& branches)
{
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();

	glEnable(GL_CULL_FACE);

	glCullFace(GL_BACK);

	glFrontFace(GL_CCW);

	glDepthFunc(GL_LEQUAL);

	shader->bind();
	cubeVAO.bind();

	shader->setUniform("VP", vp);

	glActiveTexture(GL_TEXTURE0 + shader->getTextureIndex("barkTexture"));
	barkTex->bind();

	vec3 camPos = view.camera.getCameraPosition();
	vec3 camDir = view.camera.getCameraDirection();
	shader->setUniform("camPos", camPos);
	shader->setUniform("viewDir", camDir);
	shader->setUniform("ambientColor", vec3(0.3f, 0.3f, 0.3f));
	shader->setUniform("lightColor", vec3(1.0f, 1.0f, 1.0f));
	shader->setUniform("lightDir", glm::normalize(vec3(-0.3, 0.6, -0.2)));
	shader->setUniform("treeColor", vec3(166.0f / 255.0f, 123.0f / 255.0f, 81.0f / 255.0f));
	shader->setUniform("farPlane", view.camera.getFarPlane());
	shader->setUniform("nearPlane", view.camera.getNearPlane());
	for (auto& branch : branches) {
		vec3 dif = camPos - branch.A;
		vec3 dirToCam = glm::normalize(dif);
		vec3 dir = glm::normalize(branch.C - branch.A);
		float dot = glm::dot(dir, dirToCam);

		shader->setUniform("branch.start", branch.A);
		shader->setUniform("branch.mid", branch.B);
		shader->setUniform("branch.end", branch.C);

		shader->setUniform("model", branch.boundingBox.asModel());

		shader->setUniform("branch.lowRadius", branch.lowRadius);
		shader->setUniform("branch.highRadius", branch.highRadius);

		shader->setUniform("branch.branchLength", branch.length);
		shader->setUniform("branch.startLength", branch.startLength);

		shader->setUniform("branch.uvOffset", branch.offset);

		uint32 colorSelected = branch.from.order;
		shader->setUniform("branch.color", vec3(util::IntNoise2D(colorSelected), util::IntNoise2D(colorSelected, 1), util::IntNoise2D(colorSelected, 2)) * 0.5f + 0.5f);


		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}

void Renderer::renderLeaves(DrawView view, Texture* leafTex, Shader* shader, const std::vector<Branch>& branches)
{
	glDisable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();



	leafQuadVAO.bind();
	shader->bind();

	shader->setUniform("VP", vp);
	glActiveTexture(GL_TEXTURE0 + shader->getTextureIndex("leafTex"));
	leafTex->bind();
	for (auto& branch : branches) {
		for (auto& leaf : branch.leaves) {
			shader->setUniform("model", leaf.model);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
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

void Renderer::setupSkybox(CubemapTexture* skyboxTexture, Shader* skyboxShader)
{
	this->skyboxShader = skyboxShader;
	this->skyboxTexture = skyboxTexture;
}

void Renderer::renderSkybox(DrawView view)
{
	glEnable(GL_CULL_FACE);

	glCullFace(GL_BACK);

	glFrontFace(GL_CCW);

	glDepthFunc(GL_LEQUAL);

	if (skyboxTexture && skyboxShader) {
		mat4 vp = view.camera.getProjectionMatrix() * glm::mat4(glm::mat3(view.camera.getViewMatrix()));
		skyboxShader->bind();
		cubeVAO.bind();
		glActiveTexture(GL_TEXTURE0 + skyboxShader->getTextureIndex("skybox"));
		skyboxTexture->bind();
		skyboxShader->setUniform("skybox_vp", vp);
		glDrawArrays(GL_TRIANGLES, 0, 36);
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