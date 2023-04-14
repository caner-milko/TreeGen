#pragma once
#include <vector>
#include "Tree.h"
#include <span>
#include "EditableMap.h"
#include "terrain/Terrain.h"
namespace tgen::gen
{
struct ShadowCell
{
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
	TreeWorld(TreeWorldGrowthData worldGrowthData, const util::BBox& worldBoundingBox, float cellSize, uint32 seed = 0);

	TreeWorld(TreeWorldInfo info, TreeWorldGrowthData worldGrowthData);

	void SetWorldGrowthData(TreeWorldGrowthData data);
	void resizeShadowGrid();
	void recalculateLUT();

	//void calculateShadows();
	Tree& createTree(vec2 position, GrowthDataId growthDataId);
	const std::vector<std::unique_ptr<Tree>>& getTrees() const { return trees; }
	const std::vector<ShadowCell>& getShadowGrid() const { return shadowGrid; }
	void removeTree(uint32 treeId);
	float getLightAt(const vec3& position);
	vec3 getOptimalDirection(const vec3& position);

	void castShadows(const vec3& pos, bool addShadows = true);
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
	TreeWorldGrowthData& getWorldGrowthData() { return worldGrowthData; }

	GrowthDataId getGrowthDataFromMap(vec2 worldPos);
	void setPresetMap(rc<graphics::Image> image);
	std::pair<GrowthDataId, glm::vec<3, uint8>> newGrowthData(TreeGrowthData data = {}, std::optional<glm::vec<3, uint8>> col = std::nullopt);

	inline rb<Tree> getTreeById(uint32 id)
	{
		for (auto& tree : trees)
			if (tree->id == id)
				return tree.get();
		assert(false);
		return nullptr;
	}

	void clear()
	{
		resizeShadowGrid();
		trees.clear();
		age = 0;
		treeCount = 0;
	}

	uint32 age = 0;

	Event<EventData> OnAfterWorldGrow{};
	struct TreeCreatedEvent : EventData
	{
		Tree& newTree;
	};
	Event<TreeCreatedEvent> onTreeCreated{};
	Event<TreeCreatedEvent> onTreeDestroyed{};
	rb<graphics::terrain::Terrain> terrain;
protected:
	TreeWorldInfo info;
	TreeWorldGrowthData worldGrowthData;
	std::vector<float> pyramidShadowLUT;
	std::vector<std::unique_ptr<Tree>> trees;
	uint32 treeCount = 0;
	std::vector<ShadowCell> shadowGrid;
	rc<EditableMap> presetMap;
};
}