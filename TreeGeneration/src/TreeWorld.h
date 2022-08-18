#pragma once
#include <vector>
#include "./Tree.h"

struct ShadowCell {
	float shadow = 0.0f;
};

struct TreeWorld
{
	std::vector<Tree> trees;
	std::vector<ShadowCell> shadowGrid;
	int xSize, ySize, zSize;
	float cellSize;
	vec3 leftBottomCorner;
	TreeWorld(int xSize, int ySize, int zSize, vec3 leftBottomCorner, float cellSize);

	void calculateShadows();
	Tree& createTree(vec3 position);
	//xyz = optimal direction, w = totalLight
	float getLightAt(const vec3& position, float a, float fullExposure);
	vec3 getOptimalDirection(const vec3& position, const vec3& direction, float perceptionRadius, float perceptionAngle);
	void castShadows(const vec3& pos, int pyramidHeight, float a, float b);
	ivec3 coordinateToCell(const vec3& pos);
	vec3 cellToCoordinate(const ivec3& cell);
	int cellToIndex(const ivec3& cell) const;

	ShadowCell& getCellAt(const ivec3& cell);
	void addShadowTo(const ivec3& cell, float amount);

};
