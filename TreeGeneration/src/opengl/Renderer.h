#pragma once
#include <vector>
#include "./Camera.h"
#include "../Definitions.h"
#include "./DrawView.h"
#include "./Shader.h"
#include "./GLVertexArray.h"
#include "../TreeNode.h"
#include "../TreeWorld.h"
#include <glad/glad.h>
class Renderer {

public:
	void init();
	void startFrame();
	void endFrame();
	//void render(DrawView view, DrawCall* drawCalls, uint32 count);
	void renderTree(DrawView view, Shader* shader, const TreeNode* root);
	void renderTree2(DrawView view, Shader* shader, const std::vector<Branch>& branches);

	void renderShadowPoints(DrawView view, Shader* shader, const std::vector<std::tuple<vec3, float>>& points);

	void renderShadowsOnBuds(DrawView view, Shader* shader, const TreeWorld& world, const std::vector<TreeNode>& buds);


private:
	Shader* lastShader = nullptr;
	GLVertexArray quadVAO, lineVAO, pointVAO;
};
