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

	void renderShadowPoints(DrawView view, Shader* shader, const std::vector<vec4>& points);

	void renderShadowsOnBuds(DrawView view, Shader* shader, const TreeWorld& world, const std::vector<TreeNode>& buds);

	void renderBBoxLines(DrawView view, Shader* shader, const BBox& bbox, const vec3& color);

	void setupSkybox(CubemapTexture* skyboxTexture, Shader* skyboxShader);

	void renderSkybox(DrawView view);

	GLVertexArray& getCubeVAO();
	GLVertexArray& getQuadVAO();
	GLVertexArray& getLineVAO();
	GLVertexArray& getPointVAO();
private:


	CubemapTexture* skyboxTexture = nullptr;
	Shader* skyboxShader = nullptr;


	Shader* lastShader = nullptr;

	GLVertexArray cubeVAO, quadVAO, lineVAO, pointVAO;
};
