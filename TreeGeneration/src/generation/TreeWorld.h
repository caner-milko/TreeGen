#pragma once
#include <vector>
#include "Tree.h"
#include <span>
namespace tgen::gen {
struct ShadowCell {
	float shadow = 0.0f;
};

class TreeWorld
{
public:
	struct TreeWorldInfo
	{
		uint32 seed;
		ivec3 worldSize;
		vec3 leftBottomCorner;
		float cellSize;
		bool operator==(const TreeWorldInfo& rhs) const = default;
	};
	TreeWorld(const util::BBox& worldBoundingBox, float cellSize, uint32 seed = 0);

	TreeWorld(TreeWorldInfo info);

	void resizeShadowGrid();

	//void calculateShadows();
	Tree& createTree(vec3 position, TreeGrowthData growthData);
	const std::vector<std::unique_ptr<Tree>>& getTrees() const { return trees; }
	const std::vector<ShadowCell>& getShadowGrid() const { return shadowGrid; }
	void removeTree(Tree& tree);
	float getLightAt(const vec3& position, float a, float fullExposure);
	vec3 getOptimalDirection(const vec3& position);

	void castShadows(const vec3& pos, int pyramidHeight, float a, float b, bool addShadows = true);
	ivec3 coordinateToCell(const vec3& pos) const;
	vec3 cellToCoordinate(const ivec3& cell) const;
	int cellToIndex(const ivec3& cell) const;

	inline util::BBox getBBox() const { return util::BBox(info.leftBottomCorner, info.leftBottomCorner + vec3(info.worldSize) * info.cellSize); }
	inline bool isOutOfBounds(const vec3& pos) const
	{
		ivec3 cell = coordinateToCell(pos);
		return isOutOfBounds(cell);
	}
	inline bool isOutOfBounds(const ivec3& cellPos) const
	{
		return cellPos.x < 0 || cellPos.x >= info.worldSize.x || cellPos.y < 0 || cellPos.y >= info.worldSize.y ||
			cellPos.z < 0 || cellPos.z >= info.worldSize.z;
	}
	std::vector<vec4>  renderShadowCells(const vec3& camPos, const vec3& viewDir, float fov, float visibilityRadius) const;

	ShadowCell& getCellAt(const ivec3& cell);
	const ShadowCell& getCellAt(const ivec3& cell) const;
	void addShadowTo(const ivec3& cell, float amount);
	TreeWorldInfo getWorldInfo() { return info; }
	uint32 age = 0;
protected:
	TreeWorldInfo info;
	std::vector<std::unique_ptr<Tree>> trees;
	uint32 treeCount = 0;
	std::vector<ShadowCell> shadowGrid;
};
}