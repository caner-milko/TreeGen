#pragma once
#include "../Tree.h"
#include "GLBuffer.hpp"
#include "DrawView.h"
#include "GLVertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "Scene.h"

struct TreeRendererResources {
	GLVertexArray* quadVAO = nullptr, * cubeVAO = nullptr, * pointVAO = nullptr, * lineVAO = nullptr;
	Shader* branchShader = nullptr, * leafShader = nullptr, * budPointShader = nullptr, * coloredLineShader = nullptr;
	Texture* leafTexture = nullptr, * barkTexture = nullptr;
};


class TreeRenderer {
	struct BudPoint {
		vec4 pos;
		vec4 color;
	};
	struct ColoredLine {
		vec4 pos1;
		vec4 pos2;
		vec4 color;
	};
public:
	bool debug = false;
	TreeRendererResources resources;
	TreeRenderer(Tree& tree, const TreeRendererResources& resources = {});
	void renderBranchs(DrawView view, Scene scene);
	void renderLeaves(DrawView view, Scene scene);
	void renderTree(DrawView view, bool renderBranchs, bool renderLeaves, Scene scene);
	void renderVigor(DrawView view);
	void renderOptimalDirection(DrawView view);
	void updateRenderer();
	uint32 getLeafCount() const;
	uint32 getBranchCount() const;
	uint32 getBudCount() const;

	~TreeRenderer();
private:
	Tree& tree;
	SSBO branchSSBO{}, leafSSBO{};
	uint32 leafCount = 0, branchCount = 0;
	uint32 budCount = 0;
	SSBO budSSBO{};
	SSBO coloredLineSSBO{};
};