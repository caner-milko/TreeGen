#pragma once
#include "../../Definitions.h"
#include "Terrain.h"
#include "../GLVertexArray.h"
#include "../Shader.h"
#include "../DrawView.h"
#include "../Scene.h"
#include "../Texture.h"
class TerrainRenderer {
public:
	struct TerrainRendererResources {
		sp<Shader> terrainShader;
		sp<Texture> grassTexture;
		sp<Shader> lineShader;
		sp<GLVertexArray> lineVAO;
	};
	TerrainRenderer(Terrain& terrain, const TerrainRendererResources& resources);
	~TerrainRenderer();
	void update();
	void render(DrawView view, Scene scene) const;
	TerrainRendererResources resources;
private:
	Terrain& terrain;
	std::vector<Terrain::TerrainVertex> vertices;
	GLVertexArray vao;
	int indicesSize = 0;
};