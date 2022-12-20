#pragma once
#include <vector>
#include "./Tree.h"

struct ShadowCell {
	float shadow = 0.0f;
};

struct TreeWorld
{
	uint32 seed = 0;
	std::vector<std::unique_ptr<Tree>> trees;
	uint32 treeCount = 0;
	std::vector<ShadowCell> shadowGrid;
	ivec3 worldSize;
	float cellSize;
	vec3 leftBottomCorner;
	TreeWorld(const BBox& worldBoundingBox, float cellSize, uint32 seed = 0);

	TreeWorld(ivec3 worldSize, vec3 leftBottomCorner, float cellSize, uint32 seed = 0);

	void resizeShadowGrid(ivec3 worldSize, vec3 leftBottomCorner, float cellSize);

	void calculateShadows();
	Tree* createTree(vec3 position, TreeGrowthData growthData);
	void removeTree(Tree& tree);
	float getLightAt(const vec3& position, float a, float fullExposure);
	vec3 getOptimalDirection(const vec3& position);

	void castShadows(const vec3& pos, int pyramidHeight, float a, float b);
	ivec3 coordinateToCell(const vec3& pos) const;
	vec3 cellToCoordinate(const ivec3& cell) const;
	int cellToIndex(const ivec3& cell) const;

	BBox getBBox() const;
	bool isOutOfBounds(const vec3& pos) const;

	std::vector<vec4>  renderShadowCells(const vec3& camPos, const vec3& viewDir, float fov, float visibilityRadius) const;

	ShadowCell& getCellAt(const ivec3& cell);
	const ShadowCell& getCellAt(const ivec3& cell) const;
	void addShadowTo(const ivec3& cell, float amount);

};
