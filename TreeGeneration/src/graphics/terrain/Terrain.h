#pragma once
#include "Common.h"
#include "graphics/Image.h"
#include <memory>
#include <vector>

namespace tgen::graphics::terrain {
struct TerrainVertex {
	vec3 pos;
	vec3 normal;
};
struct TerrainData {
	vec3 center = vec3(0.0);
	vec2 size = vec2(10.0);
	float maxHeight = 1.0;
	float minHeight = 0.0;
	ivec2 gridSize = ivec2(255, 255);

	rc<Image> heightMap = nullptr;
};
class Terrain {
public:
	TerrainData data;

	Terrain(const TerrainData& data);
	void generateMesh(std::vector<TerrainVertex>* vertices, std::vector<uint32>* indices) const;

	vec2 worldPosToUv(vec2 worldPos) const;
	float heightAtWorldPos(vec3 worldPos) const;
	float heightAt(vec2 uv) const;
	float valueAt(vec2 uv) const;
	vec3 normalAt(ivec2 vertex) const;

	inline int32 vertexIndex(ivec2 vertex) const {
		return vertex.y * (data.gridSize.x + 1) + vertex.x;
	}
	inline vec2 cellSize() const {
		return vec2(1.0f) / vec2(data.gridSize);
	}
private:
};
}