#pragma once
#include "Common.h"
#include "Terrain.h"
#include "opengl/types/VertexArray.h"
#include "opengl/types/Shader.h"
#include "DrawView.h"
#include "DrawScene.h"
#include "opengl/types/Texture.h"
#include "Mesh.h"
#include "Pipeline.h"
namespace tgen::graphics::terrain {
class TerrainRenderer {
public:
	TerrainRenderer(Terrain& terrain);
	DELETE_COPY_CONSTRUCTORS(TerrainRenderer);
	void update();
	void renderShadows(const DrawView& view) const;
	static void renderTerrains(std::span<rb<TerrainRenderer>> renderers, const DrawView& view, const DrawScene& scene);
	static void renderTerrainShadows(std::span<rb<TerrainRenderer>> renderers, const DrawView& view);
	struct TerrainRendererResources {
		rc<gl::Shader> terrainShader;
		rc<gl::Shader> terrainShadowShader;
		rc<gl::Texture> grassTexture;
		rc<gl::Shader> lineShader;
		rb<const IndexedMesh<gl::IndexType::UNSIGNED_INT>> lineVAO;
		rb<const gl::UBO<CameraUniform>> camUBO;
		rb<const gl::UBO<DirLightUniform>> lightUBO;
	} static resources;
private:
	Terrain& terrain;
	std::vector<TerrainVertex> vertices;
	CompleteMesh<TerrainVertex, gl::IndexType::UNSIGNED_INT> mesh;
};
}