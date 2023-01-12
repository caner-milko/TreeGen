#pragma once
#include "../../Definitions.h"
#include "../Image.h"
#include <memory>
#include <vector>
class Terrain {
public:
	struct TerrainVertex {
		vec3 pos;
		vec3 normal;
	};
	struct TerrainData {
		vec2 center = vec2(0.0);
		vec2 size;
		ivec2 gridSize = ivec2(100, 100);
		std::shared_ptr<Image> heightMap = nullptr;
	};
	Terrain(const TerrainData& data);
	void generateMesh(std::vector<TerrainVertex>* vertices, std::vector<int32>* indices) const;
	TerrainData data;

	float heightAt(vec2 uv) const;
	vec3 normalAt(ivec2 vertex) const;

	inline int32 vertexIndex(ivec2 vertex) const {
		return vertex.y * (data.gridSize.x + 1) + vertex.x;
	}
	inline vec2 cellSize() const {
		return data.size / vec2(data.gridSize);
	}
private:
};