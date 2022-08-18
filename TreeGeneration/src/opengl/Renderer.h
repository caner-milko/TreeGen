#pragma once
#include <vector>
#include "./Camera.h"
#include "../Definitions.h"
#include "./DrawView.h"
#include "./Shader.h"
#include "./GLVertexArray.h"
#include "../TreeNode.h"
#include <glad/glad.h>
class Renderer {

public:
	void init();
	//void render(DrawView view, DrawCall* drawCalls, uint32 count);
	void renderTree(DrawView view, Shader* shader, const TreeNode* root);


private:
	Shader* lastShader = nullptr;
	GLVertexArray lineVAO;
};
