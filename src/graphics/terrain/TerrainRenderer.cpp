#include "TerrainRenderer.h"
#include "Pipeline.h"
#include "Rendering.h"
#include "Common.h"
namespace tgen::graphics::terrain
{
using namespace gl;
TerrainRenderer::TerrainRendererResources TerrainRenderer::resources = {};
constexpr gl::VertexInputBindingDescription TerrainMeshInput[] = {
	{.location = 0, .binding = 0,
	.format = gl::Format::R32G32B32_FLOAT, .offset = offsetof(TerrainVertex, pos)}
	,{.location = 1, .binding = 0,
	.format = gl::Format::R32G32B32_FLOAT, .offset = offsetof(TerrainVertex, normal)} };


TerrainRenderer::TerrainRenderer(Terrain& terrain) : terrain(terrain)
{
}


void TerrainRenderer::update()
{
	std::vector<uint32> indices;
	terrain.generateMesh(&vertices, &indices);
	mesh.vbo.init(vertices);
	mesh.ebo.init(indices);
}

void TerrainRenderer::updateTerrainColor(std::vector<glm::vec4>& obstacles)
{
	obstacleSSBO.init(std::span(obstacles));
}

void TerrainRenderer::renderTerrains(std::span<rb<TerrainRenderer>> renderers, const DrawView& view, const DrawScene& scene)
{
	const mat4& vp = view.VP;
	static gl::GraphicsPipeline TerrainPipeline =
		[]()->gl::GraphicsPipeline
	{
		gl::GraphicsPipeline pipeline("Render Terrain", *resources.terrainShader);
		pipeline.vertexInputState = { TerrainMeshInput };
		return pipeline;
	}();

	Cmd::ScopedGraphicsPipeline scoped(TerrainPipeline);


	Cmd::BindUBO(0, *resources.camUBO, 0, resources.camUBO->getRawSize());
	Cmd::BindUBO(1, *resources.lightUBO, 0, resources.lightUBO->getRawSize());
	Cmd::SetUniform("color", vec3(0.0, 1.0, 0.0));
	auto& shader = Cmd::GetShader();

	{
		glBindTextureUnit(shader.getTextureIndex("material.grassTex"), resources.material.grassTexture->getHandle());
		glBindTextureUnit(shader.getTextureIndex("material.dirtTex"), resources.material.dirtTexture->getHandle());
		glBindTextureUnit(shader.getTextureIndex("material.normalMap"), resources.material.normalMap->getHandle());
		Cmd::SetUniform("material.grassColorMultiplier", resources.material.grassColorMultiplier);
		Cmd::SetUniform("material.dirtColorMultiplier", resources.material.dirtColorMultiplier);
		Cmd::SetUniform("material.normalMapStrength", resources.material.normalMapStrength);
		Cmd::SetUniform("material.uvScale", resources.material.uvScale);
	}
	glBindTextureUnit(shader.getTextureIndex("shadowMap"), scene.light.shadowMap->getHandle());
	for (auto& renderer : renderers)
	{
		Cmd::util::BindMesh(renderer->mesh);
		Cmd::BindSSBO(0, renderer->obstacleSSBO, 0, renderer->obstacleSSBO.getRawSize());
		Cmd::SetUniform("obstacleCount", (int32)renderer->obstacleSSBO.getSize());
		mat4 model = glm::scale(glm::translate(mat4(1.0), renderer->terrain.data.center + vec3(0.0f, renderer->terrain.data.minHeight, 0.0f))
			, vec3(renderer->terrain.data.size.x, renderer->terrain.data.maxHeight - renderer->terrain.data.minHeight, renderer->terrain.data.size.y));
		mat3 ITmodel = glm::inverse(glm::transpose(mat3(model)));
		Cmd::SetUniform("model", model);
		Cmd::SetUniform("ITmodel", ITmodel);
		Cmd::DrawIndexed(renderer->mesh.ebo.getSize());

	}

}
void TerrainRenderer::renderTerrainShadows(std::span<rb<TerrainRenderer>> renderers, const DrawView& view)
{
	static gl::GraphicsPipeline TerrainPipeline =
		[]()->gl::GraphicsPipeline
	{
		gl::GraphicsPipeline pipeline("Terrain Shadows", *resources.terrainShadowShader);
		pipeline.vertexInputState = { TerrainMeshInput };
		return pipeline;
	}();

	Cmd::ScopedGraphicsPipeline scoped(TerrainPipeline);


	Cmd::BindUBO(0, *resources.camUBO, 0, resources.camUBO->getRawSize());
	Cmd::BindUBO(1, *resources.lightUBO, 0, resources.lightUBO->getRawSize());
	auto& shader = Cmd::GetShader();
	for (auto& renderer : renderers)
	{
		Cmd::util::BindMesh(renderer->mesh);

		mat4 model = glm::scale(glm::translate(mat4(1.0), renderer->terrain.data.center + vec3(0.0f, renderer->terrain.data.minHeight, 0.0f))
			, vec3(renderer->terrain.data.size.x, renderer->terrain.data.maxHeight - renderer->terrain.data.minHeight, renderer->terrain.data.size.y));

		Cmd::SetUniform("model", model);

		Cmd::DrawIndexed(renderer->mesh.ebo.getSize());

	}
}

}