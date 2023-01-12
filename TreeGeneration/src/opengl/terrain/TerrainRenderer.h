#pragma once
#include "../../Definitions.h"
#include "Terrain.h"
#include "../GLVertexArray.h"
#include "../Shader.h"
#include "../DrawView.h"
#include "../Scene.h"
class TerrainRenderer {
public:
	TerrainRenderer(Terrain& terrain, Shader* terrainShader);
	~TerrainRenderer();
	void update();
	void render(DrawView view, Scene scene) const;

private:
	Terrain& terrain;
	Shader* shader;
	GLVertexArray vao;
	int indicesSize;
};