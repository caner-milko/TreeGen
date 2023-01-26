#include "TerrainRenderer.h"

TerrainRenderer::TerrainRenderer(Terrain& terrain, const TerrainRendererResources& resources) : terrain(terrain), resources(resources)
{
}

TerrainRenderer::~TerrainRenderer()
{
	vao.destroy();
}


void TerrainRenderer::update()
{
	vao.destroy();
	vao.init();
	vao.bind();
	std::vector<int32> indices;
	terrain.generateMesh(&vertices, &indices);
	indicesSize = indices.size();
	vao.attachBuffer(GLVertexArray::BufferType::ARRAY, vertices.size() * sizeof(Terrain::TerrainVertex), GLVertexArray::DrawMode::STATIC, vertices.data());
	vao.attachBuffer(GLVertexArray::BufferType::ELEMENT, indices.size() * sizeof(int32), GLVertexArray::DrawMode::STATIC, indices.data());

	vao.enableAttribute(0, 3, sizeof(Terrain::TerrainVertex), (void*)0);
	vao.enableAttribute(1, 3, sizeof(Terrain::TerrainVertex), (void*)sizeof(vec3));
}

void TerrainRenderer::render(DrawView view, Scene scene) const
{
	mat4 vp = view.camera.getProjectionMatrix() * view.camera.getViewMatrix();

	glDisable(GL_CULL_FACE);

	glCullFace(GL_BACK);

	glFrontFace(GL_CCW);

	glDepthFunc(GL_LESS);

	auto& shader = resources.terrainShader;

	shader->bind();
	vao.bind();

	shader->setUniform("VP", vp);

	vec3 camPos = view.camera.getCameraPosition();
	vec3 camDir = view.camera.getCameraDirection();
	shader->setUniform("camPos", camPos);
	shader->setUniform("viewDir", camDir);
	shader->setUniform("ambientColor", scene.ambientCol);
	shader->setUniform("lightColor", scene.lightColor);
	shader->setUniform("lightDir", scene.lightDir);
	shader->setUniform("color", vec3(0.0, 1.0, 0.0));

	mat4 model = glm::scale(glm::translate(mat4(1.0), terrain.data.center + vec3(0.0f, terrain.data.minHeight, 0.0f))
		, vec3(terrain.data.size.x, terrain.data.maxHeight - terrain.data.minHeight, terrain.data.size.y));

	shader->setUniform("model", model);

	glBindTextureUnit(shader->getTextureIndex("grassTex"), resources.grassTexture->getHandle());

	glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, 0);


	/*glLineWidth(2.0f);
	resources.lineShader->setUniform("VP", vp);
	resources.lineVAO->bind();
	resources.lineShader->setUniform("color", vec3(1.0f, 1.0f, 0.5f));
	for (auto& vertex : vertices) {
		vec3 pos = vec3(model * vec4(vertex.pos - vec3(0.5f, 0.0f, 0.5f), 1.0f));
		if (glm::length(pos - view.camera.getCameraPosition()) > 1.0f)
			continue;
		vec3 norm = glm::normalize(mat3(glm::transpose(glm::inverse(model))) * vertex.normal);
		resources.lineShader->setUniform("pos1", pos);
		resources.lineShader->setUniform("pos2", pos + norm * 0.05f);
		glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
	}*/

}
