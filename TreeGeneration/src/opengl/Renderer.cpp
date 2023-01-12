#include "./Renderer.h"
#include <queue>
#include <iostream>
#include "../Util.hpp"
#include "../Branch.h"
#include "GLBuffer.hpp"
void Renderer::init()
{
	glEnable(GL_DEPTH_TEST);
	float cubeVertices[] = {
		// Back face
		-0.5f, -0.5f, -0.5f, // Bottom-left
		 0.5f,  0.5f, -0.5f, // top-right
		 0.5f, -0.5f, -0.5f, // bottom-right         
		 0.5f,  0.5f, -0.5f, // top-right
		-0.5f, -0.5f, -0.5f, // bottom-left
		-0.5f,  0.5f, -0.5f, // top-left
		// Front face
		-0.5f, -0.5f,  0.5f, // bottom-left
		 0.5f, -0.5f,  0.5f, // bottom-right
		 0.5f,  0.5f,  0.5f, // top-right
		 0.5f,  0.5f,  0.5f, // top-right
		-0.5f,  0.5f,  0.5f, // top-left
		-0.5f, -0.5f,  0.5f, // bottom-left
		// Left face
		-0.5f,  0.5f,  0.5f, // top-right
		-0.5f,  0.5f, -0.5f, // top-left
		-0.5f, -0.5f, -0.5f, // bottom-left
		-0.5f, -0.5f, -0.5f, // bottom-left
		-0.5f, -0.5f,  0.5f, // bottom-right
		-0.5f,  0.5f,  0.5f, // top-right
		// Right face
		 0.5f,  0.5f,  0.5f, // top-left
		 0.5f, -0.5f, -0.5f, // bottom-right
		 0.5f,  0.5f, -0.5f, // top-right         
		 0.5f, -0.5f, -0.5f, // bottom-right
		 0.5f,  0.5f,  0.5f, // top-left
		 0.5f, -0.5f,  0.5f, // bottom-left     
		// Bottom face
		-0.5f, -0.5f, -0.5f, // top-right
		 0.5f, -0.5f, -0.5f, // top-left
		 0.5f, -0.5f,  0.5f, // bottom-left
		 0.5f, -0.5f,  0.5f, // bottom-left
		-0.5f, -0.5f,  0.5f, // bottom-right
		-0.5f, -0.5f, -0.5f, // top-right
		// Top face
		-0.5f,  0.5f, -0.5f, // top-left
		 0.5f,  0.5f,  0.5f, // bottom-right
		 0.5f,  0.5f, -0.5f, // top-right     
		 0.5f,  0.5f,  0.5f, // bottom-right
		-0.5f,  0.5f, -0.5f, // top-left
		-0.5f,  0.5f,  0.5f, // bottom-left        
	};

	cubeVAO.init();
	cubeVAO.bind();
	cubeVAO.attachBuffer(GLVertexArray::BufferType::ARRAY, sizeof(cubeVertices), GLVertexArray::DrawMode::STATIC, cubeVertices);
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
	quadVAO.init();
	quadVAO.bind();
	quadVAO.attachBuffer(GLVertexArray::BufferType::ARRAY, sizeof(quadVertices), GLVertexArray::DrawMode::STATIC, quadVertices);

	quadVAO.enableAttribute(0, 2, 2 * sizeof(float), nullptr);

	pointVAO.init();
	pointVAO.bind();
	const int32 pointData[] = { 0 };
	pointVAO.attachBuffer(GLVertexArray::BufferType::ELEMENT, sizeof(pointData), GLVertexArray::DrawMode::STATIC, pointData);

	lineVAO.init();
	lineVAO.bind();
	const int32 lineData[] = { 0, 1 };
	lineVAO.attachBuffer(GLVertexArray::BufferType::ELEMENT, sizeof(lineData), GLVertexArray::DrawMode::STATIC, lineData);

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

void Renderer::startDraw() {
	glEnable(GL_FRAMEBUFFER_SRGB);
}

void Renderer::endDraw() {
	glDisable(GL_FRAMEBUFFER_SRGB);
}




void Renderer::renderPlane(DrawView view, Shader* shader, mat4 model)
{
	vec3 lightDir = glm::normalize(vec3(0.4, -0.6, -0.4));
	vec3 ambientCol = 0.1f * vec3(0.2f, 0.2f, 0.15f);
	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
	glDisable(GL_CULL_FACE);
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();
	quadVAO.bind();
	shader->bind();
	shader->setUniform("VP", vp);
	shader->setUniform("model", model);

	shader->setUniform("color", vec3(1.0));
	shader->setUniform("camPos", view.camera.getCameraPosition());
	shader->setUniform("lightDir", lightDir);
	shader->setUniform("lightColor", lightColor);
	shader->setUniform("ambientColor", ambientCol);

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

void RenderShadowPoint(Shader* shader, const vec3& pos, float shadow) {
	shader->setUniform("pos", pos);
	glPointSize(shadow * 2.0f);
	shader->setUniform("shadow", shadow);
	glDrawArrays(GL_POINTS, 0, 1);
}
void Renderer::renderShadowPoints(DrawView view, Shader* shader, const std::vector<vec4>& points)
{
	glEnable(GL_PROGRAM_POINT_SIZE);
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();
	shader->bind();
	pointVAO.bind();
	shader->setUniform("VP", vp);

	uint32 ssbo;
	glCreateBuffers(1, &ssbo);
	glNamedBufferStorage(ssbo, sizeof(vec4) * points.size(), points.data(), GL_MAP_READ_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

	glDrawArraysInstanced(GL_POINTS, 0, 1, points.size());

	glDeleteBuffers(1, &ssbo);

	glDisable(GL_PROGRAM_POINT_SIZE);
}

void Renderer::renderShadowsOnBuds(DrawView view, Shader* shader, const TreeWorld& world, const std::vector<TreeNode>& nodes)
{
	glEnable(GL_PROGRAM_POINT_SIZE);
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();
	shader->bind();
	pointVAO.bind();
	shader->setUniform("VP", vp);
	std::vector<vec4> points;
	for (auto& node : nodes) {
		if (node.nodeStatus == TreeNode::BUD) {
			ivec3 cell = world.coordinateToCell(node.startPos);
			for (int i = 0; i < 10; i++) {
				if (cell.y + i >= world.worldSize.y)
					break;
				ShadowCell shadowCell = world.getCellAt(cell + ivec3(0, i, 0));
				points.emplace_back(vec4(node.startPos + vec3(0, world.cellSize * float(i), 0), shadowCell.shadow));
			}
		}
	}

	uint32 ssbo;
	glCreateBuffers(1, &ssbo);
	glNamedBufferStorage(ssbo, sizeof(vec4) * points.size(), points.data(), GL_MAP_READ_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

	glDrawArraysInstanced(GL_POINTS, 0, 1, points.size());

	glDeleteBuffers(1, &ssbo);
	glDisable(GL_PROGRAM_POINT_SIZE);
}

void Renderer::renderBBoxLines(DrawView view, Shader* shader, const BBox& bbox, const vec3& color)
{
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();
	shader->bind();
	lineVAO.bind();
	shader->setUniform("VP", vp);
	shader->setUniform("color", color);

	auto const drawLine = [&](const vec3& first, const vec3& second) {
		shader->setUniform("pos1", first);
		shader->setUniform("pos2", second);
		glDrawArrays(GL_LINES, 0, 2);
	};

	vec3 first = bbox.min;
	vec3 second = bbox.min;

	second.x = bbox.max.x;
	second.z = bbox.min.z;

	drawLine(first, second);

	second.x = bbox.min.x;
	second.z = bbox.max.z;

	drawLine(first, second);

	first.x = bbox.max.x;
	first.z = bbox.max.z;

	second.x = bbox.min.x;
	second.z = bbox.max.z;

	drawLine(first, second);

	second.x = bbox.max.x;
	second.z = bbox.min.z;

	drawLine(first, second);
	first = bbox.min;
	first.y = bbox.max.y;
	second.y = bbox.max.y;

	second.x = bbox.max.x;
	second.z = bbox.min.z;

	drawLine(first, second);

	second.x = bbox.min.x;
	second.z = bbox.max.z;

	drawLine(first, second);

	first.x = bbox.max.x;
	first.z = bbox.max.z;

	second.x = bbox.min.x;
	second.z = bbox.max.z;

	drawLine(first, second);

	second.x = bbox.max.x;
	second.z = bbox.min.z;

	drawLine(first, second);

	first = bbox.min;
	second = bbox.min;
	second.y = bbox.max.y;

	drawLine(first, second);

	first.x = bbox.max.x;
	second.x = bbox.max.x;
	drawLine(first, second);

	first.z = bbox.max.z;
	second.z = bbox.max.z;
	drawLine(first, second);

	first.x = bbox.min.x;
	second.x = bbox.min.x;
	drawLine(first, second);
}

void Renderer::setupSkybox(CubemapTexture* skyboxTexture, Shader* skyboxShader)
{
	this->skyboxShader = skyboxShader;
	this->skyboxTexture = skyboxTexture;
}

void Renderer::renderSkybox(DrawView view)
{
	glDisable(GL_CULL_FACE);

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
	glEnable(GL_CULL_FACE);
}

GLVertexArray& Renderer::getCubeVAO()
{
	return cubeVAO;
}

GLVertexArray& Renderer::getQuadVAO()
{
	return quadVAO;
}

GLVertexArray& Renderer::getLineVAO()
{
	return lineVAO;
}

GLVertexArray& Renderer::getPointVAO()
{
	return pointVAO;
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