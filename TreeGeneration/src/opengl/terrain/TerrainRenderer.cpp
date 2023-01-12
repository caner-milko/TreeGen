#include "TerrainRenderer.h"

TerrainRenderer::TerrainRenderer(Terrain& terrain, Shader* shader) : terrain(terrain), shader(shader)
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
	std::vector<Terrain::TerrainVertex> vertices;
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

	glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, 0);
}
