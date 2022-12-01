#pragma once
#include <vector>
#include "./Camera.h"
#include "../Definitions.h"
#include "./DrawView.h"
#include "./Shader.h"
#include "./GLVertexArray.h"
#include "../TreeNode.h"
#include "../TreeWorld.h"
#include "CubemapTexture.h"
#include <glad/glad.h>
class Renderer {

public:
	void init();
	void startFrame();
	void endFrame();

	void renderPlane(DrawView view, Shader* shader, mat4 model);

	//void renderTree(DrawView view, Shader* shader, const TreeNode* root);
	void renderTree2(DrawView view, Texture* barkTex, Shader* shader, const std::vector<Branch>& branches);

	void renderLeaves(DrawView view, Texture* leafTex, Shader* shader, const std::vector<Branch>& branches);



	void renderShadowPoints(DrawView view, Shader* shader, const std::vector<std::tuple<vec3, float>>& points);

	void renderShadowsOnBuds(DrawView view, Shader* shader, const TreeWorld& world, const std::vector<TreeNode>& buds);

	void setupSkybox(CubemapTexture* skyboxTexture, Shader* skyboxShader);

	void renderSkybox(DrawView view);

private:


	CubemapTexture* skyboxTexture = nullptr;
	Shader* skyboxShader = nullptr;
	GLVertexArray cubeVAO;

	Shader* lastShader = nullptr;
	GLVertexArray leafQuadVAO, pointVAO;
};
